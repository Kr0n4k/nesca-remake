#include "WebSocketAuth.h"
#include "Connector.h"
#include "externData.h"
#include "STh.h"
#include <curl/curl.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <QString>

// Base64 encoding function
std::string base64_encode(const unsigned char* data, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string encoded(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return encoded;
}

// Generate random WebSocket key
std::string WebSocketAuth::generateWebSocketKey() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::string key;
    for (int i = 0; i < 16; ++i) {
        key += static_cast<char>(dis(gen));
    }
    
    return base64_encode(reinterpret_cast<const unsigned char*>(key.c_str()), key.length());
}

// Calculate WebSocket accept key
std::string WebSocketAuth::calculateWebSocketAccept(const std::string& key) {
    std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = key + magic;
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
    
    return base64_encode(hash, static_cast<size_t>(SHA_DIGEST_LENGTH));
}

// Validate WebSocket response
bool WebSocketAuth::validateWebSocketResponse(const std::string& response, const std::string& expectedAccept) {
    // Look for "Sec-WebSocket-Accept" header
    std::string searchStr = "Sec-WebSocket-Accept:";
    size_t pos = response.find(searchStr);
    if (pos == std::string::npos) {
        return false;
    }
    
    pos += searchStr.length();
    while (pos < response.length() && (response[pos] == ' ' || response[pos] == '\t')) {
        pos++;
    }
    
    size_t endPos = response.find('\r', pos);
    if (endPos == std::string::npos) {
        endPos = response.find('\n', pos);
    }
    if (endPos == std::string::npos) {
        endPos = response.length();
    }
    
    std::string acceptValue = response.substr(pos, endPos - pos);
    
    // Remove any trailing whitespace
    acceptValue.erase(acceptValue.find_last_not_of(" \t\r\n") + 1);
    
    return acceptValue == expectedAccept;
}

// Detect WebSocket support in HTTP response
bool WebSocketAuth::detectWebSocketSupport(const std::string* response) {
    if (!response || response->empty()) {
        return false;
    }
    
    std::string lowerResponse = *response;
    std::transform(lowerResponse.begin(), lowerResponse.end(), lowerResponse.begin(), ::tolower);
    
    return (lowerResponse.find("websocket") != std::string::npos ||
            lowerResponse.find("upgrade: websocket") != std::string::npos ||
            lowerResponse.find("sec-websocket") != std::string::npos ||
            lowerResponse.find("101 switching protocols") != std::string::npos);
}

// Check if response is WebSocket upgrade
bool WebSocketAuth::isWebSocketUpgrade(const std::string* response) {
    if (!response || response->empty()) {
        return false;
    }
    
    std::string lowerResponse = *response;
    std::transform(lowerResponse.begin(), lowerResponse.end(), lowerResponse.begin(), ::tolower);
    
    return (lowerResponse.find("upgrade: websocket") != std::string::npos &&
            lowerResponse.find("101") != std::string::npos);
}

// Connect to WebSocket endpoint
WebSocketAuth::WebSocketResult WebSocketAuth::connectWebSocket(const char* ip, int port, const char* path, bool useSSL) {
    WebSocketResult result;
    result.success = false;
    result.statusCode = 0;
    
    if (!gWebSocketEnabled) {
        result.error = "WebSocket scanning is disabled";
        return result;
    }
    
    // Generate WebSocket key
    std::string wsKey = generateWebSocketKey();
    std::string expectedAccept = calculateWebSocketAccept(wsKey);
    
    // Build URL
    std::string protocol = useSSL ? "wss://" : "ws://";
    std::string url = protocol + std::string(ip) + ":" + std::to_string(port) + std::string(path);
    
    // Prepare custom headers
    std::vector<std::string> headers;
    headers.push_back("Upgrade: websocket");
    headers.push_back("Connection: Upgrade");
    headers.push_back("Sec-WebSocket-Key: " + wsKey);
    headers.push_back("Sec-WebSocket-Version: 13");
    
    // Use custom User-Agent if provided
    if (gUserAgent[0] != '\0') {
        headers.push_back(std::string("User-Agent: ") + gUserAgent);
    } else {
        headers.push_back("User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0");
    }
    
    // Connect using existing Connector
    Connector connector;
    std::string response;
    
    int res = connector.nConnect(url.c_str(), port, &response, nullptr, &headers);
    
    if (res > 0) {
        result.response = response;
        result.statusCode = 101; // WebSocket upgrade status
        
        // Check if it's a valid WebSocket upgrade
        if (isWebSocketUpgrade(&response)) {
            if (validateWebSocketResponse(response, expectedAccept)) {
                result.success = true;
            } else {
                result.error = "Invalid WebSocket accept key";
            }
        } else {
            result.error = "Not a WebSocket upgrade response";
        }
    } else {
        result.error = "Connection failed";
    }
    
    return result;
}

