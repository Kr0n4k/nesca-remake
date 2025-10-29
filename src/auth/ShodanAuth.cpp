#include "ShodanAuth.h"
#include "Connector.h"
#include "externData.h"
#include "STh.h"
#include <curl/curl.h>
#include <sstream>
#include <cstring>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <algorithm>

const char* ShodanAuth::API_BASE_URL = "https://api.shodan.io";

// Write callback for libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Make API request
std::string ShodanAuth::makeRequest(const char* apiKey, const char* endpoint, const char* params) {
    if (!apiKey || strlen(apiKey) == 0) {
        return "";
    }
    
    std::string url = std::string(API_BASE_URL) + endpoint;
    if (params && strlen(params) > 0) {
        url += "?" + std::string(params);
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }
    
    std::string response;
    std::string header = std::string("X-API-Key: ") + apiKey;
    
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, header.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    
    if (!gVerifySSL) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return "";
    }
    
    return response;
}

// Test API key
bool ShodanAuth::testApiKey(const char* apiKey) {
    if (!apiKey || strlen(apiKey) == 0) {
        return false;
    }
    
    std::string result = makeRequest(apiKey, "/account/profile", "");
    
    if (result.empty()) {
        return false;
    }
    
    // Check if we got a valid JSON response (not an error)
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(result));
    if (doc.isNull() || doc.isEmpty()) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    // Check for error field
    if (obj.contains("error")) {
        return false;
    }
    
    return true;
}

// Get account information
std::string ShodanAuth::getAccountInfo(const char* apiKey) {
    return makeRequest(apiKey, "/account/profile", "");
}

// Get my IP
std::string ShodanAuth::getMyIP(const char* apiKey) {
    return makeRequest(apiKey, "/tools/myip", "");
}

// Build query string
std::string ShodanAuth::buildQuery(const char* query, const char* filters) {
    std::ostringstream oss;
    
    if (query && strlen(query) > 0) {
        oss << "query=";
        // URL encode
        std::string q = query;
        for (size_t i = 0; i < q.length(); ++i) {
            if (q[i] == ' ') {
                oss << '+';
            } else if (q[i] == '+') {
                oss << "%2B";
            } else if (q[i] == '&') {
                oss << "%26";
            } else {
                oss << q[i];
            }
        }
    }
    
    if (filters && strlen(filters) > 0) {
        if (query && strlen(query) > 0) {
            oss << "&";
        }
        oss << filters;
    }
    
    return oss.str();
}

