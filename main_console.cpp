#include <STh.h>
#include <MainStarter.h>
#include <QCoreApplication>
#include <QTextStream>
#include <iostream>
#include <cstring>
#include <thread>
#include <csignal>
#include <ctime>
#include <chrono>
#include <QFileInfo>
#include <QProcess>
#include <externData.h>
#include <Utils.h>

// Initialize global stt object (defined in nesca_3.cpp, but we need it here too)
STh *stt = nullptr;

// Initialize global scan flag
bool globalScanFlag = false;

// Global variables for graceful shutdown and statistics
static volatile bool g_interrupted = false;
static std::chrono::steady_clock::time_point g_startTime;

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

// Signal handler for graceful shutdown
void signalHandler(int sig) {
	if (sig == SIGINT || sig == SIGTERM) {
		if (!g_interrupted) {
			g_interrupted = true;
			QTextStream out(stdout);
			out << Qt::endl << "\033[33m[WARN] Interrupt signal received. Stopping scan gracefully...\033[0m" << Qt::endl;
			globalScanFlag = false;
		} else {
			// Second interrupt - force exit
			QTextStream err(stderr);
			err << "\n\033[31m[ERROR] Force exit\033[0m" << Qt::endl;
			exit(1);
		}
	}
}

// Setup signal handlers
void setupSignalHandlers() {
	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);
}

// Run IP range generator
bool runGenerator(const QString& outputFile = "all_russia_combined.txt", 
                  const QString& directory = "russia_ranges", 
                  bool generateConfigs = false) {
	QTextStream out(stdout);
	const char* ANSI_CYAN = "\033[36m";
	const char* ANSI_GREEN = "\033[32m";
	const char* ANSI_YELLOW = "\033[33m";
	const char* ANSI_RESET = "\033[0m";
	
	out << ANSI_CYAN << "[GENERATOR]" << ANSI_RESET << " Starting Russia IP ranges generator..." << Qt::endl;
	
	// Try to find generator.py
	QFileInfo generatorFile("generator.py");
	if (!generatorFile.exists()) {
		QTextStream err(stderr);
		err << "[ERROR] generator.py not found in current directory" << Qt::endl;
		return false;
	}
	
	if (!generatorFile.isReadable()) {
		QTextStream err(stderr);
		err << "[ERROR] generator.py is not readable" << Qt::endl;
		return false;
	}
	
	// Check if Python 3 is available
	QProcess pythonCheck;
	pythonCheck.start("python3", QStringList() << "--version");
	if (!pythonCheck.waitForFinished(3000)) {
		QTextStream err(stderr);
		err << "[ERROR] Python 3 is not available. Please install python3" << Qt::endl;
		return false;
	}
	
	// Build command
	QStringList args;
	args << "generator.py";
	args << "-o" << outputFile;
	args << "-d" << directory;
	if (generateConfigs) {
		args << "-c";
	}
	
	// Run generator
	QProcess generator;
	generator.setProcessChannelMode(QProcess::MergedChannels);
	
	out << ANSI_CYAN << "[GENERATOR]" << ANSI_RESET << " Running generator..." << Qt::endl;
	generator.start("python3", args);
	
	if (!generator.waitForStarted()) {
		QTextStream err(stderr);
		err << "[ERROR] Failed to start generator.py" << Qt::endl;
		return false;
	}
	
	// Read output in real-time
	QByteArray output;
	while (generator.waitForReadyRead(1000)) {
		output.append(generator.readAll());
	}
	
	// Wait for completion (with timeout of 5 minutes)
	if (!generator.waitForFinished(300000)) {
		QTextStream err(stderr);
		err << "[ERROR] Generator timed out (5 minutes)" << Qt::endl;
		generator.kill();
		return false;
	}
	
	// Get final output
	output.append(generator.readAllStandardOutput());
	output.append(generator.readAllStandardError());
	
	// Print output
	QTextStream outputStream(output);
	while (!outputStream.atEnd()) {
		QString line = outputStream.readLine();
		out << line << Qt::endl;
	}
	
	if (generator.exitCode() != 0) {
		QTextStream err(stderr);
		err << "[ERROR] Generator failed with exit code: " << generator.exitCode() << Qt::endl;
		return false;
	}
	
	// Verify output file was created
	QFileInfo outputInfo(outputFile);
	if (!outputInfo.exists()) {
		QTextStream err(stderr);
		err << "[ERROR] Generator did not create output file: " << outputFile << Qt::endl;
		return false;
	}
	
	out << ANSI_GREEN << "[OK]" << ANSI_RESET << " Generator completed successfully" << Qt::endl;
	out << ANSI_CYAN << "[INFO]" << ANSI_RESET << " Output file: " << outputFile << Qt::endl;
	
	return true;
}

