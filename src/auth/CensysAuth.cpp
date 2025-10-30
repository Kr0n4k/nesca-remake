#include "CensysAuth.h"
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
#include <QByteArray>

const char* CensysAuth::API_BASE_URL = "https://search.censys.io/api/v2";

// Write callback for libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Get authentication header
std::string CensysAuth::getAuthHeader(const char* apiId, const char* apiSecret) {
    if (!apiId || !apiSecret || strlen(apiId) == 0 || strlen(apiSecret) == 0) {
        return "";
    }
    
    std::string credentials = std::string(apiId) + ":" + std::string(apiSecret);
    QByteArray credBytes = QByteArray::fromStdString(credentials);
    QByteArray encoded = credBytes.toBase64();
    
    return std::string("Basic ") + encoded.toStdString();
}

// Make API request
std::string CensysAuth::makeRequest(const char* apiId, const char* apiSecret, 
                                    const char* endpoint, const char* params) {
    if (!apiId || !apiSecret || strlen(apiId) == 0 || strlen(apiSecret) == 0) {
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
    std::string authHeader = getAuthHeader(apiId, apiSecret);
    
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader.c_str());
    
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

// Test API credentials
bool CensysAuth::testApiKey(const char* apiId, const char* apiSecret) {
    if (!apiId || !apiSecret || strlen(apiId) == 0 || strlen(apiSecret) == 0) {
        return false;
    }
    
    std::string result = makeRequest(apiId, apiSecret, "/account", "");
    
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
    if (obj.contains("error") || obj.contains("status_code")) {
        return false;
    }
    
    return true;
}

// Get account information
std::string CensysAuth::getAccountInfo(const char* apiId, const char* apiSecret) {
    return makeRequest(apiId, apiSecret, "/account", "");
}

// Search for hosts
CensysSearchResult CensysAuth::searchHosts(const char* apiId, const char* apiSecret, 
                                           const char* query, int page, int perPage) {
    CensysSearchResult result;
    result.success = false;
    
    if (!apiId || !apiSecret || !query || strlen(apiId) == 0 || strlen(apiSecret) == 0 || strlen(query) == 0) {
        result.error = "Invalid API credentials or query";
        return result;
    }
    
    std::ostringstream params;
    params << "q=" << curl_easy_escape(nullptr, query, strlen(query));
    params << "&per_page=" << perPage;
    params << "&page=" << page;
    
    std::string response = makeRequest(apiId, apiSecret, "/hosts/search", params.str().c_str());
    
    if (response.empty()) {
        result.error = "Empty response from API";
        return result;
    }
    
    result.jsonResponse = response;
    
    // Parse JSON response
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response));
    if (doc.isNull() || doc.isEmpty()) {
        result.error = "Invalid JSON response";
        return result;
    }
    
    QJsonObject root = doc.object();
    
    if (root.contains("status") && root["status"].toString() != "OK") {
        result.error = root.value("error").toString().toStdString();
        return result;
    }
    
    result.success = true;
    result.total = root.value("result").toObject().value("total").toInt();
    result.matches = parseHostsFromJsonArray(response);
    
    return result;
}

// Parse hosts from JSON array
std::vector<CensysHost> CensysAuth::parseHostsFromJsonArray(const std::string& json) {
    std::vector<CensysHost> hosts;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (doc.isNull() || doc.isEmpty()) {
        return hosts;
    }
    
    QJsonObject root = doc.object();
    QJsonObject result = root.value("result").toObject();
    QJsonArray hits = result.value("hits").toArray();
    
    for (const QJsonValue& hit : hits) {
        QJsonObject hitObj = hit.toObject();
        CensysHost host;
        
        host.ip = hitObj.value("ip").toString().toStdString();
        host.hostname = hitObj.value("name").toString().toStdString();
        host.timestamp = hitObj.value("last_seen").toString().toStdString();
        
        // Parse location
        QJsonObject location = hitObj.value("location").toObject();
        host.city = location.value("city").toString().toStdString();
        host.country = location.value("country").toString().toStdString();
        
        // Parse autonomous system
        QJsonObject asn = hitObj.value("autonomous_system").toObject();
        host.autonomousSystem = asn.value("name").toString().toStdString();
        host.org = asn.value("org").toString().toStdString();
        
        // Parse services
        QJsonArray services = hitObj.value("services").toArray();
        for (const QJsonValue& service : services) {
            QJsonObject serviceObj = service.toObject();
            host.port = serviceObj.value("port").toInt();
            host.protocol = serviceObj.value("transport_protocol").toString().toStdString();
            host.service = serviceObj.value("service_name").toString().toStdString();
            host.banner = serviceObj.value("banner").toString().toStdString();
            
            hosts.push_back(host);
        }
    }
    
    return hosts;
}

