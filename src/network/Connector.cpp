#include <Connector.h>
#include <HikvisionLogin.h>
#include <SSHAuth.h>
#include <WebSocketAuth.h>
#include <QuicAuth.h>
#include <externData.h>
#include <errno.h>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <AsyncConnector.h> // For Boost.Asio port check
// #include "Filter.h" // Pantene: Где файл?

//#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
//int _pingMyTarget(const char *ip)
//{
//    HANDLE hIcmpFile;
//    unsigned long ipaddr = INADDR_NONE;
//    DWORD dwRetVal = 0;
//    char SendData[32] = "Data Buffer";
//    LPVOID ReplyBuffer = NULL;
//    DWORD ReplySize = 0;
//
//    ipaddr = inet_addr(ip);
//
//    if (ipaddr == INADDR_NONE)
//    {
//        stt->doEmitionRedFoundData("[Pinger] INADDR_NONE! [" + QString(ip) + "]");
//        return 0;
//    }
//
//    hIcmpFile = IcmpCreateFile();
//    if (hIcmpFile == INVALID_HANDLE_VALUE)
//    {
//        stt->doEmitionRedFoundData("[Pinger] Unable to open handle. [" + QString::number(GetLastError()) + "]");
//        return 0;
//   }
//
//    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
//    ReplyBuffer = (VOID*) malloc(ReplySize);
//    if (ReplyBuffer == NULL)
//    {
//        stt->doEmitionRedFoundData("[Pinger] Unable to allocate memory.");
//        return 0;
//    }
//
//    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
//        NULL, ReplyBuffer, ReplySize, gPingTimeout*1000);
//    if (dwRetVal != 0) {
//        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
//        struct in_addr ReplyAddr;
//        ReplyAddr.S_un.S_addr = pEchoReply->Address;
//        printf("\tSent icmp message to %s\n", "127.0.0.1");
//        if (dwRetVal > 1)
//        {
//            if(gDebugMode) stt->doEmitionYellowFoundData("[Pinger] Received " + QString::number(dwRetVal) + " icmp message responses.");
//        }
//        else
//        {
//            if(gDebugMode) stt->doEmitionYellowFoundData("[Pinger] Received " + QString::number(dwRetVal) + " icmp message responses.");
//        }
//
//        if(gDebugMode) stt->doEmitionYellowFoundData("[Pinger] Received from: " + QString(inet_ntoa( ReplyAddr )) + "; Status = " + QString::number(pEchoReply->Status) + "; Roundtrip time = " + QString::number(pEchoReply->RoundTripTime) + "ms.");
//        return 1;
//    }
//    else
//    {
//        printf("\tCall to IcmpSendEcho failed.\n");
//        printf("\tIcmpSendEcho returned error: %ld\n", GetLastError() );
//        if(gDebugMode) stt->doEmitionRedFoundData("[Pinger] Call to IcmpSendEcho failed. IcmpSendEcho returned error: " + QString::number(GetLastError()));
//        return 0;
//    };
//}
//#else
//int _pingMyTarget(const char *ip)
//{
//    FILE *pipe = popen(("ping -w " + std::to_string(gPingTimeout) + " " + ip).c_str(), "r");
//    if(!pipe) {
//        stt->doEmitionRedFoundData("Ping pipe failed: cannot open pipe.");
//        perror("pipe");
//        return 0;
//    }
//
//    char buffer[128] = {0};
//    std::string result;
//
//    while(!feof(pipe)) {
//        if(fgets(buffer, 128, pipe) != NULL){
//            result += buffer;
//        }
//    }
//    pclose(pipe);
//
//    if(strstr((char*)result.c_str(), "100% packet loss") != NULL) return 0;
//    return 1;
//}
//#endif

struct data {
  char trace_ascii; /* 1 or 0 */
};

// Rate limiting: Simple token bucket implementation
static std::mutex rateLimiterMutex;
static std::chrono::steady_clock::time_point lastRequestTime;
static std::atomic<long long> requestCount(0);
static std::chrono::steady_clock::time_point windowStart;
static bool rateLimiterInitialized = false;

