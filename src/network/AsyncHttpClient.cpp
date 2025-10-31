#include "AsyncHttpClient.h"

#include <boost/beast/version.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <future>
#include <externData.h>
#include <sstream>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

AsyncHttpClient &AsyncHttpClient::instance() {
    static AsyncHttpClient inst;
    return inst;
}

AsyncHttpClient::AsyncHttpClient()
    : workGuard_(std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(ioContext_))) {
    ensureThreads();
}

AsyncHttpClient::~AsyncHttpClient() {
    workGuard_.reset();
    ioContext_.stop();
    for (auto &t : workers_) {
        if (t.joinable()) t.join();
    }
}

void AsyncHttpClient::ensureThreads() {
    if (!workers_.empty()) return;
    unsigned int threadCount = std::max(2u, std::thread::hardware_concurrency());
    if (gAsioThreads > 0) threadCount = static_cast<unsigned int>(gAsioThreads);
    workers_.reserve(threadCount);
    for (unsigned int i = 0; i < threadCount; ++i) {
        workers_.emplace_back([this]() { ioContext_.run(); });
    }
}

void AsyncHttpClient::asyncGet(const std::string &host,
                               int port,
                               const std::string &path,
                               bool useSSL,
                               int dnsTimeoutMs,
                               int connectTimeoutMs,
                               int overallTimeoutMs,
                               const HttpCallback &cb) {
    ensureThreads();

    auto resolver = std::make_shared<tcp::resolver>(ioContext_.get_executor());
    auto socket   = std::make_shared<tcp::socket>(ioContext_.get_executor());
    auto timerDns = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto timerConn = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto timerAll = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto cbWrap   = std::make_shared<HttpCallback>(cb);
    auto completed= std::make_shared<bool>(false);

    AsyncHttpClient::Response resp;
    auto respPtr = std::make_shared<Response>(resp);

    int effOverall = overallTimeoutMs > 0 ? overallTimeoutMs : 3000;
    int effDns = (gAsioDnsTimeoutMs > 0) ? gAsioDnsTimeoutMs : (dnsTimeoutMs > 0 ? dnsTimeoutMs : effOverall);
    int effConn = (gAsioConnectTimeoutMs > 0) ? gAsioConnectTimeoutMs : (connectTimeoutMs > 0 ? connectTimeoutMs : effOverall);

    // Overall timeout
    timerAll->expires_after(std::chrono::milliseconds(effOverall));
    timerAll->async_wait([socket, cbWrap, completed](const boost::system::error_code &ec){
        if (*completed) return;
        if (!ec) {
            *completed = true;
            boost::system::error_code ignore;
            socket->close(ignore);
            (*cbWrap)(false, "overall_timeout", AsyncHttpClient::Response{});
        }
    });

    // DNS timeout
    timerDns->expires_after(std::chrono::milliseconds(effDns));
    timerDns->async_wait([socket, cbWrap, completed](const boost::system::error_code &ec){
        if (*completed) return;
        if (!ec) {
            *completed = true;
            boost::system::error_code ignore;
            socket->close(ignore);
            (*cbWrap)(false, "dns_timeout", AsyncHttpClient::Response{});
        }
    });

    // Resolve
    resolver->async_resolve(host, std::to_string(port),
        [this, resolver, socket, timerDns, timerConn, timerAll, cbWrap, completed, respPtr, effConn, host, path, useSSL]
        (const boost::system::error_code &ec, tcp::resolver::results_type results){
            if (*completed) return;
            if (ec) {
                *completed = true;
                (*cbWrap)(false, ec.message(), AsyncHttpClient::Response{});
                return;
            }

            // DNS resolved -> cancel DNS timer
            {
                timerDns->cancel();
            }

            // Connect timeout
            timerConn->expires_after(std::chrono::milliseconds(effConn));
            timerConn->async_wait([socket, cbWrap, completed](const boost::system::error_code &ecTo){
                if (*completed) return;
                if (!ecTo) {
                    *completed = true;
                    boost::system::error_code ignore;
                    socket->close(ignore);
                    (*cbWrap)(false, "connect_timeout", AsyncHttpClient::Response{});
                }
            });

            auto sslCtx   = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client);
            auto sslStream= std::make_shared<boost::asio::ssl::stream<tcp::socket&>>(*socket);

            boost::asio::async_connect(*socket, results,
                [this, socket, sslCtx, sslStream, timerConn, timerAll, cbWrap, completed, respPtr, host, path, useSSL]
                (const boost::system::error_code &ec2, const tcp::endpoint&){
                    if (*completed) return;
                    if (ec2) {
                        *completed = true;
                        (*cbWrap)(false, ec2.message(), AsyncHttpClient::Response{});
                        return;
                    }
                    // cancel timers
                    {
                        timerConn->cancel();
                    }

                    auto doHttp = [cbWrap, completed, timerAll, host, path](auto &stream){
                        // Compose GET request
                        http::request<http::string_body> req{http::verb::get, path.empty() ? "/" : path, 11};
                        req.set(http::field::host, host);
                        req.set(http::field::user_agent, (strlen(gUserAgent) > 0 ? gUserAgent : "nesca-asio"));
                        req.set(http::field::accept, "*/*");

                        auto reqPtr = std::make_shared<http::request<http::string_body>>(std::move(req));
                        auto buffer = std::make_shared<boost::beast::flat_buffer>();
                        auto resPtr = std::make_shared<http::response<http::string_body>>();

                        http::async_write(stream, *reqPtr,
                            [&stream, buffer, resPtr, cbWrap, completed, timerAll]
                            (const boost::system::error_code &wec, std::size_t){
                                if (*completed) return;
                                if (wec) {
                                    *completed = true;
                                    (*cbWrap)(false, wec.message(), AsyncHttpClient::Response{});
                                    return;
                                }
                                http::async_read(stream, *buffer, *resPtr,
                                    [&stream, resPtr, cbWrap, completed, timerAll]
                                    (const boost::system::error_code &rec, std::size_t){
                                        if (*completed) return;
                                        *completed = true;
                                        boost::system::error_code ignore;
                                        timerAll->cancel();
                                        stream.next_layer().shutdown(tcp::socket::shutdown_both, ignore);
                                        stream.next_layer().close(ignore);

                                        if (rec) {
                                            (*cbWrap)(false, rec.message(), AsyncHttpClient::Response{});
                                        } else {
                                            AsyncHttpClient::Response out;
                                            out.status = static_cast<int>(resPtr->result_int());
                                            std::ostringstream h;
                                            for (auto const& f : resPtr->base()) {
                                                h << f.name_string() << ": " << f.value() << "\r\n";
                                            }
                                            out.headers = h.str();
                                            out.body = resPtr->body();
                                            (*cbWrap)(true, "", out);
                                        }
                                    }
                                );
                            }
                        );
                    };

                    if (useSSL) {
                        if (gVerifySSL) {
                            sslCtx->set_default_verify_paths();
                            sslStream->set_verify_mode(boost::asio::ssl::verify_peer);
                        } else {
                            sslStream->set_verify_mode(boost::asio::ssl::verify_none);
                        }
                        if(!SSL_set_tlsext_host_name(sslStream->native_handle(), host.c_str())) {
                            *completed = true;
                            (*cbWrap)(false, "sni_error", AsyncHttpClient::Response{});
                            return;
                        }
                        sslStream->async_handshake(boost::asio::ssl::stream_base::client,
                            [sslStream, doHttp](const boost::system::error_code &hec) mutable {
                                if (hec) {
                                    return;
                                }
                                doHttp(*sslStream);
                            }
                        );
                    } else {
                        struct PlainWrapper {
                            tcp::socket &s;
                            tcp::socket & next_layer() { return s; }
                        } plain{*socket};
                        doHttp(plain);
                    }
                }
            );
        }
    );
}