// Parse host from JSON
CensysHost CensysAuth::parseHostFromJson(const std::string& json) {
    CensysHost host;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (doc.isNull() || doc.isEmpty()) {
        return host;
    }
    
    QJsonObject root = doc.object();
    host.ip = root.value("ip").toString().toStdString();
    host.hostname = root.value("name").toString().toStdString();
    
    return host;
}

// Get host information
CensysHostInfo CensysAuth::parseHostInfoFromJson(const std::string& json) {
    CensysHostInfo info;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (doc.isNull() || doc.isEmpty()) {
        return info;
    }
    
    QJsonObject root = doc.object();
    info.ip = root.value("ip").toString().toStdString();
    info.hostname = root.value("name").toString().toStdString();
    
    // Parse location
    QJsonObject location = root.value("location").toObject();
    info.city = location.value("city").toString().toStdString();
    info.country = location.value("country").toString().toStdString();
    
    // Parse autonomous system
    QJsonObject asn = root.value("autonomous_system").toObject();
    info.autonomousSystem = asn.value("name").toString().toStdString();
    info.org = asn.value("org").toString().toStdString();
    
    // Parse services
    QJsonArray services = root.value("services").toArray();
    for (const QJsonValue& service : services) {
        QJsonObject serviceObj = service.toObject();
        CensysHost host;
        host.ip = info.ip;
        host.port = serviceObj.value("port").toInt();
        host.protocol = serviceObj.value("transport_protocol").toString().toStdString();
        host.service = serviceObj.value("service_name").toString().toStdString();
        host.banner = serviceObj.value("banner").toString().toStdString();
        
        info.services.push_back(host);
    }
    
    return info;
}

// Build query string
std::string CensysAuth::buildQuery(const char* query, const char* filters) {
    std::string result = query ? query : "";
    
    if (filters && strlen(filters) > 0) {
        if (!result.empty()) {
            result += " AND ";
        }
        result += filters;
    }
    
    return result;
}

// Build camera query
std::string CensysAuth::buildCameraQuery() {
    return "(services.service_name:http OR services.service_name:https) AND "
           "(services.banner:*camera* OR services.banner:*hikvision* OR services.banner:*dahua* "
           "OR services.banner:*axis* OR services.banner:*onvif* OR services.banner:*axis*)";
}

// Build server query
std::string CensysAuth::buildServerQuery() {
    return "(services.service_name:http OR services.service_name:https OR "
           "services.service_name:ssh OR services.service_name:ftp OR "
           "services.service_name:telnet)";
}

// Build IoT query
std::string CensysAuth::buildIoTQuery() {
    return "(services.service_name:http OR services.service_name:https) AND "
           "(services.banner:*iot* OR services.banner:*smart* OR services.banner:*device* "
           "OR services.banner:*hub* OR services.banner:*bridge*)";
}

// Build vulnerable query
std::string CensysAuth::buildVulnerableQuery() {
    return "vulnerabilities.count:>0";
}

// Search for specific port
CensysSearchResult CensysAuth::searchPort(const char* apiId, const char* apiSecret, 
                                          const char* port, const char* query, int page) {
    std::string q = "services.port:";
    q += port;
    
    if (query && strlen(query) > 0) {
        q += " AND ";
        q += query;
    }
    
    return searchHosts(apiId, apiSecret, q.c_str(), page);
}