void enforceRateLimit() {
	if (gMaxRate <= 0) {
		return;  // No rate limiting
	}
	
	std::lock_guard<std::mutex> lock(rateLimiterMutex);
	auto now = std::chrono::steady_clock::now();
	
	// Initialize on first call
	if (!rateLimiterInitialized) {
		windowStart = now;
		lastRequestTime = now;
		requestCount = 0;
		rateLimiterInitialized = true;
	}
	
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - windowStart).count();
	
	// Reset window every second
	if (elapsed >= 1000) {
		windowStart = now;
		requestCount = 0;
		lastRequestTime = now;
		// No need to wait if window just reset
		requestCount++;
		return;
	}
	
	// Check if we've exceeded the rate limit
	if (requestCount >= gMaxRate) {
		// Wait until the next window starts
		long long waitTime = 1000 - elapsed;
		if (waitTime > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
			now = std::chrono::steady_clock::now();
			windowStart = now;
			requestCount = 0;
		}
	} else {
		// Calculate minimum time between requests (in milliseconds)
		long long minIntervalMs = (gMaxRate > 0) ? (1000 / gMaxRate) : 0;
		if (minIntervalMs > 0) {
			auto timeSinceLastRequest = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRequestTime).count();
			if (timeSinceLastRequest < minIntervalMs) {
				long long sleepTime = minIntervalMs - timeSinceLastRequest;
				if (sleepTime > 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
					now = std::chrono::steady_clock::now();
				}
			}
		}
	}
	
	lastRequestTime = now;
	requestCount++;
}

static
int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
  if (type == CURLINFO_HEADER_OUT) {
	  //data[size] = '\0';
	  //Activity += strlen(data);
	  data[size] = '\0';
	  QString qData = QString(data);
	  Activity += qData.length();
	  stt->doEmitionAddOutData(qData);
  }
  //else if (type == CURLINFO_HEADER_IN) {
	 // QString qData = QString(data);
	 // Activity += qData.length();
	 // stt->doEmitionAddIncData("", qData);
  //}

  return 0;
}

size_t nWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	if (((std::string*)userp)->size() > 180000) return -1;
	((std::string*)userp)->append((char*)contents, realsize);
	Activity += realsize;
	return realsize;

	//struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	//if (mem->size > 180000) return -1;
	//size_t realsize = size * nmemb;
	//mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);

	//if (mem->memory == NULL) {
	//	stt->doEmitionRedFoundData("not enough memory (realloc returned NULL)\n");
	//	return 0;
	//}

	//memcpy(&(mem->memory[mem->size]), contents, realsize);
	//mem->size += realsize;
	//mem->memory[mem->size] = 0;
	//Activity += realsize;
	//return realsize;
}

