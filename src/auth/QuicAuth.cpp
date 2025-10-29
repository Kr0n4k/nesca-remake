#include "QuicAuth.h"
#include "Connector.h"
#include "externData.h"
#include "STh.h"
#include <curl/curl.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <QString>

// Generate QUIC Client Hello
std::string QuicAuth::generateQuicClientHello() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::stringstream ss;
    ss << "QUIC ";
    
    // Generate random connection ID
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    return ss.str();
}

// Validate QUIC response
bool QuicAuth::validateQuicResponse(const std::string& response) {
    if (response.empty()) return false;
    
    std::string lowerResponse = response;
    std::transform(lowerResponse.begin(), lowerResponse.end(), lowerResponse.begin(), ::tolower);
    
    return (lowerResponse.find("quic") != std::string::npos ||
            lowerResponse.find("h3") != std::string::npos ||
            lowerResponse.find("http/3") != std::string::npos ||
            lowerResponse.find("alt-svc") != std::string::npos ||
            lowerResponse.find(":443") != std::string::npos);
}

// Extract QUIC version from response
std::string QuicAuth::extractQuicVersion(const std::string& response) {
    if (response.empty()) return "";
    
    // Look for QUIC version indicators
    size_t pos = response.find("QUIC/");
    if (pos != std::string::npos) {
        size_t start = pos + 5;
        size_t end = response.find(" ", start);
        if (end == std::string::npos) end = response.find("\r", start);
        if (end == std::string::npos) end = response.find("\n", start);
        if (end == std::string::npos) end = response.length();
        
        return response.substr(start, end - start);
    }
    
    // Look for HTTP/3 indicators
    pos = response.find("HTTP/3");
    if (pos != std::string::npos) {
        return "HTTP/3";
    }
    
    // Look for h3 indicators
    pos = response.find("h3=");
    if (pos != std::string::npos) {
        size_t start = pos + 3;
        size_t end = response.find(";", start);
        if (end == std::string::npos) end = response.find(" ", start);
        if (end == std::string::npos) end = response.find("\r", start);
        if (end == std::string::npos) end = response.find("\n", start);
        if (end == std::string::npos) end = response.length();
        
        return "h3-" + response.substr(start, end - start);
    }
    
    return "Unknown";
}

// Extract server name from response
std::string QuicAuth::extractServerName(const std::string& response) {
    if (response.empty()) return "";
    
    // Look for Server header
    size_t pos = response.find("Server:");
    if (pos != std::string::npos) {
        pos += 7;
        while (pos < response.length() && (response[pos] == ' ' || response[pos] == '\t')) {
            pos++;
        }
        
        size_t end = response.find("\r", pos);
        if (end == std::string::npos) end = response.find("\n", pos);
        if (end == std::string::npos) end = response.length();
        
        return response.substr(pos, end - pos);
    }
    
    return "";
}

// Detect QUIC support in response
bool QuicAuth::detectQuicSupport(const std::string* response) {
    if (!response || response->empty()) {
        return false;
    }
    
    return validateQuicResponse(*response);
}

// Check if response is QUIC
bool QuicAuth::isQuicResponse(const std::string* response) {
    if (!response || response->empty()) {
        return false;
    }
    
    std::string lowerResponse = *response;
    std::transform(lowerResponse.begin(), lowerResponse.end(), lowerResponse.begin(), ::tolower);
    
    return (lowerResponse.find("quic") != std::string::npos ||
            lowerResponse.find("h3") != std::string::npos ||
            lowerResponse.find("http/3") != std::string::npos);
}

// Detect QUIC version
std::string QuicAuth::detectQuicVersion(const std::string* response) {
    if (!response || response->empty()) {
        return "";
    }
    
    return extractQuicVersion(*response);
}

// Get QUIC server name
std::string QuicAuth::getQuicServerName(const std::string* response) {
    if (!response || response->empty()) {
        return "";
    }
    
    return extractServerName(*response);
}

// Connect to QUIC endpoint
QuicAuth::QuicResult QuicAuth::connectQuic(const char* ip, int port, const char* hostname, bool useHttp3) {
    QuicResult result;
    result.success = false;
    result.statusCode = 0;
    result.version = "";
    result.serverName = "";
    
    if (!gQuicEnabled) {
        result.error = "QUIC scanning is disabled";
        return result;
    }
    
    // Build URL
    std::string protocol = useHttp3 ? "https://" : "http://";
    std::string url = protocol + std::string(ip) + ":" + std::to_string(port) + "/";
    
    // Prepare custom headers for QUIC/HTTP3
    std::vector<std::string> headers;
    headers.push_back("Alt-Svc: h3=\":443\"; ma=86400");
    headers.push_back("Upgrade-Insecure-Requests: 1");
    
    // Use custom User-Agent if provided
    if (gUserAgent[0] != '\0') {
        headers.push_back(std::string("User-Agent: ") + gUserAgent);
    } else {
        headers.push_back("User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0");
    }
    
    // Add hostname if provided
    if (hostname) {
        headers.push_back(std::string("Host: ") + hostname);
    }
    
    // Connect using existing Connector
    Connector connector;
    std::string response;
    
    int res = connector.nConnect(url.c_str(), port, &response, nullptr, &headers);
    
    if (res > 0) {
        result.response = response;
        result.statusCode = 200; // Assume success for QUIC
        
        // Check if it's a valid QUIC response
        if (isQuicResponse(&response)) {
            result.success = true;
            result.version = detectQuicVersion(&response);
            result.serverName = getQuicServerName(&response);
        } else {
            // Check for Alt-Svc header indicating QUIC support
            std::string lowerResponse = response;
            std::transform(lowerResponse.begin(), lowerResponse.end(), lowerResponse.begin(), ::tolower);
            
            if (lowerResponse.find("alt-svc") != std::string::npos && 
                lowerResponse.find("h3") != std::string::npos) {
                result.success = true;
                result.version = "HTTP/3 (Alt-Svc)";
            } else {
                result.error = "Not a QUIC response";
            }
        }
    } else {
        result.error = "Connection failed";
    }
    
    return result;
}

