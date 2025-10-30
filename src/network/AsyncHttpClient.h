#ifndef ASYNCHTTPCLIENT_H
#define ASYNCHTTPCLIENT_H

#include <string>
#include <functional>
#include <memory>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

// Minimal HTTP client over Boost.Beast (plain HTTP only)
class AsyncHttpClient {
public:
    struct Response {
        int status = 0;
        std::string headers;
        std::string body;
    };

    using HttpCallback = std::function<void(bool success, const std::string &error, const Response &resp)>;

    static AsyncHttpClient &instance();

    // Simple GET. host may be IP or hostname. path defaults to "/". Plain HTTP only.
    void asyncGet(const std::string &host,
                  int port,
                  const std::string &path,
                  bool useSSL,
                  int dnsTimeoutMs,
                  int connectTimeoutMs,
                  int overallTimeoutMs,
                  const HttpCallback &cb);

    // Convenience blocking wrapper.
    bool tryGet(const std::string &host,
                int port,
                const std::string &path,
                bool useSSL,
                int dnsTimeoutMs,
                int connectTimeoutMs,
                int overallTimeoutMs,
                Response *out,
                std::string *errorOut = nullptr);

    // Simple POST with body and content-type
    void asyncPost(const std::string &host,
                   int port,
                   const std::string &path,
                   bool useSSL,
                   const std::string &body,
                   const std::string &contentType,
                   int dnsTimeoutMs,
                   int connectTimeoutMs,
                   int overallTimeoutMs,
                   const HttpCallback &cb);

    bool tryPost(const std::string &host,
                 int port,
                 const std::string &path,
                 bool useSSL,
                 const std::string &body,
                 const std::string &contentType,
                 int dnsTimeoutMs,
                 int connectTimeoutMs,
                 int overallTimeoutMs,
                 Response *out,
                 std::string *errorOut = nullptr);

private:
    AsyncHttpClient();
    ~AsyncHttpClient();

    void ensureThreads();

    boost::asio::io_context ioContext_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuard_;
    std::vector<std::thread> workers_;
};

#endif // ASYNCHTTPCLIENT_H