int pConnect(const char* ip, const int port, std::string *buffer,
	const char *postData,
	const std::vector<std::string> *customHeaders,
	const std::string *lpString,
	bool digestMode)
{
	// Enforce rate limiting
	enforceRateLimit();
	
	buffer->clear();
	int res = 0;
	int attempts = 0;
	int maxAttempts = gRetries + 1;  // +1 for initial attempt
	
	// Retry loop
	while (attempts < maxAttempts) {
		attempts++;
		CURL *curl = curl_easy_init();

		if (curl != NULL)
		{
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			if (MapWidgetOpened) {
				struct data config;
				config.trace_ascii = 1; /* enable ascii tracing */
				curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
				//curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
				curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			}
			// Construct full URL with protocol
			std::string fullUrl = std::string(ip);
			// Check if URL already has a protocol
			if (fullUrl.find("://") == std::string::npos) {
				// Add http:// prefix if not present
				fullUrl = "http://" + fullUrl;
			}
			curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_PORT, port);
			
			// Use custom User-Agent if provided
			if (gUserAgent[0] != '\0') {
				curl_easy_setopt(curl, CURLOPT_USERAGENT, gUserAgent);
			} else {
				curl_easy_setopt(curl, CURLOPT_USERAGENT,
					"Mozilla/5.0 (X11; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0");
			}
			
			curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
			curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 0L);
			
			// HTTP/2 support
			if (gHttp2Enabled) {
				curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
			}
			
			// SSL verification settings
			if (gVerifySSL) {
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 1L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
			} else {
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			}
			
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
			int proxyPort = std::atoi(gProxyPort);
			if (proxyPort > 0 && proxyPort < 65535) curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyPort);
			curl_easy_setopt(curl, CURLOPT_PROXY, gProxyIP);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
			// Use shorter timeouts for faster scanning
			int connectTimeout = (gTimeOut > 3000) ? 3000 : gTimeOut;  // Max 3 seconds for connect
			int totalTimeout = (gTimeOut > 5000) ? 5000 : gTimeOut + 2;  // Max 5 seconds total
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connectTimeout);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, totalTimeout);
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
			
			// Additional performance optimizations
			curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);
			curl_easy_setopt(curl, CURLOPT_TCP_FASTOPEN, 1L);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 0L);  // No redirects for speed
			curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 60L);
			curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
			curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

			if (postData != NULL) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);

			if (customHeaders != NULL) {

				struct curl_slist *chunk = NULL;
				for (auto &ch : *customHeaders) chunk = curl_slist_append(chunk, ch.c_str());
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
			}

			if (lpString != NULL) {
				curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1L);
				curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, 1L);
				curl_easy_setopt(curl, CURLOPT_USERPWD, lpString->c_str());
				if (digestMode)
				{
					curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_DIGEST);
					res = curl_easy_perform(curl);

					if (port != 21 && lpString != NULL) {
						int pos = Utils::ustrstr(*buffer, "\r\n\r\n");
						if (pos != -1) {
							*buffer = buffer->substr(pos + 4);
						}
					}
				}
				else res = curl_easy_perform(curl);
			}
			else res = curl_easy_perform(curl);

			int sz = buffer->size();

			curl_easy_cleanup(curl);
			
			// If successful, return immediately
			if (res == CURLE_OK || sz > 0) {
				return sz;
			}
			
			// Check if error is retriable
			bool isRetriable = (res == CURLE_OPERATION_TIMEDOUT ||
			                   res == CURLE_COULDNT_CONNECT ||
			                   res == CURLE_SEND_ERROR ||
			                   res == CURLE_RECV_ERROR ||
			                   res == 35 ||  // SSL connect error
			                   res == CURLE_GOT_NOTHING);
			
			// If not retriable or no more attempts, break
			if (!isRetriable || attempts >= maxAttempts) {
				break;
			}
			
			// Small delay before retry (exponential backoff)
			if (attempts < maxAttempts) {
				int delayMs = (1 << (attempts - 1)) * 100;  // 100ms, 200ms, 400ms...
				if (delayMs > 1000) delayMs = 1000;  // Cap at 1 second
				std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
				buffer->clear();  // Clear buffer before retry
			}
		}
		else {
			stt->doEmitionRedFoundData("Curl error: failed to initialize");
			return -1;
		}
	}

	// All retries exhausted - handle final result
	int sz = buffer->size();
	if (res == 35) {
		return -1;
	} else if (res == CURLE_LOGIN_DENIED && port == 21) {
		return -1;
	}
	else if (res == CURLE_OPERATION_TIMEDOUT
		|| res == CURLE_COULDNT_CONNECT
		|| res == CURLE_SEND_ERROR
		|| res == CURLE_RECV_ERROR
		) {
		// Check for buffer/memory issues by attempting to create a socket
		errno = 0;
		SOCKET eNobuffSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (eNobuffSocket == (SOCKET)-1) {
			if (errno == ENOBUFS || errno == ENOMEM) {
				stt->doEmitionRedFoundData("Insufficient buffer/memory space. Sleeping for 10 sec...");
				Sleep(10000);
			}
		} else {
			shutdown(eNobuffSocket, SD_BOTH);
			closesocket(eNobuffSocket);
		}
		return -1;
	}
	else {
		if (res == 6) return -2;
		else if (res != 13 &&
			res != 67 &&
			res != CURLE_GOT_NOTHING &&
			res != 56 &&
			res != 35 &&
			res != 19 &&
			res != 23)
		{
			if (res == 5) {
				stt->doEmitionRedFoundData("The given proxy host could not be resolved.");
				return -2;
			}
			else if (res == 8) {
				return -2;
			}
			else if (res == 18) {
				return -2;
			}
			else if (sz == 0) {
				stt->doEmitionRedFoundData("CURL error: (" + QString::number(res) + ") " + QString(ip));
			}
		}
		return sz;
	}
}
int pConnectRTSP(const char* ip, const int port, std::string *buffer, const std::string *lpString, bool isDigest)
{
	// Enforce rate limiting
	enforceRateLimit();
	
	buffer->clear();
	int res = 0;
	CURL *curl = curl_easy_init();

	if (curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		if (MapWidgetOpened) {
			struct data config;
			config.trace_ascii = 1; /* enable ascii tracing */
			curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
			curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}

		curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_DESCRIBE);
		
		// Use custom User-Agent if provided, otherwise use RTSP default
		if (gUserAgent[0] != '\0') {
			curl_easy_setopt(curl, CURLOPT_USERAGENT, gUserAgent);
		} else {
			curl_easy_setopt(curl, CURLOPT_USERAGENT,
				"LibVLC/2.1.5 (LIVE555 Streaming Media v2014.05.27)");
		}
		
		curl_easy_setopt(curl, CURLOPT_URL, ip);
		curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, ip);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
		
		// HTTP/2 support for RTSP
		if (gHttp2Enabled) {
			curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
		}
		
		// SSL verification settings
		if (gVerifySSL) {
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		} else {
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		
		int proxyPort = std::atoi(gProxyPort);
		if (proxyPort > 0 && proxyPort < 65535) curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyPort);
		curl_easy_setopt(curl, CURLOPT_PROXY, gProxyIP);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, gTimeOut);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, gTimeOut + 3);
		if (isDigest) {
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_DIGEST);
		}
		else {
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
		}
		curl_easy_setopt(curl, CURLOPT_USERPWD, lpString->c_str());

		res = curl_easy_perform(curl);

		int sz = buffer->size();
		curl_easy_cleanup(curl);

		if (res == CURLE_OK || sz > 0) {
			return sz;
		}
		
		return -1;
	}

	stt->doEmitionRedFoundData("Curl error.");
	return -1;
}