// Test QUIC endpoint
QuicAuth::QuicResult QuicAuth::testQuicEndpoint(const char* ip, int port, const char* hostname) {
    QuicResult result = connectQuic(ip, port, hostname, true);
    
    if (result.success) {
        if (gDebugMode) {
            QString message = QString("[QUIC] Found QUIC endpoint: %1:%2 (Version: %3)")
                .arg(ip)
                .arg(port)
                .arg(QString::fromStdString(result.version));
            stt->doEmitionYellowFoundData(message);
        }
    }
    
    return result;
}

// Test QUIC authentication
QuicAuth::QuicResult QuicAuth::testQuicAuth(const char* ip, int port, const char* hostname, 
                                           const char* username, const char* password) {
    QuicResult result;
    result.success = false;
    result.statusCode = 0;
    result.version = "";
    result.serverName = "";
    
    if (!gQuicEnabled) {
        result.error = "QUIC scanning is disabled";
        return result;
    }
    
    // Build URL with authentication
    std::string authStr = std::string(username) + ":" + std::string(password);
    std::string url = "https://" + authStr + "@" + std::string(ip) + ":" + std::to_string(port) + "/";
    
    // Prepare custom headers for QUIC/HTTP3
    std::vector<std::string> headers;
    headers.push_back("Alt-Svc: h3=\":443\"; ma=86400");
    headers.push_back("Upgrade-Insecure-Requests: 1");
    
    // Use custom User-Agent if provided
    if (gUserAgent[0] != '\0') {
        headers.push_back(std::string("User-Agent: ") + gUserAgent);
    } else {
        headers.push_back("User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0");
    }
    
    // Add hostname if provided
    if (hostname) {
        headers.push_back(std::string("Host: ") + hostname);
    }
    
    // Connect using existing Connector
    Connector connector;
    std::string response;
    
    int res = connector.nConnect(url.c_str(), port, &response, nullptr, &headers);
    
    if (res > 0) {
        result.response = response;
        result.statusCode = 200;
        
        if (isQuicResponse(&response)) {
            result.success = true;
            result.version = detectQuicVersion(&response);
            result.serverName = getQuicServerName(&response);
        } else {
            result.error = "Authentication failed or not QUIC";
        }
    } else {
        result.error = "Authentication failed";
    }
    
    return result;
}

// Get common QUIC ports
std::vector<int> QuicAuth::getCommonQuicPorts() {
    return {443, 80, 8080, 8443, 80, 443, 8080, 8443, 80, 443};
}

// Get QUIC versions
std::vector<std::string> QuicAuth::getQuicVersions() {
    return {"Q043", "Q046", "Q050", "h3-29", "h3-28", "h3-27", "HTTP/3"};
}

// Test HTTP/3 over QUIC
QuicAuth::QuicResult QuicAuth::testHttp3OverQuic(const char* ip, int port, const char* hostname, const char* path) {
    QuicResult result;
    result.success = false;
    result.statusCode = 0;
    result.version = "";
    result.serverName = "";
    
    if (!gQuicEnabled) {
        result.error = "QUIC scanning is disabled";
        return result;
    }
    
    // Build URL for HTTP/3
    std::string url = "https://" + std::string(ip) + ":" + std::to_string(port) + std::string(path);
    
    // Prepare headers specifically for HTTP/3
    std::vector<std::string> headers;
    headers.push_back("Alt-Svc: h3=\":443\"; ma=86400");
    headers.push_back("Upgrade-Insecure-Requests: 1");
    headers.push_back("Connection: Upgrade");
    headers.push_back("Upgrade: h3");
    
    // Use custom User-Agent if provided
    if (gUserAgent[0] != '\0') {
        headers.push_back(std::string("User-Agent: ") + gUserAgent);
    } else {
        headers.push_back("User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0");
    }
    
    // Add hostname if provided
    if (hostname) {
        headers.push_back(std::string("Host: ") + hostname);
    }
    
    // Connect using existing Connector
    Connector connector;
    std::string response;
    
    int res = connector.nConnect(url.c_str(), port, &response, nullptr, &headers);
    
    if (res > 0) {
        result.response = response;
        result.statusCode = 200;
        
        // Check for HTTP/3 specific indicators
        std::string lowerResponse = response;
        std::transform(lowerResponse.begin(), lowerResponse.end(), lowerResponse.begin(), ::tolower);
        
        if (lowerResponse.find("h3") != std::string::npos || 
            lowerResponse.find("http/3") != std::string::npos ||
            lowerResponse.find("alt-svc") != std::string::npos) {
            result.success = true;
            result.version = "HTTP/3";
            result.serverName = getQuicServerName(&response);
        } else {
            result.error = "Not HTTP/3 over QUIC";
        }
    } else {
        result.error = "Connection failed";
    }
    
    return result;
}