// Search for hosts
ShodanSearchResult ShodanAuth::searchHosts(const char* apiKey, const char* query, int page, bool minify) {
    ShodanSearchResult result;
    result.success = false;
    result.total = 0;
    result.error = "";
    
    if (!apiKey || strlen(apiKey) == 0) {
        result.error = "API key is required";
        return result;
    }
    
    std::ostringstream params;
    params << "query=";
    
    std::string q = query ? query : "";
    // Simple URL encoding
    for (size_t i = 0; i < q.length(); ++i) {
        if (q[i] == ' ') {
            params << '+';
        } else if (q[i] == '+') {
            params << "%2B";
        } else {
            params << q[i];
        }
    }
    
    params << "&page=" << page;
    if (minify) {
        params << "&minify=true";
    }
    
    std::string response = makeRequest(apiKey, "/shodan/host/search", params.str().c_str());
    
    if (response.empty()) {
        result.error = "Request failed or empty response";
        return result;
    }
    
    result.jsonResponse = response;
    
    // Parse JSON response
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response));
    if (doc.isNull() || doc.isEmpty()) {
        result.error = "Invalid JSON response";
        return result;
    }
    
    QJsonObject obj = doc.object();
    
    // Check for error
    if (obj.contains("error")) {
        result.error = obj["error"].toString().toStdString();
        return result;
    }
    
    result.success = true;
    
    // Get total count
    if (obj.contains("total")) {
        result.total = obj["total"].toInt();
    }
    
    // Get matches
    if (obj.contains("matches")) {
        QJsonArray matches = obj["matches"].toArray();
        for (int i = 0; i < matches.size(); ++i) {
            QJsonObject match = matches[i].toObject();
            ShodanHost host;
            
            if (match.contains("ip_str")) {
                host.ip = match["ip_str"].toString().toStdString();
            }
            if (match.contains("hostnames")) {
                QJsonArray hostnames = match["hostnames"].toArray();
                if (hostnames.size() > 0) {
                    host.hostname = hostnames[0].toString().toStdString();
                }
            }
            if (match.contains("location")) {
                QJsonObject location = match["location"].toObject();
                if (location.contains("city")) {
                    host.city = location["city"].toString().toStdString();
                }
                if (location.contains("country_name")) {
                    host.country = location["country_name"].toString().toStdString();
                }
            }
            if (match.contains("org")) {
                host.org = match["org"].toString().toStdString();
            }
            if (match.contains("os")) {
                host.os = match["os"].toString().toStdString();
            }
            if (match.contains("port")) {
                host.port = QString::number(match["port"].toInt()).toStdString();
            }
            if (match.contains("product")) {
                host.product = match["product"].toString().toStdString();
            }
            if (match.contains("version")) {
                host.version = match["version"].toString().toStdString();
            }
            if (match.contains("data")) {
                host.banner = match["data"].toString().toStdString();
            }
            if (match.contains("timestamp")) {
                host.timestamp = match["timestamp"].toString().toStdString();
            }
            if (match.contains("vulns")) {
                QJsonArray vulns = match["vulns"].toArray();
                for (int j = 0; j < vulns.size(); ++j) {
                    host.vulns.push_back(vulns[j].toString().toStdString());
                }
            }
            
            result.matches.push_back(host);
        }
    }
    
    return result;
}

// Get host information
ShodanHostInfoResult ShodanAuth::getHostInfo(const char* apiKey, const char* ip) {
    ShodanHostInfoResult result;
    result.success = false;
    result.error = "";
    
    if (!apiKey || strlen(apiKey) == 0) {
        result.error = "API key is required";
        return result;
    }
    
    if (!ip || strlen(ip) == 0) {
        result.error = "IP address is required";
        return result;
    }
    
    std::string endpoint = std::string("/shodan/host/") + ip;
    std::string response = makeRequest(apiKey, endpoint.c_str(), "");
    
    if (response.empty()) {
        result.error = "Request failed or empty response";
        return result;
    }
    
    result.jsonResponse = response;
    
    // Parse JSON response
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response));
    if (doc.isNull() || doc.isEmpty()) {
        result.error = "Invalid JSON response";
        return result;
    }
    
    QJsonObject obj = doc.object();
    
    // Check for error
    if (obj.contains("error")) {
        result.error = obj["error"].toString().toStdString();
        return result;
    }
    
    result.success = true;
    result.hostInfo = parseHostInfoFromJson(response);
    
    return result;
}

// Count results for a query
int ShodanAuth::countResults(const char* apiKey, const char* query) {
    if (!apiKey || strlen(apiKey) == 0 || !query || strlen(query) == 0) {
        return -1;
    }
    
    std::ostringstream params;
    params << "query=";
    
    std::string q = query;
    for (size_t i = 0; i < q.length(); ++i) {
        if (q[i] == ' ') {
            params << '+';
        } else {
            params << q[i];
        }
    }
    
    std::string response = makeRequest(apiKey, "/shodan/host/count", params.str().c_str());
    
    if (response.empty()) {
        return -1;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response));
    if (doc.isNull() || doc.isEmpty()) {
        return -1;
    }
    
    QJsonObject obj = doc.object();
    if (obj.contains("total")) {
        return obj["total"].toInt();
    }
    
    return -1;
}