// Search certificates
CensysCertificateResult CensysAuth::searchCertificates(const char* apiId, const char* apiSecret, 
                                                        const char* query, int page) {
    CensysCertificateResult result;
    result.success = false;
    
    if (!apiId || !apiSecret || !query) {
        result.error = "Invalid API credentials or query";
        return result;
    }
    
    std::ostringstream params;
    params << "q=" << curl_easy_escape(nullptr, query, strlen(query));
    params << "&per_page=100";
    params << "&page=" << page;
    
    std::string response = makeRequest(apiId, apiSecret, "/certificates/search", params.str().c_str());
    
    if (response.empty()) {
        result.error = "Empty response from API";
        return result;
    }
    
    result.jsonResponse = response;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response));
    if (doc.isNull() || doc.isEmpty()) {
        result.error = "Invalid JSON response";
        return result;
    }
    
    QJsonObject root = doc.object();
    
    if (root.contains("status") && root["status"].toString() != "OK") {
        result.error = root.value("error").toString().toStdString();
        return result;
    }
    
    result.success = true;
    
    // Parse certificate data
    QJsonArray hits = root.value("result").toObject().value("hits").toArray();
    if (!hits.isEmpty()) {
        QJsonObject firstCert = hits[0].toObject();
        result.certificate.fingerprint = firstCert.value("fingerprint").toString().toStdString();
        
        QJsonObject parsed = firstCert.value("parsed").toObject();
        result.certificate.issuer = parsed.value("issuer").toArray()[0].toString().toStdString();
        result.certificate.subject = parsed.value("subject").toArray()[0].toString().toStdString();
    }
    
    return result;
}

// Get certificate details
CensysCertificateResult CensysAuth::getCertificateDetails(const char* apiId, const char* apiSecret, 
                                                           const char* fingerprint) {
    CensysCertificateResult result;
    result.success = false;
    
    if (!apiId || !apiSecret || !fingerprint) {
        result.error = "Invalid API credentials or fingerprint";
        return result;
    }
    
    std::string endpoint = "/certificates/";
    endpoint += fingerprint;
    
    std::string response = makeRequest(apiId, apiSecret, endpoint.c_str(), "");
    
    if (response.empty()) {
        result.error = "Empty response from API";
        return result;
    }
    
    result.jsonResponse = response;
    
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response));
    if (doc.isNull() || doc.isEmpty()) {
        result.error = "Invalid JSON response";
        return result;
    }
    
    QJsonObject root = doc.object();
    
    if (root.contains("status") && root["status"].toString() != "OK") {
        result.error = root.value("error").toString().toStdString();
        return result;
    }
    
    result.success = true;
    result.certificate.fingerprint = root.value("fingerprint").toString().toStdString();
    
    QJsonObject parsed = root.value("parsed").toObject();
    result.certificate.issuerDN = parsed.value("issuer_dn").toString().toStdString();
    result.certificate.subjectDN = parsed.value("subject_dn").toString().toStdString();
    result.certificate.serialNumber = parsed.value("serial_number").toString().toStdString();
    
    QJsonArray validity = parsed.value("validity").toArray();
    if (validity.size() >= 2) {
        result.certificate.notBefore = validity[0].toString().toStdString();
        result.certificate.notAfter = validity[1].toString().toStdString();
    }
    
    QJsonArray names = parsed.value("names").toArray();
    for (const QJsonValue& name : names) {
        result.certificate.names.push_back(name.toString().toStdString());
    }
    
    return result;
}

// Search with filters
CensysSearchResult CensysAuth::searchWithFilters(const char* apiId, const char* apiSecret, 
                                                  const char* query, const char* filters, int page) {
    std::string q = buildQuery(query, filters);
    return searchHosts(apiId, apiSecret, q.c_str(), page);
}

