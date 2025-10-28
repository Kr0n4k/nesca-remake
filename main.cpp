#include "STh.h"
#include "MainStarter.h"
#include "ResultExporter.h"
#include "ProgressMonitor.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <iostream>
#include <cstring>
#include <chrono>
#include "externData.h"
#include "Utils.h"
#include "mainResources.h"

// Initialize global stt object (will be used by other modules)
STh *stt = nullptr;

// Initialize globalScanFlag for console mode (other variables are defined in MainStarter.cpp and FileUpdater.cpp)
bool globalScanFlag = false;

// Note: jsonArr is already defined in MainStarter.cpp, don't define it here

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
	out << "  -t, --threads N              Number of threads (default: auto)" << Qt::endl;
	out << "  --tld TLD                    Top-level domain for DNS scan (default: .com)" << Qt::endl;
	out << "  --export FORMAT              Export results format (json|csv|both, default: json)" << Qt::endl;
	out << "  --no-export                  Disable automatic export after scan" << Qt::endl;
	out << "  --export-only [DIR]          Export existing results without scanning" << Qt::endl;
	out << "  --export-file FILE           Output file for export (default: auto-generated)" << Qt::endl;
	out << "  --export-filter-ip IP        Filter export by IP pattern" << Qt::endl;
	out << "  --export-filter-port PORTS   Filter export by ports (comma-separated)" << Qt::endl;
	out << "  --export-filter-type TYPES   Filter export by types (comma-separated: camera,auth,ftp,ssh,other)" << Qt::endl;
	out << "  --live-stats                 Show live statistics during scan (default: enabled)" << Qt::endl;
	out << "  --no-live-stats              Disable live statistics display" << Qt::endl;
	out << "  -h, --help                   Show this help message" << Qt::endl;
	out << Qt::endl;
	out << "Examples:" << Qt::endl;
	out << "  " << progName << " --ip 192.168.1.1-192.168.1.255 -p 80,443,8080" << Qt::endl;
	out << "  " << progName << " --dns test[a-z] --tld .com -p 80,443" << Qt::endl;
	out << "  " << progName << " --import ip_list.txt -p 80,443" << Qt::endl;
	out << "  " << progName << " --ip 192.168.1.0/24 -p 80,443" << Qt::endl;
	out << "  " << progName << " --import ips.txt --export csv --export-filter-type camera,auth" << Qt::endl;
	out << "  " << progName << " --ip 10.0.0.0/24 --export both --export-filter-port 80,8080" << Qt::endl;
	out << "  " << progName << " --export-only" << Qt::endl;
	out << "  " << progName << " --export-only results_2025.01.15_target" << Qt::endl;
	out << "  " << progName << " --ip 192.168.1.0/24 --no-export" << Qt::endl;
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	
	QString mode;
	QString target;
	QString ports = PORTSET;
	QString tld = ".com";
	QString exportFormat = "json";  // Default: export to JSON
	bool noExport = false;
	bool exportOnly = false;
	QString exportOnlyDir;
	QString exportFile;
	QString exportFilterIP;
	QStringList exportFilterPorts;
	QStringList exportFilterTypes;
	bool liveStats = true;  // Default: enabled
	std::chrono::steady_clock::time_point scanStartTime;
	
	// Parse command line arguments first to check for export-only mode
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
				gThreads = atoi(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --threads requires a number" << Qt::endl;
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
		} else if (strcmp(argv[i], "--export") == 0) {
			if (i + 1 < argc) {
				exportFormat = QString(argv[++i]).toLower();
				if (exportFormat != "json" && exportFormat != "csv" && exportFormat != "both") {
					QTextStream err(stderr);
					err << "Error: --export must be 'json', 'csv', or 'both'" << Qt::endl;
					return 1;
				}
			} else {
				QTextStream err(stderr);
				err << "Error: --export requires a format (json|csv|both)" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--no-export") == 0) {
			noExport = true;
			exportFormat = "";  // Clear default
		} else if (strcmp(argv[i], "--export-only") == 0) {
			exportOnly = true;
			mode = "export-only";  // Special mode
			if (i + 1 < argc && argv[i + 1][0] != '-') {
				exportOnlyDir = QString(argv[++i]);
			}
			continue;  // Skip other processing for export-only
		} else if (strcmp(argv[i], "--export-file") == 0) {
			if (i + 1 < argc) {
				exportFile = QString(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --export-file requires a file path" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--export-filter-ip") == 0) {
			if (i + 1 < argc) {
				exportFilterIP = QString(argv[++i]);
			} else {
				QTextStream err(stderr);
				err << "Error: --export-filter-ip requires an IP pattern" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--export-filter-port") == 0) {
			if (i + 1 < argc) {
				QStringList ports = QString(argv[++i]).split(',');
				for (const QString &port : ports) {
					QString trimmed = port.trimmed();
					if (!trimmed.isEmpty()) {
						exportFilterPorts.append(trimmed);
					}
				}
			} else {
				QTextStream err(stderr);
				err << "Error: --export-filter-port requires port list" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--export-filter-type") == 0) {
			if (i + 1 < argc) {
				QStringList types = QString(argv[++i]).split(',');
				for (const QString &type : types) {
					QString trimmed = type.trimmed();
					if (!trimmed.isEmpty()) {
						exportFilterTypes.append(trimmed);
					}
				}
			} else {
				QTextStream err(stderr);
				err << "Error: --export-filter-type requires type list" << Qt::endl;
				return 1;
			}
		} else if (strcmp(argv[i], "--live-stats") == 0) {
			liveStats = true;
		} else if (strcmp(argv[i], "--no-live-stats") == 0) {
			liveStats = false;
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
	
	// Initialize stt object only if not in export-only mode
	if (!exportOnly) {
		stt = new STh();
		// Set default threads if not specified
		if (gThreads == 0) {
			gThreads = 100; // Default thread count
		}
		scanStartTime = std::chrono::steady_clock::now(); // Initialize start time
	}
	
	// Handle export-only mode
	if (exportOnly) {
		QTextStream out(stdout);
		out << "[INFO] Export-only mode: exporting existing results..." << Qt::endl;
		
		ResultExporter exporter;
		QString resultsDir = exportOnlyDir;
		
		if (resultsDir.isEmpty()) {
			// Find most recent results directory
			QDir dir(".");
			QStringList filters;
			filters << "results_*";
			QStringList entries = dir.entryList(filters, QDir::Dirs | QDir::NoDotAndDotDot);
			
			if (!entries.isEmpty()) {
				entries.sort();
				resultsDir = entries.last();
				out << "[INFO] Using most recent results directory: " << resultsDir << Qt::endl;
			} else {
				QTextStream err(stderr);
				err << "[ERROR] No results directories found. Use --export-only DIR to specify directory." << Qt::endl;
				return 1;
			}
		}
		
		// Parse HTML results
		if (!exporter.parseHTMLResults(resultsDir)) {
			QTextStream err(stderr);
			err << "[ERROR] Could not parse results from: " << resultsDir << Qt::endl;
			err << "[INFO] Trying current directory..." << Qt::endl;
			if (!exporter.parseHTMLResults(".")) {
				err << "[ERROR] No results found to export" << Qt::endl;
				return 1;
			}
		}
		
		// Generate output filename if not specified
		QString baseFile = exportFile;
		if (baseFile.isEmpty()) {
			if (!exportOnlyDir.isEmpty()) {
				baseFile = exportOnlyDir + "_export";
			} else {
				QFileInfo info(resultsDir);
				baseFile = info.baseName() + "_export";
			}
		}
		
		// Export in requested format(s)
		bool success = false;
		if (exportFormat == "json" || exportFormat == "both") {
			QString jsonFile = baseFile;
			if (!jsonFile.endsWith(".json") && exportFormat == "json") {
				jsonFile += ".json";
			} else if (exportFormat == "both") {
				jsonFile += ".json";
			}
			if (exporter.exportToJSON(jsonFile, exportFilterIP, exportFilterPorts, exportFilterTypes)) {
				out << "[OK] Exported JSON to: " << jsonFile << Qt::endl;
				success = true;
			} else {
				out << "[ERROR] Failed to export JSON" << Qt::endl;
			}
		}
		
		if (exportFormat == "csv" || exportFormat == "both") {
			QString csvFile = baseFile;
			if (!csvFile.endsWith(".csv") && exportFormat == "csv") {
				csvFile += ".csv";
			} else if (exportFormat == "both") {
				csvFile += ".csv";
			}
			if (exporter.exportToCSV(csvFile, exportFilterIP, exportFilterPorts, exportFilterTypes)) {
				out << "[OK] Exported CSV to: " << csvFile << Qt::endl;
				success = true;
			} else {
				out << "[ERROR] Failed to export CSV" << Qt::endl;
			}
		}
		
		if (success) {
			QJsonObject stats = exporter.getStatistics();
			out << "[INFO] Statistics: " << stats["total"].toInt() << " total results exported" << Qt::endl;
		}
		
		return success ? 0 : 1;
	}
	
	if (mode.isEmpty()) {
		QTextStream err(stderr);
		err << "Error: No scan mode specified. Use --ip, --dns, --import, or --export-only" << Qt::endl;
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
	
	// Start progress monitor if enabled
	ProgressMonitor *progressMonitor = nullptr;
	if (liveStats && !exportOnly) {
		progressMonitor = new ProgressMonitor();
		progressMonitor->start();
		QTextStream out(stdout);
		out << Qt::endl << "[INFO] Starting scan with live statistics..." << Qt::endl;
		out << "[INFO] Progress will be updated every 2 seconds" << Qt::endl << Qt::endl;
	}
	
	// Start scan in a thread
	stt->start();
	
	// Wait for scan to complete
	stt->wait();
	
	// Stop progress monitor
	if (progressMonitor) {
		progressMonitor->stop();
		progressMonitor->wait(3000); // Wait up to 3 seconds
		delete progressMonitor;
		progressMonitor = nullptr;
		QTextStream out(stdout);
		out << Qt::endl; // Ensure new line after progress
	}
	
	// Print final statistics
	if (!exportOnly) {
		QTextStream out(stdout);
		const char* ANSI_CYAN = "\033[36m";
		const char* ANSI_GREEN = "\033[32m";
		const char* ANSI_YELLOW = "\033[33m";
		const char* ANSI_BOLD = "\033[1m";
		const char* ANSI_RESET = "\033[0m";
		
		auto endTime = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - scanStartTime).count();
		
		extern int found, saved, camerasC1, PieBA, PieOther, PieSSH, Alive, filtered;
		extern unsigned long long gTargetsNumber;
		
		out << Qt::endl << ANSI_BOLD << ANSI_CYAN << "===========================================================" << ANSI_RESET << Qt::endl;
		out << ANSI_BOLD << "Scan Summary:" << ANSI_RESET << Qt::endl;
		out << "-----------------------------------------------------------" << Qt::endl;
		
		if (duration > 0) {
			double ipsPerSec = (gTargetsNumber > 0 && duration > 0) ? 
				(double)(gTargetsNumber) / duration : 0.0;
			out << "Duration:        " << ANSI_CYAN << duration << ANSI_RESET << " seconds" << Qt::endl;
			out << "Average Speed:   " << ANSI_CYAN << QString::number(ipsPerSec, 'f', 2) << ANSI_RESET << " IPs/sec" << Qt::endl;
		}
		
		out << "Targets:         " << ANSI_CYAN << gTargetsNumber << ANSI_RESET << " total" << Qt::endl;
		out << "Found:           " << ANSI_GREEN << found << ANSI_RESET << " nodes";
		if (found > 0 && gTargetsNumber > 0) {
			double successRate = (double)found / (double)gTargetsNumber * 100.0;
			out << " (" << QString::number(successRate, 'f', 2) << "%)";
		}
		out << Qt::endl;
		
		out << "Saved:           " << ANSI_GREEN << saved << ANSI_RESET << " nodes";
		if (found > 0) {
			double saveRate = ((double)saved / (double)found * 100.0);
			out << " (" << QString::number(saveRate, 'f', 1) << "%)";
		}
		out << Qt::endl;
		
		if (camerasC1 > 0 || PieBA > 0 || PieOther > 0 || PieSSH > 0) {
			out << Qt::endl << "By Type:" << Qt::endl;
			if (camerasC1 > 0) out << "  Cameras:       " << ANSI_YELLOW << camerasC1 << ANSI_RESET << Qt::endl;
			if (PieBA > 0) out << "  Auth:          " << ANSI_YELLOW << PieBA << ANSI_RESET << Qt::endl;
			if (PieOther > 0) out << "  Other:         " << ANSI_YELLOW << PieOther << ANSI_RESET << Qt::endl;
			if (PieSSH > 0) out << "  SSH:           " << ANSI_YELLOW << PieSSH << ANSI_RESET << Qt::endl;
		}
		
		if (Alive > 0) out << "Alive:           " << ANSI_GREEN << Alive << ANSI_RESET << Qt::endl;
		if (filtered > 0) out << "Filtered:        " << ANSI_YELLOW << filtered << ANSI_RESET << Qt::endl;
		
		out << ANSI_BOLD << ANSI_CYAN << "===========================================================" << ANSI_RESET << Qt::endl;
	}
	
	// Export results if enabled (default: yes, unless --no-export)
	if (!noExport && !exportFormat.isEmpty()) {
		QTextStream out(stdout);
		out << Qt::endl << "[INFO] Exporting results..." << Qt::endl;
		
		ResultExporter exporter;
		
		// Find results directory
		QString resultsDir = "./";
		QDir dir(".");
		QStringList filters;
		filters << "results_*";
		QStringList entries = dir.entryList(filters, QDir::Dirs | QDir::NoDotAndDotDot);
		
		if (!entries.isEmpty()) {
			// Use most recent results directory
			entries.sort();
			resultsDir = entries.last();
			out << "[INFO] Found results directory: " << resultsDir << Qt::endl;
		} else {
			// Try to construct from date and target
			QString date = Utils::getStartDate().c_str();
			QString currentTarget = Utils::getCurrentTarget().c_str();
			resultsDir = QString("results_%1_%2").arg(date, currentTarget);
		}
		
		// Parse HTML results
		if (!exporter.parseHTMLResults(resultsDir)) {
			out << "[WARN] Could not parse results from: " << resultsDir << Qt::endl;
			out << "[INFO] Trying current directory..." << Qt::endl;
			if (!exporter.parseHTMLResults(".")) {
				out << "[ERROR] No results found to export" << Qt::endl;
			}
		}
		
		// Generate output filename if not specified
		QString baseFile = exportFile;
		if (baseFile.isEmpty()) {
			QString date = Utils::getStartDate().c_str();
			QString target = Utils::getCurrentTarget().c_str();
			baseFile = QString("export_%1_%2").arg(date, target);
		}
		
		// Export in requested format(s)
		bool success = false;
		if (exportFormat == "json" || exportFormat == "both") {
			QString jsonFile = baseFile;
			if (!jsonFile.endsWith(".json") && exportFormat == "json") {
				jsonFile += ".json";
			} else if (exportFormat == "both") {
				jsonFile += ".json";
			}
			if (exporter.exportToJSON(jsonFile, exportFilterIP, exportFilterPorts, exportFilterTypes)) {
				out << "[OK] Exported JSON to: " << jsonFile << Qt::endl;
				success = true;
			} else {
				out << "[ERROR] Failed to export JSON" << Qt::endl;
			}
		}
		
		if (exportFormat == "csv" || exportFormat == "both") {
			QString csvFile = baseFile;
			if (!csvFile.endsWith(".csv") && exportFormat == "csv") {
				csvFile += ".csv";
			} else if (exportFormat == "both") {
				csvFile += ".csv";
			}
			if (exporter.exportToCSV(csvFile, exportFilterIP, exportFilterPorts, exportFilterTypes)) {
				out << "[OK] Exported CSV to: " << csvFile << Qt::endl;
				success = true;
			} else {
				out << "[ERROR] Failed to export CSV" << Qt::endl;
			}
		}
		
		if (success) {
			QJsonObject stats = exporter.getStatistics();
			out << "[INFO] Statistics: " << stats["total"].toInt() << " total results" << Qt::endl;
		}
	}
	
	// Cleanup
	delete stt;
	stt = nullptr;
	
	return 0;
}
	
