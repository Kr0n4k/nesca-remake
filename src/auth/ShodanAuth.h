#ifndef SHODANAUTH_H
#define SHODANAUTH_H

#include <string>
#include <vector>
#include <QString>

struct ShodanHost {
    std::string ip;
    std::string hostname;
    std::string city;
    std::string country;
    std::string org;
    std::string os;
    std::string port;
    std::string product;
    std::string version;
    std::string banner;
    std::string timestamp;
    std::vector<std::string> vulns;
};

struct ShodanHostInfo {
    std::string ip;
    std::string hostname;
    std::string city;
    std::string country;
    std::string org;
    std::string os;
    int portsCount;
    std::vector<std::string> ports;
    std::vector<ShodanHost> services;
    std::vector<std::string> vulns;
    std::string lastUpdate;
};

struct ShodanSearchResult {
    bool success;
    std::string error;
    int total;
    std::vector<ShodanHost> matches;
    std::string jsonResponse;
};

struct ShodanHostInfoResult {
    bool success;
    std::string error;
    ShodanHostInfo hostInfo;
    std::string jsonResponse;
};

class ShodanAuth {
public:
    // API base URL
    static const char* API_BASE_URL;
    
    // Test API key
    static bool testApiKey(const char* apiKey);
    
    // Get account information
    static std::string getAccountInfo(const char* apiKey);
    
    // Search for hosts
    static ShodanSearchResult searchHosts(const char* apiKey, const char* query, int page = 1, bool minify = true);
    
    // Get host information
    static ShodanHostInfoResult getHostInfo(const char* apiKey, const char* ip);
    
    // Search for specific port
    static ShodanSearchResult searchPort(const char* apiKey, const char* port, const char* query = "", int page = 1);
    
    // Count results for a query
    static int countResults(const char* apiKey, const char* query);
    
    // Search with filters (e.g., "port:80 country:US")
    static ShodanSearchResult searchWithFilters(const char* apiKey, const char* query, const char* filters = "", int page = 1);
    
    // Get my IP
    static std::string getMyIP(const char* apiKey);
    
    // Helper: Parse JSON response
    static ShodanHost parseHostFromJson(const std::string& json);
    static ShodanHostInfo parseHostInfoFromJson(const std::string& json);
    
    // Helper: Build query string
    static std::string buildQuery(const char* query, const char* filters = "");
    
private:
    // Make API request
    static std::string makeRequest(const char* apiKey, const char* endpoint, const char* params = "");
    
    // Parse JSON array
    static std::vector<ShodanHost> parseHostsFromJsonArray(const std::string& json);
};

#endif // SHODANAUTH_H
