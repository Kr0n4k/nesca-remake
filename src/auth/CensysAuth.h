#ifndef CENSYSAUTH_H
#define CENSYSAUTH_H

#include <string>
#include <vector>
#include <QString>

struct CensysHost {
    std::string ip;
    std::string hostname;
    std::string city;
    std::string country;
    std::string org;
    int port;
    std::string protocol;
    std::string service;
    std::string banner;
    std::string timestamp;
    std::vector<std::string> vulns;
    std::string autonomousSystem;
};

struct CensysHostInfo {
    std::string ip;
    std::string hostname;
    std::string city;
    std::string country;
    std::string org;
    std::string os;
    std::string autonomousSystem;
    std::string location;
    std::vector<CensysHost> services;
    std::vector<std::string> vulns;
    std::string lastUpdate;
};

struct CensysSearchResult {
    bool success;
    std::string error;
    int total;
    std::vector<CensysHost> matches;
    std::string jsonResponse;
};

struct CensysCertificate {
    std::string fingerprint;
    std::string issuer;
    std::string subject;
    std::string issuerDN;
    std::string subjectDN;
    std::string notBefore;
    std::string notAfter;
    std::vector<std::string> names;
    std::string serialNumber;
};

struct CensysCertificateResult {
    bool success;
    std::string error;
    CensysCertificate certificate;
    std::string jsonResponse;
};

class CensysAuth {
public:
    // API base URL
    static const char* API_BASE_URL;
    
    // Test API credentials
    static bool testApiKey(const char* apiId, const char* apiSecret);
    
    // Get account information
    static std::string getAccountInfo(const char* apiId, const char* apiSecret);
    
    // Search for hosts
    static CensysSearchResult searchHosts(const char* apiId, const char* apiSecret, 
                                          const char* query, int page = 1, int perPage = 100);
    
    // Get host information
    static CensysHostInfo parseHostInfoFromJson(const std::string& json);
    
    // Search for specific port
    static CensysSearchResult searchPort(const char* apiId, const char* apiSecret, 
                                        const char* port, const char* query = "", int page = 1);
    
    // Search certificates
    static CensysCertificateResult searchCertificates(const char* apiId, const char* apiSecret, 
                                                      const char* query, int page = 1);
    
    // Get certificate details
    static CensysCertificateResult getCertificateDetails(const char* apiId, const char* apiSecret, 
                                                         const char* fingerprint);
    
    // Search with filters (e.g., "services.port:443 AND services.service_name:HTTP")
    static CensysSearchResult searchWithFilters(const char* apiId, const char* apiSecret, 
                                                const char* query, const char* filters = "", 
                                                int page = 1);
    
    // Helper: Parse JSON response
    static CensysHost parseHostFromJson(const std::string& json);
    
    // Helper: Build query string
    static std::string buildQuery(const char* query, const char* filters = "");
    
    // Helper: Build query for specific device types
    static std::string buildCameraQuery();
    static std::string buildServerQuery();
    static std::string buildIoTQuery();
    static std::string buildVulnerableQuery();
    
private:
    // Make API request
    static std::string makeRequest(const char* apiId, const char* apiSecret, 
                                   const char* endpoint, const char* params = "");
    
    // Parse JSON array
    static std::vector<CensysHost> parseHostsFromJsonArray(const std::string& json);
    
    // Get authentication header
    static std::string getAuthHeader(const char* apiId, const char* apiSecret);
};

#endif // CENSYSAUTH_H