// Search with filters
ShodanSearchResult ShodanAuth::searchWithFilters(const char* apiKey, const char* query, const char* filters, int page) {
    std::string fullQuery = query ? query : "";
    if (filters && strlen(filters) > 0) {
        if (!fullQuery.empty()) {
            fullQuery += " " + std::string(filters);
        } else {
            fullQuery = filters;
        }
    }
    
    return searchHosts(apiKey, fullQuery.empty() ? nullptr : fullQuery.c_str(), page, true);
}

// Parse host from JSON
ShodanHost ShodanAuth::parseHostFromJson(const std::string& json) {
    ShodanHost host;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (doc.isNull() || !doc.isObject()) {
        return host;
    }
    
    QJsonObject obj = doc.object();
    
    if (obj.contains("ip_str")) {
        host.ip = obj["ip_str"].toString().toStdString();
    }
    if (obj.contains("hostnames")) {
        QJsonArray hostnames = obj["hostnames"].toArray();
        if (hostnames.size() > 0) {
            host.hostname = hostnames[0].toString().toStdString();
        }
    }
    
    return host;
}

// Parse host info from JSON
ShodanHostInfo ShodanAuth::parseHostInfoFromJson(const std::string& json) {
    ShodanHostInfo info;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (doc.isNull() || !doc.isObject()) {
        return info;
    }
    
    QJsonObject obj = doc.object();
    
    if (obj.contains("ip_str")) {
        info.ip = obj["ip_str"].toString().toStdString();
    }
    if (obj.contains("hostnames")) {
        QJsonArray hostnames = obj["hostnames"].toArray();
        if (hostnames.size() > 0) {
            info.hostname = hostnames[0].toString().toStdString();
        }
    }
    if (obj.contains("city")) {
        info.city = obj["city"].toString().toStdString();
    }
    if (obj.contains("country_name")) {
        info.country = obj["country_name"].toString().toStdString();
    }
    if (obj.contains("org")) {
        info.org = obj["org"].toString().toStdString();
    }
    if (obj.contains("os")) {
        info.os = obj["os"].toString().toStdString();
    }
    if (obj.contains("data")) {
        QJsonArray dataArray = obj["data"].toArray();
        info.portsCount = dataArray.size();
        
        for (int i = 0; i < dataArray.size(); ++i) {
            QJsonObject service = dataArray[i].toObject();
            ShodanHost host = parseHostFromJson(QJsonDocument(service).toJson(QJsonDocument::Compact).toStdString());
            
            if (service.contains("port")) {
                std::string port = QString::number(service["port"].toInt()).toStdString();
                info.ports.push_back(port);
                host.port = port;
            }
            if (service.contains("product")) {
                host.product = service["product"].toString().toStdString();
            }
            if (service.contains("version")) {
                host.version = service["version"].toString().toStdString();
            }
            if (service.contains("data")) {
                host.banner = service["data"].toString().toStdString();
            }
            
            info.services.push_back(host);
        }
    }
    
    return info;
}

// Search for specific port
ShodanSearchResult ShodanAuth::searchPort(const char* apiKey, const char* port, const char* query, int page) {
    std::string fullQuery = std::string("port:") + port;
    if (query && strlen(query) > 0) {
        fullQuery += " " + std::string(query);
    }
    
    return searchHosts(apiKey, fullQuery.c_str(), page, true);
}

// Parse hosts from JSON array
std::vector<ShodanHost> ShodanAuth::parseHostsFromJsonArray(const std::string& json) {
    std::vector<ShodanHost> hosts;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (doc.isNull() || !doc.isArray()) {
        return hosts;
    }
    
    QJsonArray array = doc.array();
    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array[i].toObject();
        ShodanHost host = parseHostFromJson(QJsonDocument(obj).toJson(QJsonDocument::Compact).toStdString());
        hosts.push_back(host);
    }
    
    return hosts;
}
