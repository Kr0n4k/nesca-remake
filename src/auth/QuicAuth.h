#ifndef QUICAUTH_H
#define QUICAUTH_H

#include <string>
#include <vector>

class QuicAuth {
public:
    struct QuicResult {
        bool success;
        std::string response;
        std::string error;
        int statusCode;
        std::string version;
        std::string serverName;
    };
    
    // QUIC connection and handshake
    static QuicResult connectQuic(const char* ip, int port, const char* hostname = nullptr, bool useHttp3 = true);
    static QuicResult testQuicEndpoint(const char* ip, int port, const char* hostname = nullptr);
    
    // QUIC detection in responses
    static bool detectQuicSupport(const std::string* response);
    static bool isQuicResponse(const std::string* response);
    
    // QUIC version detection
    static std::string detectQuicVersion(const std::string* response);
    static std::string getQuicServerName(const std::string* response);
    
    // QUIC authentication testing
    static QuicResult testQuicAuth(const char* ip, int port, const char* hostname, const char* username, const char* password);
    
    // Common QUIC ports and configurations
    static std::vector<int> getCommonQuicPorts();
    static std::vector<std::string> getQuicVersions();
    
    // HTTP/3 over QUIC
    static QuicResult testHttp3OverQuic(const char* ip, int port, const char* hostname, const char* path = "/");
    
private:
    static std::string generateQuicClientHello();
    static bool validateQuicResponse(const std::string& response);
    static std::string extractQuicVersion(const std::string& response);
    static std::string extractServerName(const std::string& response);
};

#endif // QUICAUTH_H