void eraser(std::string *buffer, const std::string delim1, const std::string delim2) {
	int pos = -1;
	while ((pos = buffer->find(delim1)) != -1) {
		int ePos = buffer->find(delim2, pos);
		if (ePos != -1) {
			buffer->erase(pos, ePos - pos - 1 + delim2.length());
		}
		else {
			buffer->erase(pos, buffer->length() - pos - 1);
		}
	}
}
void cutoutComments(std::string *buffer) {
	//eraser(buffer, "//", "\n"); //Cant's handle urls: http://bla.bla
	eraser(buffer, "<!--", "-->");
	eraser(buffer, "/*", "*/");
}

// Function to detect HTTP/2 in response headers
bool detectHttp2InResponse(const std::string *buffer) {
	if (buffer->empty()) return false;
	
	// Look for HTTP/2 specific headers or indicators
	return (buffer->find("HTTP/2") != std::string::npos ||
			buffer->find("h2") != std::string::npos ||
			buffer->find("HTTP2") != std::string::npos);
}

// Function to detect WebSocket support in response headers
bool detectWebSocketInResponse(const std::string *buffer) {
	if (buffer->empty()) return false;
	
	// Look for WebSocket specific headers or indicators
	return (buffer->find("Upgrade: websocket") != std::string::npos ||
			buffer->find("Sec-WebSocket") != std::string::npos ||
			buffer->find("101 Switching Protocols") != std::string::npos ||
			buffer->find("websocket") != std::string::npos);
}

