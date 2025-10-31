#include "DistServer.h"

#include <boost/beast/version.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <sstream>

namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

DistServer::DistServer(const std::string &host, int port)
    : host_(host), port_(port) {}

DistServer::~DistServer() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void DistServer::loadTasks(const std::deque<std::string> &ips) {
    std::lock_guard<std::mutex> lk(mtx_);
    tasks_ = ips;
}

void DistServer::start() {
    if (running_) return;
    running_ = true;
    thread_ = std::thread([this]{ run(); });
}

std::string DistServer::nextBatch(size_t n) {
    std::lock_guard<std::mutex> lk(mtx_);
    if (tasks_.empty()) return std::string();
    std::ostringstream out;
    size_t cnt = 0;
    while (!tasks_.empty() && cnt < n) {
        out << tasks_.front() << "\n";
        tasks_.pop_front();
        ++cnt;
    }
    return out.str();
}

void DistServer::run() {
    boost::asio::io_context ioc;
    tcp::acceptor acceptor{ioc.get_executor(), tcp::endpoint(boost::asio::ip::make_address(host_), static_cast<unsigned short>(port_))};

    while (running_) {
        tcp::socket socket{ioc.get_executor()};
        boost::system::error_code ec;
        acceptor.accept(socket, ec);
        if (ec) continue;

        boost::beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if (ec) { socket.close(); continue; }

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "nesca-dist");
        res.keep_alive(false);

        if (req.method() == http::verb::get && req.target().starts_with("/health")) {
            res.body() = "OK";
        } else if (req.method() == http::verb::get && req.target().starts_with("/task")) {
            // parse size param
            size_t n = 100;
            auto target = std::string(req.target());
            auto pos = target.find("size=");
            if (pos != std::string::npos) {
                n = std::strtoul(target.c_str()+pos+5, nullptr, 10);
                if (n == 0 || n > 10000) n = 100;
            }
            auto data = nextBatch(n);
            if (data.empty()) {
                res.result(http::status::no_content);
            } else {
                res.set(http::field::content_type, "text/plain");
                res.body() = std::move(data);
            }
        } else if (req.method() == http::verb::post && req.target().starts_with("/result")) {
            res.body() = "OK";
        } else {
            res.result(http::status::not_found);
            res.body() = "not found";
        }

        res.prepare_payload();
        http::write(socket, res, ec);
        socket.shutdown(tcp::socket::shutdown_send, ec);
        socket.close();
    }
}


