#include "STh.h"
#include "MainStarter.h"
#include <QCoreApplication>
#include <QTextStream>
#include <iostream>
#include <cstring>
#include <thread>
#include "externData.h"
#include "Utils.h"

// Initialize global stt object (defined in nesca_3.cpp, but we need it here too)
STh *stt = nullptr;

// Initialize global scan flag
bool globalScanFlag = false;

// Helper function to get optimal thread count
int getOptimalThreadCount() {
	unsigned int hwThreads = std::thread::hardware_concurrency();
	if (hwThreads == 0) {
		// Fallback if hardware_concurrency is not available
		return 100;
	}
	// Use 2x CPU cores for I/O bound operations, but cap at reasonable maximum
	int optimal = hwThreads * 2;
	if (optimal < 50) optimal = 50;   // Minimum reasonable value
	if (optimal > 500) optimal = 500; // Maximum reasonable value
	return optimal;
}

// Validate and set thread count
bool setThreadCount(int threads, bool autoDetect) {
	const int MIN_THREADS = 1;
	const int MAX_THREADS = 2000;
	
	if (autoDetect) {
		gThreads = getOptimalThreadCount();
		return true;
	}
	
	if (threads < MIN_THREADS || threads > MAX_THREADS) {
		QTextStream err(stderr);
		err << "Error: Thread count must be between " << MIN_THREADS 
		    << " and " << MAX_THREADS << Qt::endl;
		return false;
	}
	
	gThreads = threads;
	return true;
}