// Function to detect QUIC support in response headers
bool detectQuicInResponse(const std::string *buffer) {
	if (buffer->empty()) return false;
	
	// Look for QUIC specific headers or indicators
	return (buffer->find("Alt-Svc: h3") != std::string::npos ||
			buffer->find("quic") != std::string::npos ||
			buffer->find("h3=") != std::string::npos ||
			buffer->find("HTTP/3") != std::string::npos ||
			buffer->find("Q043") != std::string::npos ||
			buffer->find("Q046") != std::string::npos ||
			buffer->find("Q050") != std::string::npos);
}

// WebSocket connection function
int pConnectWebSocket(const char* ip, const int port, std::string *buffer, const char* path = "/", bool useSSL = false) {
	// Enforce rate limiting
	enforceRateLimit();
	
	buffer->clear();
	
	if (!gWebSocketEnabled) {
		return -1;
	}
	
	// Test WebSocket connection
	WebSocketAuth::WebSocketResult result = WebSocketAuth::testWebSocketEndpoint(ip, port, path, useSSL);
	
	if (result.success) {
		*buffer = result.response;
		return buffer->size();
	} else {
		*buffer = "WebSocket connection failed: " + result.error;
		return -1;
	}
}

// QUIC connection function
int pConnectQuic(const char* ip, const int port, std::string *buffer, const char* hostname = nullptr, bool useHttp3 = true) {
	// Enforce rate limiting
	enforceRateLimit();
	
	buffer->clear();
	
	if (!gQuicEnabled) {
		return -1;
	}
	
	// Test QUIC connection
	QuicAuth::QuicResult result = QuicAuth::testQuicEndpoint(ip, port, hostname);
	
	if (result.success) {
		*buffer = result.response;
		return buffer->size();
	} else {
		*buffer = "QUIC connection failed: " + result.error;
		return -1;
	}
}

int Connector::nConnect(const char* ip, const int port, std::string *buffer,
                        const char *postData,
                        const std::vector<std::string> *customHeaders,
                        const std::string *lpString,
						bool digestMode,
						bool isRTSP, bool isDigest){
	int res = 0;
	
	if (!isRTSP) {
		res = pConnect(ip, port, buffer, postData, customHeaders, lpString, digestMode);
	}
	else {
		res = pConnectRTSP(ip, port, buffer, lpString, isDigest);
	}
	cutoutComments(buffer);

	// Check for HTTP/2 response and log if detected
	if (res > 0 && detectHttp2InResponse(buffer)) {
		if (gDebugMode) {
			stt->doEmitionYellowFoundData("[HTTP/2] Detected HTTP/2 response from " + QString(ip));
		}
	}
	
	// Check for WebSocket support and log if detected
	if (res > 0 && detectWebSocketInResponse(buffer)) {
		if (gDebugMode) {
			stt->doEmitionYellowFoundData("[WebSocket] Detected WebSocket support from " + QString(ip));
		}
	}
	
	// Check for QUIC support and log if detected
	if (res > 0 && detectQuicInResponse(buffer)) {
		if (gDebugMode) {
			stt->doEmitionYellowFoundData("[QUIC] Detected QUIC support from " + QString(ip));
		}
	}

	if (MapWidgetOpened) stt->doEmitionAddIncData(QString(ip), QString(buffer->c_str()));
	Activity += buffer->size();

	return res;
}

