#ifndef WEBSOCKETAUTH_H
#define WEBSOCKETAUTH_H

#include <string>
#include <vector>

class WebSocketAuth {
public:
    struct WebSocketResult {
        bool success;
        std::string response;
        std::string error;
        int statusCode;
    };
    
    // WebSocket connection and handshake
    static WebSocketResult connectWebSocket(const char* ip, int port, const char* path = "/", bool useSSL = false);
    static WebSocketResult testWebSocketEndpoint(const char* ip, int port, const char* path = "/", bool useSSL = false);
    
    // WebSocket detection in HTTP responses
    static bool detectWebSocketSupport(const std::string* response);
    static bool isWebSocketUpgrade(const std::string* response);
    
    // WebSocket authentication testing
    static WebSocketResult testWebSocketAuth(const char* ip, int port, const char* path, const char* username, const char* password, bool useSSL = false);
    
    // Common WebSocket paths to test
    static std::vector<std::string> getCommonWebSocketPaths();
    
private:
    static std::string generateWebSocketKey();
    static std::string calculateWebSocketAccept(const std::string& key);
    static bool validateWebSocketResponse(const std::string& response, const std::string& expectedAccept);
};

#endif // WEBSOCKETAUTH_H