// Test WebSocket endpoint
WebSocketAuth::WebSocketResult WebSocketAuth::testWebSocketEndpoint(const char* ip, int port, const char* path, bool useSSL) {
    WebSocketResult result = connectWebSocket(ip, port, path, useSSL);
    
    if (result.success) {
        if (gDebugMode) {
            QString message = QString("[WebSocket] Found WebSocket endpoint: %1:%2%3")
                .arg(ip)
                .arg(port)
                .arg(path);
            stt->doEmitionYellowFoundData(message);
        }
    }
    
    return result;
}

// Test WebSocket authentication
WebSocketAuth::WebSocketResult WebSocketAuth::testWebSocketAuth(const char* ip, int port, const char* path, 
                                                               const char* username, const char* password, bool useSSL) {
    WebSocketResult result;
    result.success = false;
    result.statusCode = 0;
    
    if (!gWebSocketEnabled) {
        result.error = "WebSocket scanning is disabled";
        return result;
    }
    
    // Generate WebSocket key
    std::string wsKey = generateWebSocketKey();
    std::string expectedAccept = calculateWebSocketAccept(wsKey);
    
    // Build URL with authentication
    std::string protocol = useSSL ? "wss://" : "ws://";
    std::string authStr = std::string(username) + ":" + std::string(password);
    std::string url = protocol + authStr + "@" + std::string(ip) + ":" + std::to_string(port) + std::string(path);
    
    // Prepare custom headers
    std::vector<std::string> headers;
    headers.push_back("Upgrade: websocket");
    headers.push_back("Connection: Upgrade");
    headers.push_back("Sec-WebSocket-Key: " + wsKey);
    headers.push_back("Sec-WebSocket-Version: 13");
    
    // Use custom User-Agent if provided
    if (gUserAgent[0] != '\0') {
        headers.push_back(std::string("User-Agent: ") + gUserAgent);
    } else {
        headers.push_back("User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0");
    }
    
    // Connect using existing Connector
    Connector connector;
    std::string response;
    
    int res = connector.nConnect(url.c_str(), port, &response, nullptr, &headers);
    
    if (res > 0) {
        result.response = response;
        result.statusCode = 101;
        
        if (isWebSocketUpgrade(&response)) {
            if (validateWebSocketResponse(response, expectedAccept)) {
                result.success = true;
            } else {
                result.error = "Invalid WebSocket accept key";
            }
        } else {
            result.error = "Not a WebSocket upgrade response";
        }
    } else {
        result.error = "Authentication failed";
    }
    
    return result;
}

// Get common WebSocket paths
std::vector<std::string> WebSocketAuth::getCommonWebSocketPaths() {
    return {
        "/",
        "/ws",
        "/websocket",
        "/socket.io/",
        "/socket.io/?EIO=4&transport=websocket",
        "/api/ws",
        "/api/websocket",
        "/chat",
        "/chat/ws",
        "/live",
        "/live/ws",
        "/stream",
        "/stream/ws",
        "/realtime",
        "/realtime/ws",
        "/notifications",
        "/notifications/ws",
        "/status",
        "/status/ws",
        "/monitor",
        "/monitor/ws"
    };
}