int Connector::checkIsDigestRTSP(const char *ip, std::string *buffer) {
	// Enforce rate limiting
	enforceRateLimit();
	
	buffer->clear();
	int res = 0;
	CURL *curl = curl_easy_init();

	if (curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		if (MapWidgetOpened) {
			struct data config;
			config.trace_ascii = 1; /* enable ascii tracing */
			curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
			curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}

		curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_DESCRIBE);
		
		// Use custom User-Agent if provided, otherwise use RTSP default
		if (gUserAgent[0] != '\0') {
			curl_easy_setopt(curl, CURLOPT_USERAGENT, gUserAgent);
		} else {
			curl_easy_setopt(curl, CURLOPT_USERAGENT,
				"LibVLC/2.1.5 (LIVE555 Streaming Media v2014.05.27)");
		}
		
		curl_easy_setopt(curl, CURLOPT_URL, ip);
		curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, ip);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
		
		// HTTP/2 support for RTSP
		if (gHttp2Enabled) {
			curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
		}
		
		// SSL verification settings
		if (gVerifySSL) {
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		} else {
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		
		int proxyPort = std::atoi(gProxyPort);
		if (proxyPort > 0 && proxyPort < 65535) curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyPort);
		curl_easy_setopt(curl, CURLOPT_PROXY, gProxyIP);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, gTimeOut);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, gTimeOut + 3);

		res = curl_easy_perform(curl);

		int sz = buffer->size();

		curl_easy_cleanup(curl);
		if (res == CURLE_OK || sz > 0) {
			if (MapWidgetOpened) stt->doEmitionAddIncData(QString(ip), QString(buffer->c_str()));
			Activity += sz;

			if (Utils::ustrstr(buffer, "200 ok") != -1) {
				return 2;
			}
			else if (Utils::ustrstr(buffer, "not found") != -1) {
				return -1;
			}
			else if (Utils::ustrstr(buffer, "digest") != -1) {
				return 1;
			}
			else {
				return 0;
			}
		}

		return -1;
	}

	stt->doEmitionRedFoundData("Curl error.");
	return -1;
}

