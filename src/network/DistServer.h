#ifndef DISTSERVER_H
#define DISTSERVER_H

#include <string>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

class DistServer {
public:
    DistServer(const std::string &host, int port);
    ~DistServer();

    // Load tasks (IPs) into queue
    void loadTasks(const std::deque<std::string> &ips);

    // Start server (non-blocking); stop in destructor
    void start();

private:
    void run();
    std::string nextBatch(size_t n);

    std::string host_;
    int port_;
    std::deque<std::string> tasks_;
    std::mutex mtx_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};

#endif // DISTSERVER_H


