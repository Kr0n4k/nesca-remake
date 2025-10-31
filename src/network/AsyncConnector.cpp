#include "AsyncConnector.h"

#include <future>
#include <chrono>

#include <externData.h>

using boost::asio::ip::tcp;

AsyncConnector &AsyncConnector::instance() {
    static AsyncConnector inst;
    return inst;
}

AsyncConnector::AsyncConnector()
    : workGuard_(std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(ioContext_))) {
    ensureThreads();
}

AsyncConnector::~AsyncConnector() {
    workGuard_.reset();
    ioContext_.stop();
    for (auto &t : workers_) {
        if (t.joinable()) t.join();
    }
}

void AsyncConnector::ensureThreads() {
    if (!workers_.empty()) return;
    unsigned int threadCount = std::max(2u, std::thread::hardware_concurrency());
    if (gAsioThreads > 0) threadCount = static_cast<unsigned int>(gAsioThreads);
    workers_.reserve(threadCount);
    for (unsigned int i = 0; i < threadCount; ++i) {
        workers_.emplace_back([this]() { ioContext_.run(); });
    }
}

void AsyncConnector::asyncConnect(const std::string &host,
                                  int port,
                                  int timeoutMs,
                                  const ConnectCallback &cb) {
    ensureThreads();

    auto resolver = std::make_shared<tcp::resolver>(ioContext_.get_executor());
    auto socket   = std::make_shared<tcp::socket>(ioContext_.get_executor());
    auto timerDns = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto timerConn = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto cbWrapper = std::make_shared<ConnectCallback>(cb);

    // State to ensure single completion
    auto completed = std::make_shared<bool>(false);

    // Resolve timeout
    int effectiveOverall = timeoutMs > 0 ? timeoutMs : 1000;
    int dnsTimeout = (gAsioDnsTimeoutMs > 0) ? gAsioDnsTimeoutMs : effectiveOverall;
    int connTimeout = (gAsioConnectTimeoutMs > 0) ? gAsioConnectTimeoutMs : effectiveOverall;

    timerDns->expires_after(std::chrono::milliseconds(dnsTimeout));
    timerDns->async_wait([socket, timerDns, cbWrapper, completed](const boost::system::error_code &ec){
        if (*completed) return;
        if (!ec) {
            *completed = true;
            boost::system::error_code ignore;
            socket->close(ignore);
            (*cbWrapper)(false, "dns_timeout");
        }
    });

    // Resolve then connect
    resolver->async_resolve(host, std::to_string(port),
        [this, resolver, socket, timerDns, timerConn, cbWrapper, completed, connTimeout](const boost::system::error_code &ec, tcp::resolver::results_type results){
            if (*completed) return;
            if (ec) {
                *completed = true;
                (*cbWrapper)(false, ec.message());
                return;
            }

            // cancel DNS timer and start connect timer
            {
                timerDns->cancel();
            }
            timerConn->expires_after(std::chrono::milliseconds(connTimeout));
            timerConn->async_wait([socket, timerConn, cbWrapper, completed](const boost::system::error_code &ecTo){
                if (*completed) return;
                if (!ecTo) {
                    *completed = true;
                    boost::system::error_code ignore;
                    socket->close(ignore);
                    (*cbWrapper)(false, "connect_timeout");
                }
            });

            boost::asio::async_connect(*socket, results,
                [socket, timerConn, cbWrapper, completed](const boost::system::error_code &ec2, const tcp::endpoint&){
                    if (*completed) return;
                    *completed = true;
                    timerConn->cancel();
                    if (ec2) {
                        (*cbWrapper)(false, ec2.message());
                    } else {
                        boost::system::error_code ignore;
                        socket->close(ignore);
                        (*cbWrapper)(true, "");
                    }
                }
            );
        }
    );
}

bool AsyncConnector::tryConnect(const std::string &host, int port, int timeoutMs, std::string *errorOut) {
    auto pr = std::make_shared<std::promise<std::pair<bool, std::string>>>();
    auto fut = pr->get_future();
    asyncConnect(host, port, timeoutMs, [pr](bool ok, const std::string &err) {
        pr->set_value({ok, err});
    });
    auto res = fut.get();
    if (errorOut) *errorOut = res.second;
    return res.first;
}