// Validate import file exists and is readable
bool validateImportFile(const QString& filePath) {
	QFileInfo fileInfo(filePath);
	
	if (!fileInfo.exists()) {
		QTextStream err(stderr);
		err << "[ERROR] File does not exist: " << filePath << Qt::endl;
		return false;
	}
	
	if (!fileInfo.isFile()) {
		QTextStream err(stderr);
		err << "[ERROR] Path is not a file: " << filePath << Qt::endl;
		return false;
	}
	
	if (!fileInfo.isReadable()) {
		QTextStream err(stderr);
		err << "[ERROR] File is not readable: " << filePath << Qt::endl;
		return false;
	}
	
	if (fileInfo.size() == 0) {
		QTextStream err(stderr);
		err << "[WARN] Warning: File is empty: " << filePath << Qt::endl;
	}
	
	return true;
}

// Print detailed statistics at the end
void printStatistics() {
	QTextStream out(stdout);
	const char* ANSI_CYAN = "\033[36m";
	const char* ANSI_GREEN = "\033[32m";
	const char* ANSI_YELLOW = "\033[33m";
	const char* ANSI_BOLD = "\033[1m";
	const char* ANSI_RESET = "\033[0m";
	
	auto endTime = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - g_startTime).count();
	
	out << Qt::endl << ANSI_BOLD << ANSI_CYAN << "===========================================================" << ANSI_RESET << Qt::endl;
	out << ANSI_BOLD << "Scan Statistics:" << ANSI_RESET << Qt::endl;
	out << "-----------------------------------------------------------" << Qt::endl;
	
	extern int found, saved, camerasC1, PieBA, PieOther, PieSSH, Alive, filtered;
	extern unsigned long long gTargetsNumber;
	extern long long unsigned int gTargets;
	
	if (duration > 0) {
		double ipsPerSec = (gTargetsNumber > 0 && duration > 0) ? 
			(double)(gTargetsNumber - gTargets) / duration : 0.0;
		out << "Duration:        " << ANSI_CYAN << duration << ANSI_RESET << " seconds" << Qt::endl;
		out << "Speed:           " << ANSI_CYAN << QString::number(ipsPerSec, 'f', 2) << ANSI_RESET << " IPs/sec" << Qt::endl;
	}
	
	out << "Targets:         " << ANSI_CYAN << gTargetsNumber << ANSI_RESET << " total" << Qt::endl;
	
	if (found > 0) {
		double successRate = (gTargetsNumber > 0) ? ((double)found / (double)gTargetsNumber * 100.0) : 0.0;
		out << "Found:           " << ANSI_GREEN << found << ANSI_RESET << " nodes (" 
		    << QString::number(successRate, 'f', 2) << "%)" << Qt::endl;
	} else {
		out << "Found:           " << ANSI_GREEN << found << ANSI_RESET << " nodes" << Qt::endl;
	}
	
	out << "Saved:           " << ANSI_GREEN << saved << ANSI_RESET << " nodes";
	if (found > 0) {
		double saveRate = ((double)saved / (double)found * 100.0);
		out << " (" << QString::number(saveRate, 'f', 1) << "%)";
	}
	out << Qt::endl;
	
	if (camerasC1 > 0 || PieBA > 0 || PieOther > 0 || PieSSH > 0) {
		out << Qt::endl << "By Type:" << Qt::endl;
		if (camerasC1 > 0) out << "  Cameras:         " << ANSI_YELLOW << camerasC1 << ANSI_RESET << Qt::endl;
		if (PieBA > 0) out << "  Auth:            " << ANSI_YELLOW << PieBA << ANSI_RESET << Qt::endl;
		if (PieOther > 0) out << "  Other:           " << ANSI_YELLOW << PieOther << ANSI_RESET << Qt::endl;
		if (PieSSH > 0) out << "  SSH:             " << ANSI_YELLOW << PieSSH << ANSI_RESET << Qt::endl;
	}
	
	if (Alive > 0) out << "Alive:           " << ANSI_GREEN << Alive << ANSI_RESET << Qt::endl;
	if (filtered > 0) out << "Filtered:        " << ANSI_YELLOW << filtered << ANSI_RESET << Qt::endl;
	
	out << ANSI_BOLD << ANSI_CYAN << "===========================================================" << ANSI_RESET << Qt::endl;
}

