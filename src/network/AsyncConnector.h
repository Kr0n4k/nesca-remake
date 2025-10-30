#ifndef ASYNCCONNECTOR_H
#define ASYNCCONNECTOR_H

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

// Small, header-only facade to perform async TCP connects with timeout.
// Provides both callback-based and blocking helpers built on Boost.Asio.
class AsyncConnector {
public:
    using ConnectCallback = std::function<void(bool success, const std::string &error)>;

    // Singleton accessor to reuse io_context and threads across calls
    static AsyncConnector &instance();

    // Fire-and-callback async connect with timeout (milliseconds)
    // Thread-safe; schedules work on internal io_context
    void asyncConnect(const std::string &host,
                      int port,
                      int timeoutMs,
                      const ConnectCallback &cb);

    // Convenience blocking wrapper (waits until callback completes)
    bool tryConnect(const std::string &host, int port, int timeoutMs, std::string *errorOut = nullptr);

private:
    AsyncConnector();
    ~AsyncConnector();

    // Non-copyable
    AsyncConnector(const AsyncConnector&) = delete;
    AsyncConnector& operator=(const AsyncConnector&) = delete;

    void ensureThreads();

    boost::asio::io_context ioContext_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuard_;
    std::vector<std::thread> workers_;
};

#endif // ASYNCCONNECTOR_H