void printUsage(const char* progName) {
	QTextStream out(stdout);
	out << "Usage: " << progName << " [OPTIONS]" << Qt::endl;
	out << Qt::endl;
	out << "Modes:" << Qt::endl;
	out << "  --ip IP_RANGE              IP scan mode (e.g., 192.168.1.1-192.168.1.255 or 192.168.1.0/24)" << Qt::endl;
	out << "  --dns DNS_MASK             DNS scan mode (e.g., test[a-z])" << Qt::endl;
	out << "  --import FILE              Import scan mode from file" << Qt::endl;
	out << Qt::endl;
	out << "Options:" << Qt::endl;
	out << "  -p, --ports PORTS          Ports to scan (comma-separated, default: " << PORTSET << ")" << Qt::endl;
	out << "  -t, --threads N            Number of threads (1-2000, default: auto-detect)" << Qt::endl;
	out << "  --tld TLD                  Top-level domain for DNS scan (default: .com)" << Qt::endl;
	out << "  -h, --help                 Show this help message" << Qt::endl;
	out << Qt::endl;
	out << "Note: Auto-detect uses 2x CPU cores (min: 50, max: 500 threads)" << Qt::endl;
	out << Qt::endl;
	out << "Examples:" << Qt::endl;
	out << "  " << progName << " --ip 192.168.1.1-192.168.1.255 -p 80,443,8080" << Qt::endl;
	out << "  " << progName << " --dns test[a-z] --tld .com -p 80,443" << Qt::endl;
	out << "  " << progName << " --import ip_list.txt -p 80,443 -t 200" << Qt::endl;
	out << "  " << progName << " --import ip_list.txt -p 80,443 --threads auto" << Qt::endl;
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	
	// Initialize stt object
	stt = new STh();
	
	QString mode;
	QString target;
	QString ports = PORTSET;
	QString tld = ".com";
	bool threadsSpecified = false;
	bool autoDetectThreads = false;
	
	// Parse command line arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--ip") == 0 || strcmp(argv[i], "-i") == 0) {
			if (i + 1 < argc) {
				mode = "ip";
				target = QString(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --ip requires an IP range" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--dns") == 0 || strcmp(argv[i], "-d") == 0) {
			if (i + 1 < argc) {
				mode = "dns";
				target = QString(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --dns requires a DNS mask" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--import") == 0) {
			if (i + 1 < argc) {
				mode = "import";
				target = QString(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --import requires a file path" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--ports") == 0) {
			if (i + 1 < argc) {
				ports = QString(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --ports requires port list" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0) {
			if (i + 1 < argc) {
				QString threadsArg = QString(argv[++i]);
				if (threadsArg.toLower() == "auto") {
					autoDetectThreads = true;
					threadsSpecified = true;
				} else {
					bool ok;
					int threads = threadsArg.toInt(&ok);
					if (!ok || !setThreadCount(threads, false)) {
						return 1;
					}
					threadsSpecified = true;
				}
			} else {
				QTextStream err(stderr);
				err << "Error: --threads requires a number or 'auto'" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--tld") == 0) {
			if (i + 1 < argc) {
				tld = QString(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --tld requires a TLD" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			printUsage(argv[0]);
			return 0;
		} else {
			QTextStream err(stderr);
			err << "Unknown option: " << argv[i] << Qt::endl;
			err << "Use --help for usage information" << Qt::endl;
			return 1;
		}
	}
	
	if (mode.isEmpty()) {
		QTextStream err(stderr);
		err << "Error: No scan mode specified. Use --ip, --dns, or --import" << Qt::endl;
		err << "Use --help for usage information" << Qt::endl;
		return 1;
	}
	
	if (target.isEmpty()) {
		QTextStream err(stderr);
		err << "Error: No target specified" << Qt::endl;
		return 1;
	}
	
	// Set up scan parameters
	if (mode == "ip") {
		stt->setMode(0);
		// Handle IP range format
		QString ipTarget = target;
		ipTarget.replace("http://", "");
		ipTarget.replace("https://", "");
		ipTarget.replace("ftp://", "");
		if (ipTarget.endsWith("/")) {
			ipTarget.chop(1);
		}
		// Convert single IP or CIDR to range format if needed
		if (ipTarget.indexOf("-") < 0 && ipTarget.indexOf("/") < 0) {
			ipTarget = ipTarget + "-" + ipTarget;
		}
		stt->setTarget(ipTarget);
	} else if (mode == "dns") {
		stt->setMode(1);
		QStringList lst = target.split(".");
		QString dnsTarget = lst[0];
		if (lst.size() > 1) {
			QString topLevelDomainStr;
			for (int i = 1; i < lst.size(); ++i) {
				topLevelDomainStr += ".";
				topLevelDomainStr += lst[i];
			}
			tld = topLevelDomainStr;
		}
		strncpy(gTLD, tld.toLocal8Bit().data(), sizeof(gTLD) - 1);
		gTLD[sizeof(gTLD) - 1] = '\0';
		stt->setTarget(dnsTarget);
	} else if (mode == "import") {
		stt->setMode(-1);
		stt->setTarget(target);
	}
	
	ports.replace(" ", "");
	stt->setPorts(ports);
	
	// Set threads if not specified (auto-detect)
	if (!threadsSpecified || autoDetectThreads) {
		if (!setThreadCount(0, true)) {
			return 1;
		}
	}
	
	// Display thread configuration with ANSI colors
	QTextStream out(stdout);
	const char* ANSI_CYAN = "\033[36m";
	const char* ANSI_BOLD = "\033[1m";
	const char* ANSI_GREEN = "\033[32m";
	const char* ANSI_RESET = "\033[0m";
	
	unsigned int hwThreads = std::thread::hardware_concurrency();
	if (hwThreads > 0 && (autoDetectThreads || !threadsSpecified)) {
		out << ANSI_CYAN << "ℹ" << ANSI_RESET << " Using " << ANSI_BOLD << ANSI_GREEN 
		    << gThreads << ANSI_RESET << " threads (auto-detected from " 
		    << hwThreads << " CPU cores)" << Qt::endl;
	} else {
		out << ANSI_CYAN << "ℹ" << ANSI_RESET << " Using " << ANSI_BOLD << ANSI_GREEN 
		    << gThreads << ANSI_RESET << " threads" << Qt::endl;
	}
	
	// Start scan in a thread
	stt->start();
	
	// Wait for scan to complete
	stt->wait();
	
	// Cleanup
	delete stt;
	stt = nullptr;
	
	return 0;
}