void printUsage(const char* progName) {
	QTextStream out(stdout);
	out << "Usage: " << progName << " [OPTIONS]" << Qt::endl;
	out << Qt::endl;
	out << "Modes:" << Qt::endl;
	out << "  --ip IP_RANGE              IP scan mode (e.g., 192.168.1.1-192.168.1.255 or 192.168.1.0/24)" << Qt::endl;
	out << "  --dns DNS_MASK             DNS scan mode (e.g., test[a-z])" << Qt::endl;
	out << "  --import FILE              Import scan mode from file" << Qt::endl;
	out << "  --generate [FILE]          Generate Russia IP ranges (default: all_russia_combined.txt)" << Qt::endl;
	out << "  --generate-and-scan [FILE] Generate ranges and immediately start scanning" << Qt::endl;
	out << Qt::endl;
	out << "Options:" << Qt::endl;
	out << "  -p, --ports PORTS          Ports to scan (comma-separated, default: " << PORTSET << ")" << Qt::endl;
	out << "  -t, --threads N            Number of threads (1-2000, default: auto-detect)" << Qt::endl;
	out << "  --timeout MS               Connection timeout in milliseconds (default: 3000)" << Qt::endl;
	out << "  --max-rate RATE            Max requests per second (0 = unlimited, default: 0)" << Qt::endl;
	out << "  --retries N                Number of retries on failure (default: 0)" << Qt::endl;
	out << "  --verify-ssl               Verify SSL certificates (default: disabled)" << Qt::endl;
	out << "  --user-agent STRING        Custom User-Agent string" << Qt::endl;
	out << "  --adaptive                 Enable adaptive thread adjustment based on network load" << Qt::endl;
	out << "  --smart-scan               Prioritize popular ports for found IPs" << Qt::endl;
	out << "  --batch-size N             Batch size for mass scanning (default: disabled)" << Qt::endl;
	out << "  --ping-timeout SECONDS     Ping timeout in seconds (default: 1)" << Qt::endl;
	out << "  --tld TLD                  Top-level domain for DNS scan (default: .com)" << Qt::endl;
	out << "  --generator-configs        Generate scan configs when using --generate" << Qt::endl;
	out << "  -h, --help                 Show this help message" << Qt::endl;
	out << Qt::endl;
	out << "Note: Auto-detect uses 2x CPU cores (min: 50, max: 500 threads)" << Qt::endl;
	out << Qt::endl;
	out << "Examples:" << Qt::endl;
	out << "  " << progName << " --ip 192.168.1.1-192.168.1.255 -p 80,443,8080" << Qt::endl;
	out << "  " << progName << " --dns test[a-z] --tld .com -p 80,443" << Qt::endl;
	out << "  " << progName << " --import ip_list.txt -p 80,443 -t 200" << Qt::endl;
	out << "  " << progName << " --generate all_russia.txt" << Qt::endl;
	out << "  " << progName << " --generate-and-scan all_russia.txt -p 80,3000,8000,37777" << Qt::endl;
	out << "  " << progName << " --import ip_list.txt --timeout 5000 --ping-timeout 2" << Qt::endl;
	out << "  " << progName << " --ip 192.168.1.0/24 --max-rate 1000 --retries 2 --verify-ssl" << Qt::endl;
	out << "  " << progName << " --ip 10.0.0.0/24 --user-agent \"Nesca-Scanner/2.0\" --timeout 5000" << Qt::endl;
	out << "  " << progName << " --ip 192.168.1.0/24 --adaptive --smart-scan --batch-size 50" << Qt::endl;
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	
	// Initialize stt object
	stt = new STh();
	
	// Setup signal handlers for graceful shutdown
	setupSignalHandlers();
	
	// Record start time for statistics
	g_startTime = std::chrono::steady_clock::now();
	
	QString mode;
	QString target;
	QString ports = PORTSET;
	QString tld = ".com";
	bool threadsSpecified = false;
	bool autoDetectThreads = false;
	bool generateConfigs = false;
	QString generateOutput = "all_russia_combined.txt";
	
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
		} else if (strcmp(argv[i], "--generate") == 0 || strcmp(argv[i], "-g") == 0) {
			mode = "generate";
			if (i + 1 < argc && argv[i + 1][0] != '-') {
				generateOutput = QString(argv[++i]);
			}
		} else if (strcmp(argv[i], "--generate-and-scan") == 0) {
			mode = "generate_and_scan";
			if (i + 1 < argc && argv[i + 1][0] != '-') {
				generateOutput = QString(argv[++i]);
				target = generateOutput;
			} else {
				target = generateOutput;
			}
		} else if (strcmp(argv[i], "--generator-configs") == 0) {
			generateConfigs = true;
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
		} else if (strcmp(argv[i], "--timeout") == 0) {
			if (i + 1 < argc) {
				bool ok;
				int timeout = QString(argv[++i]).toInt(&ok);
				if (!ok || timeout < 1 || timeout > 60000) {
					QTextStream err(stderr);
					err << "Error: --timeout must be between 1 and 60000 milliseconds" << Qt::endl;
					return 1;
				}
				// Convert milliseconds to seconds for curl (round up to at least 1 second)
				gTimeOut = (timeout + 999) / 1000;
				if (gTimeOut < 1) gTimeOut = 1;
			} else {
				QTextStream err(stderr);
				err << "Error: --timeout requires a number (milliseconds)" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--max-rate") == 0) {
			if (i + 1 < argc) {
				bool ok;
				int maxRate = QString(argv[++i]).toInt(&ok);
				if (!ok || maxRate < 0 || maxRate > 100000) {
					QTextStream err(stderr);
					err << "Error: --max-rate must be between 0 and 100000 requests per second" << Qt::endl;
					return 1;
				}
				gMaxRate = maxRate;
			} else {
				QTextStream err(stderr);
				err << "Error: --max-rate requires a number" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--retries") == 0) {
			if (i + 1 < argc) {
				bool ok;
				int retries = QString(argv[++i]).toInt(&ok);
				if (!ok || retries < 0 || retries > 10) {
					QTextStream err(stderr);
					err << "Error: --retries must be between 0 and 10" << Qt::endl;
					return 1;
				}
				gRetries = retries;
			} else {
				QTextStream err(stderr);
				err << "Error: --retries requires a number" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--verify-ssl") == 0) {
			gVerifySSL = true;
		} else if (strcmp(argv[i], "--adaptive") == 0) {
			gAdaptiveScan = true;
		} else if (strcmp(argv[i], "--smart-scan") == 0) {
			gSmartScan = true;
		} else if (strcmp(argv[i], "--batch-size") == 0) {
			if (i + 1 < argc) {
				bool ok;
				int batchSize = QString(argv[++i]).toInt(&ok);
				if (!ok || batchSize < 1 || batchSize > 10000) {
					QTextStream err(stderr);
					err << "Error: --batch-size must be between 1 and 10000" << Qt::endl;
					return 1;
				}
				gBatchSize = batchSize;
			} else {
				QTextStream err(stderr);
				err << "Error: --batch-size requires a number" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--user-agent") == 0) {
			if (i + 1 < argc) {
				QString ua = QString(argv[++i]);
				if (ua.length() > 255) {
					QTextStream err(stderr);
					err << "Error: --user-agent string too long (max 255 characters)" << Qt::endl;
					return 1;
				}
				strncpy(gUserAgent, ua.toLocal8Bit().data(), sizeof(gUserAgent) - 1);
				gUserAgent[sizeof(gUserAgent) - 1] = '\0';
			} else {
				QTextStream err(stderr);
				err << "Error: --user-agent requires a string" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--ping-timeout") == 0) {
			if (i + 1 < argc) {
				bool ok;
				int pingTimeout = QString(argv[++i]).toInt(&ok);
				if (!ok || pingTimeout < 1 || pingTimeout > 10) {
					QTextStream err(stderr);
					err << "Error: --ping-timeout must be between 1 and 10 seconds" << Qt::endl;
					return 1;
				}
				gPingTimeout = pingTimeout;
			} else {
				QTextStream err(stderr);
				err << "Error: --ping-timeout requires a number" << Qt::endl;
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
	
	// Handle generation mode
	if (mode == "generate") {
		if (!runGenerator(generateOutput, "russia_ranges", generateConfigs)) {
			return 1;
		}
		return 0;
	}
	
	// Handle generate-and-scan mode
	if (mode == "generate_and_scan") {
		QTextStream out(stdout);
		const char* ANSI_CYAN = "\033[36m";
		const char* ANSI_RESET = "\033[0m";
		
		// Check if file already exists
		QFileInfo fileInfo(target);
		if (fileInfo.exists()) {
			out << ANSI_CYAN << "[INFO]" << ANSI_RESET << " File exists: " << target 
			    << ", skipping generation. Use --generate to regenerate." << Qt::endl;
		} else {
			// Generate the file first
			if (!runGenerator(generateOutput, "russia_ranges", generateConfigs)) {
				QTextStream err(stderr);
				err << "[ERROR] Failed to generate IP ranges file" << Qt::endl;
				return 1;
			}
		}
		
		// Switch to import mode
		mode = "import";
	}
	
	if (mode.isEmpty()) {
		QTextStream err(stderr);
		err << "Error: No scan mode specified. Use --ip, --dns, --import, or --generate" << Qt::endl;
		err << "Use --help for usage information" << Qt::endl;
		return 1;
	}
	
	if (target.isEmpty() && mode != "generate") {
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
		// Validate import file before proceeding
		if (!validateImportFile(target)) {
			return 1;
		}
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
		out << ANSI_CYAN << "[INFO]" << ANSI_RESET << " Using " << ANSI_BOLD << ANSI_GREEN 
		    << gThreads << ANSI_RESET << " threads (auto-detected from " 
		    << hwThreads << " CPU cores)" << Qt::endl;
	} else {
		out << ANSI_CYAN << "[INFO]" << ANSI_RESET << " Using " << ANSI_BOLD << ANSI_GREEN 
		    << gThreads << ANSI_RESET << " threads" << Qt::endl;
	}
	
	// Display configuration summary (reuse ANSI constants from above)
	out << ANSI_CYAN << "[INFO]" << ANSI_RESET << " Timeout: " << (gTimeOut * 1000) << "ms, Ping timeout: " << gPingTimeout << "s";
	if (gMaxRate > 0) {
		out << ", Max rate: " << gMaxRate << " req/s";
	}
	if (gRetries > 0) {
		out << ", Retries: " << gRetries;
	}
	if (gVerifySSL) {
		out << ", SSL verification: enabled";
	}
	if (gUserAgent[0] != '\0') {
		out << ", User-Agent: " << gUserAgent;
	}
	if (gAdaptiveScan) {
		out << ", Adaptive: enabled";
	}
	if (gSmartScan) {
		out << ", Smart scan: enabled";
	}
	if (gBatchSize > 0) {
		out << ", Batch size: " << gBatchSize;
	}
	out << Qt::endl << Qt::endl;
	
	// Start scan in a thread
	stt->start();
	
	// Wait for scan to complete
	stt->wait();
	
	// Check if interrupted
	if (g_interrupted) {
		QTextStream out(stdout);
		out << "\033[33m[WARN] Scan interrupted by user\033[0m" << Qt::endl;
	}
	
	// Print final statistics
	printStatistics();
	
	// Cleanup
	delete stt;
	stt = nullptr;
	
	return g_interrupted ? 130 : 0; // Return 130 for SIGINT (standard exit code)
}