bool portCheck(const char * sDVRIP, int wDVRPort) {
//	sockaddr_in sa;
//	sa.sin_family = AF_INET;
//	sa.sin_port = htons(wDVRPort);
//
//	hostent *host = NULL;
//#if defined(WIN32)
//	if (inet_addr(sDVRIP) != INADDR_NONE) sa.sin_addr.S_un.S_addr = inet_addr(sDVRIP);
//#else
//	if (inet_addr(sDVRIP) != INADDR_NONE) sa.sin_addr.s_addr = inet_addr(sDVRIP);
//#endif
//	else if (host = gethostbyname(sDVRIP)) ((unsigned long*)&sa.sin_addr)[0] = ((unsigned long**)host->h_addr_list)[0][0];
//	else {
//		stt->doEmitionDebugFoundData("Port check failed - inet_addr failure. [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
//			"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
//		return false;
//	}
//
//	SOCKET sock = INVALID_SOCKET;
//	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (sock == INVALID_SOCKET) return false;
//	else if (ENOBUFS == sock || ENOMEM == sock) {
//		stt->doEmitionRedFoundData("Insufficient buffer/memory space. Sleeping for 10 sec..."); 
//		return false;
//	}
//	
//	int res = connect(sock, (sockaddr*)&sa, sizeof(sa));
//
//	//shutdown(sock, SD_BOTH);
//	//closesocket(sock);
//	int resE = WSAGetLastError();
//	if (res == SOCKET_ERROR) {
//		if (gNegDebugMode)
//		{
//			stt->doEmitionDebugFoundData("Port check failed - SOCKET_ERROR. [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
//				"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
//		}
//		return false;
//	}
//	else {
//		stt->doEmitionDebugFoundData("WSAGetLastError1: " + QString::number(resE) + "socket: " + QString::number(sock) + " [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
//			"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
//		char tBuff[1] = { 0 };
//		int recvCode = send(sock, tBuff, 0, 0);
//		resE = WSAGetLastError();
//		stt->doEmitionDebugFoundData("WSAGetLastError2: " + QString::number(resE) + ") [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
//			"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
//		if (-1 == recvCode) {
//			stt->doEmitionDebugFoundData("Port check failed (recv code: " + QString::number(recvCode) + ") [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
//				"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
//			return false;
//		}
//		shutdown(sock, SD_BOTH);
//		closesocket(sock);
//		if (gNegDebugMode)
//		{
//			stt->doEmitionDebugFoundData("Port check succeeded (curl_code: " + QString::number(res) + ") [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
//				"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
//		}
//		return true;
//	}
//
//	if (gNegDebugMode)
//	{
//		stt->doEmitionDebugFoundData("Port check failed - unknown socket error. [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
//			"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
//	}
//	return false;

	// Enforce rate limiting
	enforceRateLimit();
	
	// Choose between Async (Boost.Asio) and curl based on flag
	if (gUseAsioPortCheck) {
		int effectiveTimeout = (gAsioTimeoutMs > 0) ? gAsioTimeoutMs : (gTimeOut * 1000);
		bool success = AsyncConnector::instance().tryConnect(sDVRIP, wDVRPort, effectiveTimeout);
		if (success && gNegDebugMode) {
			stt->doEmitionDebugFoundData("Port check succeeded (asio) [<a href=\"" + QString(sDVRIP) +
				"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + "</font></a>]");
		}
		return success;
	}
	
	// Fallback to curl
	CURL *curl = curl_easy_init();
	if (curl != NULL) {
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, sDVRIP);
		curl_easy_setopt(curl, CURLOPT_PORT, wDVRPort);
		int proxyPort = std::atoi(gProxyPort);
		if (proxyPort > 0 && proxyPort < 65535) curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyPort);
		curl_easy_setopt(curl, CURLOPT_PROXY, gProxyIP);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, gTimeOut);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, gTimeOut);
		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
		int res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (res != CURLE_OK) {
			if (gNegDebugMode)
			{
				/*stt->doEmitionDebugFoundData("Port check failed (curl_code: " + QString::number(res) + ") [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
					"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");*/
				// Check for buffer/memory issues by attempting to create a socket
				errno = 0;
				SOCKET eNobuffSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (eNobuffSocket == (SOCKET)-1) {
					// Socket creation failed, check if it's a buffer/memory issue
					if (errno == ENOBUFS || errno == ENOMEM) {
						stt->doEmitionRedFoundData("Insufficient buffer/memory space. Sleeping for 10 sec...");
						Sleep(10000);
					}
				} else {
					// Socket created successfully, close it
					shutdown(eNobuffSocket, SD_BOTH);
					closesocket(eNobuffSocket);
				}
			}
			return false;
		}
		else {
			if (gNegDebugMode)
			{
				stt->doEmitionDebugFoundData("Port check succeeded (curl_code: " + QString::number(res) + ") [<a href=\"" + QString(sDVRIP) +
					"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + "</font></a>]");
			}
			return true;
		}
	}
	else {
		//if (gNegDebugMode)
		//{
		//	stt->doEmitionDebugFoundData("Port check failed - curl_easy_init() error. [<a href=\"" + QString(sDVRIP) + ":" + QString::number(wDVRPort) +
		//		"/\"><font color=\"#0084ff\">" + QString(sDVRIP) + ":" + QString::number(wDVRPort) + "</font></a>]");
		//}
		return false;
	}
}
int Connector::connectToPort(char* ip, int port)
{
//    if(gPingNScan)
//    {
//        if(_pingMyTarget(ip) == 0) return -2;
//    };

    std::string buffer;
    int size = 0;
	char tempIp[128] = { 0 };
	int sz = strlen(ip);
	if (443 == port) {
		sprintf(tempIp, "https://%s:%d", ip, port);
		//strcpy(tempIp, "https://");
	}
	else if (21 == port) {
		//strcpy(tempIp, "ftp://");
		sprintf(tempIp, "ftp://%s:%d", ip, port);
		//sprintf(tempIp, "%s", ip);
	}
	/*else if (554 == port) {
		sprintf(tempIp, "rtsp://%s:%d", ip, port);
	}*/
	else {
		//strcpy(tempIp, "http://");
		sprintf(tempIp, "http://%s:%d", ip, port);
	}
	//strncat(tempIp, ip, sz > 96 ? 96 : sz);

	if (port != 37777 && port != 8000 && port != 34567 && port != 9000){
		if (port == 22) size = SSHAuth::SSHLobby(ip, port, &buffer);			//SSH
		else if (21 == port) size = nConnect(ip, port, &buffer);
		else {
			size = nConnect(tempIp, port, &buffer);
			
			// Test WebSocket if HTTP connection was successful and WebSocket is enabled
			if (size > 0 && gWebSocketEnabled && (port == 80 || port == 443 || port == 8080 || port == 8443)) {
				std::string wsBuffer;
				int wsSize = pConnectWebSocket(ip, port, &wsBuffer, "/", (port == 443 || port == 8443));
				if (wsSize > 0) {
					if (gDebugMode) {
						stt->doEmitionYellowFoundData("[WebSocket] Found WebSocket endpoint: " + QString(ip) + ":" + QString::number(port));
					}
					// Add WebSocket info to buffer
					buffer += "\n[WebSocket] Endpoint available at /";
				}
			}
			
			// Test QUIC if HTTP connection was successful and QUIC is enabled
			if (size > 0 && gQuicEnabled && (port == 80 || port == 443 || port == 8080 || port == 8443)) {
				std::string quicBuffer;
				int quicSize = pConnectQuic(ip, port, &quicBuffer, nullptr, true);
				if (quicSize > 0) {
					if (gDebugMode) {
						stt->doEmitionYellowFoundData("[QUIC] Found QUIC endpoint: " + QString(ip) + ":" + QString::number(port));
					}
					// Add QUIC info to buffer
					buffer += "\n[QUIC] Protocol available";
				}
			}
		}

		if (size > 0)
		{
			++Alive;//ME2
			++found;//PieStat
			Lexems lx;
			lx.filler(tempIp, ip, port, &buffer, size, &lx);
		}
		else if (size == -2) return -2;
	} else {
		// For special ports (37777, 8000, 34567, 9000), use direct connection
		// instead of HTTP to properly detect Hikvision cameras
		if (port == 37777 || port == 8000 || port == 34567 || port == 9000) {
			// Use direct socket connection for Hikvision cameras
			sockaddr_in sa;
			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			
			hostent *host = NULL;
#if defined(WIN32)
			if (inet_addr(ip) != INADDR_NONE) sa.sin_addr.S_un.S_addr = inet_addr(ip);
			else if (host = gethostbyname(ip)) ((unsigned long*)&sa.sin_addr)[0] = ((unsigned long**)host->h_addr_list)[0][0];
#else
			if (inet_addr(ip) != INADDR_NONE) sa.sin_addr.s_addr = inet_addr(ip);
			else if (host = gethostbyname(ip)) ((unsigned long*)&sa.sin_addr)[0] = ((unsigned long**)host->h_addr_list)[0][0];
#endif
			else {
				return -1;
			}
			
			SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sock == INVALID_SOCKET) return -1;
			
			struct linger linger = { 1, gTimeOut };
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger));
			
			int res = connect(sock, (sockaddr*)&sa, sizeof(sa));
			if (res != SOCKET_ERROR) {
				// Connection successful, now we can detect the camera type
				// For Hikvision cameras, we need to send a probe and get response
				if (port == 8000) {
					// Try Hikvision iVMS protocol
					send(sock, headerIVMS, 32, 0);
					char tBuff[32] = { 0 };
					int offset = 0;
					int sz = 0;
					int bsz = 0;
					int timeout = 5;
					int bTO;
					while ((sz = recvWT(sock, tBuff, 16, timeout, &bTO)) > 0) {
                                                memcpy(const_cast<char*>(buffer.data()) + offset, tBuff, sz);
						offset += sz;
						bsz += sz;
					}
					buffer.resize(bsz);
				} else if (port == 37777) {
					// Try RVI protocol
					send(sock, headerRVI, 32, 0);
					char tBuff[32] = { 0 };
					int offset = 0;
					int sz = 0;
					int bsz = 0;
					int bTO;
					while ((sz = recvWT(sock, tBuff, 16, gTimeOut, &bTO)) > 0) {
                                                memcpy(const_cast<char*>(buffer.data()) + offset, tBuff, sz);
						offset += sz;
						bsz += sz;
					}
					buffer.resize(bsz);
				}
				
				++Alive;
				++found;
				Lexems lx;
				lx.filler(ip, ip, port, &buffer, buffer.size(), &lx);
				shutdown(sock, SD_BOTH);
				closesocket(sock);
			} else {
				shutdown(sock, SD_BOTH);
				closesocket(sock);
				return -1;
			}
		} else {
			// For other special ports, use portCheck
			if (portCheck(ip, port)) {
				++Alive;//ME2
				++found;//PieStat
				Lexems lx;
				lx.filler(ip, ip, port, &buffer, size, &lx);
			};
		}
	}
	return 0;
}