bool AsyncHttpClient::tryGet(const std::string &host,
                             int port,
                             const std::string &path,
                             bool useSSL,
                             int dnsTimeoutMs,
                             int connectTimeoutMs,
                             int overallTimeoutMs,
                             Response *out,
                             std::string *errorOut) {
    auto pr = std::make_shared<std::promise<std::tuple<bool, std::string, Response>>>();
    auto fu = pr->get_future();
    asyncGet(host, port, path, useSSL, dnsTimeoutMs, connectTimeoutMs, overallTimeoutMs,
        [pr](bool ok, const std::string &err, const Response &resp) {
            pr->set_value({ok, err, resp});
        }
    );
    auto result = fu.get();
    bool ok = std::get<0>(result);
    if (errorOut) *errorOut = std::get<1>(result);
    if (out) *out = std::get<2>(result);
    return ok;
}

void AsyncHttpClient::asyncPost(const std::string &host,
                                int port,
                                const std::string &path,
                                bool useSSL,
                                const std::string &body,
                                const std::string &contentType,
                                int dnsTimeoutMs,
                                int connectTimeoutMs,
                                int overallTimeoutMs,
                                const HttpCallback &cb) {
    ensureThreads();

    auto resolver = std::make_shared<tcp::resolver>(ioContext_.get_executor());
    auto socket   = std::make_shared<tcp::socket>(ioContext_.get_executor());
    auto timerDns = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto timerConn = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto timerAll = std::make_shared<boost::asio::steady_timer>(ioContext_.get_executor());
    auto cbWrap   = std::make_shared<HttpCallback>(cb);
    auto completed= std::make_shared<bool>(false);

    int effOverall = overallTimeoutMs > 0 ? overallTimeoutMs : 3000;
    int effDns = (gAsioDnsTimeoutMs > 0) ? gAsioDnsTimeoutMs : (dnsTimeoutMs > 0 ? dnsTimeoutMs : effOverall);
    int effConn = (gAsioConnectTimeoutMs > 0) ? gAsioConnectTimeoutMs : (connectTimeoutMs > 0 ? connectTimeoutMs : effOverall);

    // Overall timeout
    timerAll->expires_after(std::chrono::milliseconds(effOverall));
    timerAll->async_wait([socket, cbWrap, completed](const boost::system::error_code &ec){
        if (*completed) return;
        if (!ec) {
            *completed = true;
            boost::system::error_code ignore;
            socket->close(ignore);
            (*cbWrap)(false, "overall_timeout", AsyncHttpClient::Response{});
        }
    });

    // DNS timeout
    timerDns->expires_after(std::chrono::milliseconds(effDns));
    timerDns->async_wait([socket, cbWrap, completed](const boost::system::error_code &ec){
        if (*completed) return;
        if (!ec) {
            *completed = true;
            boost::system::error_code ignore;
            socket->close(ignore);
            (*cbWrap)(false, "dns_timeout", AsyncHttpClient::Response{});
        }
    });

    // Resolve
    resolver->async_resolve(host, std::to_string(port),
        [this, resolver, socket, timerDns, timerConn, timerAll, cbWrap, completed, host, path, useSSL, body, contentType, effConn]
        (const boost::system::error_code &ec, tcp::resolver::results_type results){
            if (*completed) return;
            if (ec) {
                *completed = true;
                (*cbWrap)(false, ec.message(), AsyncHttpClient::Response{});
                return;
            }
            {
                timerDns->cancel();
            }
            timerConn->expires_after(std::chrono::milliseconds(effConn));
            timerConn->async_wait([socket, cbWrap, completed](const boost::system::error_code &ecTo){
                if (*completed) return;
                if (!ecTo) {
                    *completed = true;
                    boost::system::error_code ignore;
                    socket->close(ignore);
                    (*cbWrap)(false, "connect_timeout", AsyncHttpClient::Response{});
                }
            });
            auto sslCtx   = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client);
            auto sslStream= std::make_shared<boost::asio::ssl::stream<tcp::socket&>>(*socket);
            boost::asio::async_connect(*socket, results,
                [this, socket, sslCtx, sslStream, timerConn, timerAll, cbWrap, completed, host, path, useSSL, body, contentType]
                (const boost::system::error_code &ec2, const tcp::endpoint&){
                    if (*completed) return;
                    if (ec2) {
                        *completed = true;
                        (*cbWrap)(false, ec2.message(), AsyncHttpClient::Response{});
                        return;
                    }
                    {
                        timerConn->cancel();
                    }
                    auto doHttpPost = [cbWrap, completed, timerAll, host, path, body, contentType](auto &stream){
                        http::request<http::string_body> req{http::verb::post, path.empty() ? "/" : path, 11};
                        req.set(http::field::host, host);
                        req.set(http::field::user_agent, (strlen(gUserAgent) > 0 ? gUserAgent : "nesca-asio"));
                        req.set(http::field::accept, "*/*");
                        req.set(http::field::content_type, contentType);
                        req.body() = body;
                        req.prepare_payload();
                        auto reqPtr = std::make_shared<http::request<http::string_body>>(std::move(req));
                        auto buffer = std::make_shared<boost::beast::flat_buffer>();
                        auto resPtr = std::make_shared<http::response<http::string_body>>();
                        http::async_write(stream, *reqPtr,
                            [&stream, buffer, resPtr, cbWrap, completed, timerAll]
                            (const boost::system::error_code &wec, std::size_t){
                                if (*completed) return;
                                if (wec) {
                                    *completed = true;
                                    (*cbWrap)(false, wec.message(), AsyncHttpClient::Response{});
                                    return;
                                }
                                http::async_read(stream, *buffer, *resPtr,
                                    [&stream, resPtr, cbWrap, completed, timerAll]
                                    (const boost::system::error_code &rec, std::size_t){
                                        if (*completed) return;
                                        *completed = true;
                                        boost::system::error_code ignore;
                                        timerAll->cancel();
                                        stream.next_layer().shutdown(tcp::socket::shutdown_both, ignore);
                                        stream.next_layer().close(ignore);
                                        if (rec) {
                                            (*cbWrap)(false, rec.message(), AsyncHttpClient::Response{});
                                        } else {
                                            AsyncHttpClient::Response out;
                                            out.status = static_cast<int>(resPtr->result_int());
                                            std::ostringstream h;
                                            for (auto const& f : resPtr->base()) {
                                                h << f.name_string() << ": " << f.value() << "\r\n";
                                            }
                                            out.headers = h.str();
                                            out.body = resPtr->body();
                                            (*cbWrap)(true, "", out);
                                        }
                                    }
                                );
                            }
                        );
                    };
                    if (useSSL) {
                        if (gVerifySSL) {
                            sslCtx->set_default_verify_paths();
                            sslStream->set_verify_mode(boost::asio::ssl::verify_peer);
                        } else {
                            sslStream->set_verify_mode(boost::asio::ssl::verify_none);
                        }
                        if(!SSL_set_tlsext_host_name(sslStream->native_handle(), host.c_str())) {
                            *completed = true;
                            (*cbWrap)(false, "sni_error", AsyncHttpClient::Response{});
                            return;
                        }
                        sslStream->async_handshake(boost::asio::ssl::stream_base::client,
                            [sslStream, doHttpPost](const boost::system::error_code &hec) mutable {
                                if (hec) {
                                    return;
                                }
                                doHttpPost(*sslStream);
                            }
                        );
                    } else {
                        struct PlainWrapper { tcp::socket &s; tcp::socket & next_layer() { return s; } } plain{*socket};
                        doHttpPost(plain);
                    }
                }
            );
        }
    );
}

bool AsyncHttpClient::tryPost(const std::string &host,
                              int port,
                              const std::string &path,
                              bool useSSL,
                              const std::string &body,
                              const std::string &contentType,
                              int dnsTimeoutMs,
                              int connectTimeoutMs,
                              int overallTimeoutMs,
                              Response *out,
                              std::string *errorOut) {
    auto pr = std::make_shared<std::promise<std::tuple<bool, std::string, Response>>>();
    auto fu = pr->get_future();
    asyncPost(host, port, path, useSSL, body, contentType, dnsTimeoutMs, connectTimeoutMs, overallTimeoutMs,
        [pr](bool ok, const std::string &err, const Response &resp) {
            pr->set_value({ok, err, resp});
        }
    );
    auto result = fu.get();
    bool ok = std::get<0>(result);
    if (errorOut) *errorOut = std::get<1>(result);
    if (out) *out = std::get<2>(result);
    return ok;
}


