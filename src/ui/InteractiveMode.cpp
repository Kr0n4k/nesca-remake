#include <InteractiveMode.h>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QMap>
#include <algorithm>

bool InteractiveMode::colorEnabled = true;
QStringList InteractiveMode::mainMenuOptions;
QMap<QString, ScanPreset> InteractiveMode::quickPresets;

void InteractiveMode::initializePresets() {
    if (!quickPresets.isEmpty()) return;
    
    // Quick Camera Scan
    ScanPreset camera;
    camera.name = "quick-camera-scan";
    camera.description = "Quick scan for IP cameras (ports 80, 8080, 554, 8554)";
    camera.ports = "80,8080,554,8554";
    camera.threads = 150;
    camera.adaptive = false;
    camera.smartScan = true;
    camera.additionalArgs = QStringList() << "--adaptive" << "--smart-scan";
    quickPresets["quick-camera-scan"] = camera;
    
    // Quick Server Scan
    ScanPreset server;
    server.name = "quick-server-scan";
    server.description = "Quick scan for servers (ports 22, 80, 443, 3389, 5985, 5986)";
    server.ports = "22,80,443,3389,5985,5986";
    server.threads = 200;
    server.adaptive = true;
    server.smartScan = false;
    server.additionalArgs = QStringList() << "--adaptive";
    quickPresets["quick-server-scan"] = server;
    
    // Network Equipment Scan
    ScanPreset network;
    network.name = "quick-network-scan";
    network.description = "Quick scan for network equipment (ports 80, 443, 8080, 8291, 8443)";
    network.ports = "80,443,8080,8291,8443";
    network.threads = 150;
    network.adaptive = true;
    network.smartScan = true;
    network.additionalArgs = QStringList() << "--adaptive" << "--smart-scan";
    quickPresets["quick-network-scan"] = network;
    
    // IoT Devices Scan
    ScanPreset iot;
    iot.name = "quick-iot-scan";
    iot.description = "Quick scan for IoT devices (ports 80, 8080, 8081, 8443)";
    iot.ports = "80,8080,8081,8443";
    iot.threads = 100;
    iot.adaptive = false;
    iot.smartScan = true;
    iot.additionalArgs = QStringList() << "--smart-scan";
    quickPresets["quick-iot-scan"] = iot;
}

InteractiveMode::InteractiveMode() {
    initializePresets();
}

void InteractiveMode::printBanner() {
    QTextStream out(stdout);
    
    if (colorEnabled) {
        const char* ANSI_BOLD = "\033[1m";
        const char* ANSI_CYAN = "\033[36m";
        const char* ANSI_GREEN = "\033[32m";
        const char* ANSI_RESET = "\033[0m";
        
        out << ANSI_BOLD << ANSI_CYAN;
        out << "╔════════════════════════════════════════════════════╗" << Qt::endl;
        out << "║" << ANSI_GREEN << "              NESCA Network Scanner v2" << ANSI_CYAN << "              ║" << Qt::endl;
        out << "║" << ANSI_GREEN << "          Interactive Mode" << ANSI_CYAN << "                              ║" << Qt::endl;
        out << "╚════════════════════════════════════════════════════╝" << ANSI_RESET << Qt::endl;
    } else {
        out << "════════════════════════════════════════════════════" << Qt::endl;
        out << "           NESCA Network Scanner v2" << Qt::endl;
        out << "           Interactive Mode" << Qt::endl;
        out << "════════════════════════════════════════════════════" << Qt::endl;
    }
    out << Qt::endl;
}

void InteractiveMode::printMenu(const QStringList& options, const QString& title) {
    QTextStream out(stdout);
    
    if (colorEnabled) {
        const char* ANSI_BOLD = "\033[1m";
        const char* ANSI_CYAN = "\033[36m";
        const char* ANSI_RESET = "\033[0m";
        out << ANSI_BOLD << ANSI_CYAN << "=== " << title << " ===" << ANSI_RESET << Qt::endl;
    } else {
        out << "=== " << title << " ===" << Qt::endl;
    }
    
    for (int i = 0; i < options.size(); ++i) {
        if (colorEnabled) {
            const char* ANSI_YELLOW = "\033[33m";
            const char* ANSI_RESET = "\033[0m";
            out << ANSI_YELLOW << "[" << (i + 1) << "]" << ANSI_RESET << " " << options[i] << Qt::endl;
        } else {
            out << "[" << (i + 1) << "] " << options[i] << Qt::endl;
        }
    }
    
    if (colorEnabled) {
        const char* ANSI_YELLOW = "\033[33m";
        const char* ANSI_RESET = "\033[0m";
        out << ANSI_YELLOW << "[0]" << ANSI_RESET << " Exit" << Qt::endl;
    } else {
        out << "[0] Exit" << Qt::endl;
    }
    
    out << Qt::endl;
}

int InteractiveMode::getChoice(int maxOption) {
    QTextStream out(stdout);
    QTextStream in(stdin);
    
    while (true) {
        if (colorEnabled) {
            const char* ANSI_CYAN = "\033[36m";
            const char* ANSI_RESET = "\033[0m";
            out << ANSI_CYAN << "Your choice: " << ANSI_RESET;
        } else {
            out << "Your choice: ";
        }
        out.flush();
        
        QString input = in.readLine().trimmed();
        bool ok;
        int choice = input.toInt(&ok);
        
        if (ok && choice >= 0 && choice <= maxOption) {
            return choice;
        }
        
        if (colorEnabled) {
            const char* ANSI_RED = "\033[31m";
            const char* ANSI_RESET = "\033[0m";
            out << ANSI_RED << "Invalid choice. Please enter a number between 0 and " << maxOption << ANSI_RESET << Qt::endl;
        } else {
            out << "Invalid choice. Please enter a number between 0 and " << maxOption << Qt::endl;
        }
    }
}

QString InteractiveMode::getInput(const QString& prompt) {
    QTextStream out(stdout);
    QTextStream in(stdin);
    
    if (colorEnabled) {
        const char* ANSI_CYAN = "\033[36m";
        const char* ANSI_RESET = "\033[0m";
        out << ANSI_CYAN << prompt << ANSI_RESET;
    } else {
        out << prompt;
    }
    out.flush();
    
    return in.readLine().trimmed();
}

bool InteractiveMode::getYesNo(const QString& prompt, bool defaultValue) {
    QString defaultStr = defaultValue ? "Y/n" : "y/N";
    QString input = getInput(QString("%1 [%2]: ").arg(prompt, defaultStr));
    
    if (input.isEmpty()) {
        return defaultValue;
    }
    
    input = input.toLower();
    return input == "y" || input == "yes";
}

QString InteractiveMode::colorize(const QString& text, const QString& color) {
    if (!colorEnabled) {
        return text;
    }
    
    QMap<QString, QString> colorMap;
    colorMap["black"] = "\033[30m";
    colorMap["red"] = "\033[31m";
    colorMap["green"] = "\033[32m";
    colorMap["yellow"] = "\033[33m";
    colorMap["blue"] = "\033[34m";
    colorMap["magenta"] = "\033[35m";
    colorMap["cyan"] = "\033[36m";
    colorMap["white"] = "\033[37m";
    colorMap["bold"] = "\033[1m";
    colorMap["reset"] = "\033[0m";
    
    QString ansiCode = colorMap.value(color.toLower(), "");
    return ansiCode + text + colorMap["reset"];
}

void InteractiveMode::printProgressBar(int current, int total, int width) {
    QTextStream out(stdout);
    
    if (total == 0) return;
    
    float percentage = (float)current / (float)total;
    int filled = (int)(percentage * width);
    int empty = width - filled;
    
    QString bar;
    if (colorEnabled) {
        const char* ANSI_GREEN = "\033[32m";
        const char* ANSI_YELLOW = "\033[33m";
        const char* ANSI_RESET = "\033[0m";
        
        bar += ANSI_GREEN;
        for (int i = 0; i < filled; ++i) {
            bar += "█";
        }
        bar += ANSI_YELLOW;
        for (int i = 0; i < empty; ++i) {
            bar += "░";
        }
        bar += ANSI_RESET;
    } else {
        for (int i = 0; i < filled; ++i) {
            bar += "=";
        }
        for (int i = 0; i < empty; ++i) {
            bar += "-";
        }
    }
    
    out << "\r[" << bar << "] " << QString::number(percentage * 100, 'f', 1) << "% (" 
        << current << "/" << total << ")";
    out.flush();
    
    if (current >= total) {
        out << Qt::endl;
    }
}

void InteractiveMode::printStatus(const QString& message, const QString& status) {
    QTextStream out(stdout);
    
    QString prefix;
    if (status == "info") {
        prefix = colorize("[INFO]", "cyan");
    } else if (status == "success") {
        prefix = colorize("[OK]", "green");
    } else if (status == "warning") {
        prefix = colorize("[WARN]", "yellow");
    } else if (status == "error") {
        prefix = colorize("[ERROR]", "red");
    } else {
        prefix = "[INFO]";
    }
    
    out << prefix << " " << message << Qt::endl;
}

int InteractiveMode::scanModeMenu() {
    QStringList modes;
    modes << "IP Range Scan" 
          << "DNS Mask Scan"
          << "Import from File"
          << "Quick Presets";
    
    printMenu(modes, "Select Scan Mode");
    return getChoice(modes.size());
}

int InteractiveMode::scanSettingsMenu(QString& ports, int& threads, bool& adaptive, bool& smartScan) {
    QTextStream out(stdout);
    
    // Ports
    QString portsInput = getInput("Ports to scan (comma-separated, default: 80,443,8080): ");
    if (!portsInput.isEmpty()) {
        ports = portsInput;
    } else {
        ports = "80,443,8080";
    }
    
    // Threads
    QString threadsInput = getInput("Number of threads (default: 100): ");
    if (!threadsInput.isEmpty()) {
        bool ok;
        int t = threadsInput.toInt(&ok);
        if (ok && t > 0) {
            threads = t;
        }
    }
    
    // Adaptive
    adaptive = getYesNo("Enable adaptive scanning?", false);
    
    // Smart Scan
    smartScan = getYesNo("Enable smart scan (prioritize popular ports)?", false);
    
    return 0;
}

int InteractiveMode::exportSettingsMenu(QString& format, bool& autoExport) {
    QStringList formats;
    formats << "JSON"
            << "CSV"
            << "Both (JSON + CSV)";
    
    printMenu(formats, "Export Format");
    int choice = getChoice(formats.size());
    
    if (choice == 1) format = "json";
    else if (choice == 2) format = "csv";
    else if (choice == 3) format = "both";
    else format = "json";
    
    autoExport = getYesNo("Auto-export after scan?", true);
    
    return 0;
}

void InteractiveMode::executeScan(const QString& mode, const QString& target, 
                                  const QString& ports, int threads, bool adaptive, 
                                  bool smartScan, const QString& exportFormat) {
    QTextStream out(stdout);
    
    out << Qt::endl;
    printStatus("Starting scan...", "info");
    out << Qt::endl;
    
    // Build command
    QStringList args;
    args << target;
    
    if (mode == "ip") {
        args.prepend("--ip");
    } else if (mode == "dns") {
        args.prepend("--dns");
    } else if (mode == "import") {
        args.prepend("--import");
    }
    
    args << "-p" << ports;
    args << "-t" << QString::number(threads);
    
    if (adaptive) {
        args << "--adaptive";
    }
    if (smartScan) {
        args << "--smart-scan";
    }
    
    if (!exportFormat.isEmpty()) {
        args << "--export" << exportFormat;
    }
    
    if (colorEnabled) {
        args << "--color";
    }
    
    // Execute nesca process
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    
    QString program = QCoreApplication::applicationFilePath();
    process.start(program, args);
    
    if (!process.waitForStarted()) {
        printStatus("Failed to start scan process", "error");
        return;
    }
    
    // Read output with progress bar support
    QByteArray buffer;
    while (process.waitForReadyRead(-1)) {
        buffer += process.readAll();
        
        // Simple progress bar update (if output contains progress info)
        QString output = QString::fromUtf8(buffer);
        // Could parse progress from output here
    }
    
    process.waitForFinished();
    
    if (process.exitCode() == 0) {
        printStatus("Scan completed successfully", "success");
    } else {
        printStatus("Scan completed with errors", "warning");
    }
}

int InteractiveMode::run(QCoreApplication& app, int argc, char *argv[]) {
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    
    initializePresets();
    
    printBanner();
    
    while (true) {
        int modeChoice = scanModeMenu();
        
        if (modeChoice == 0) {
            printStatus("Exiting...", "info");
            return 0;
        }
        
        QString mode, target;
        
        if (modeChoice == 1) {
            // IP Range Scan
            target = getInput("Enter IP range (e.g., 192.168.1.0/24 or 192.168.1.1-192.168.1.255): ");
            if (target.isEmpty()) {
                printStatus("IP range is required", "error");
                continue;
            }
            mode = "ip";
        } else if (modeChoice == 2) {
            // DNS Mask Scan
            target = getInput("Enter DNS mask (e.g., test[a-z]): ");
            if (target.isEmpty()) {
                printStatus("DNS mask is required", "error");
                continue;
            }
            mode = "dns";
        } else if (modeChoice == 3) {
            // Import from File
            target = getInput("Enter file path: ");
            if (target.isEmpty()) {
                printStatus("File path is required", "error");
                continue;
            }
            mode = "import";
        } else if (modeChoice == 4) {
            // Quick Presets
            QStringList presetNames;
            for (auto it = quickPresets.constBegin(); it != quickPresets.constEnd(); ++it) {
                presetNames << it.value().description;
            }
            printMenu(presetNames, "Quick Presets");
            int presetChoice = getChoice(presetNames.size());
            
            if (presetChoice == 0) continue;
            
            QStringList keys = quickPresets.keys();
            QString presetKey = keys[presetChoice - 1];
            return runQuickPreset(presetKey, app);
        }
        
        // Scan settings
        QString ports = "80,443,8080";
        int threads = 100;
        bool adaptive = false;
        bool smartScan = false;
        scanSettingsMenu(ports, threads, adaptive, smartScan);
        
        // Export settings
        QString exportFormat = "json";
        bool autoExport = true;
        exportSettingsMenu(exportFormat, autoExport);
        
        // Confirm
        QTextStream out(stdout);
        out << Qt::endl;
        printStatus("Scan Configuration:", "info");
        out << "  Mode: " << mode << Qt::endl;
        out << "  Target: " << target << Qt::endl;
        out << "  Ports: " << ports << Qt::endl;
        out << "  Threads: " << threads << Qt::endl;
        out << "  Adaptive: " << (adaptive ? "Yes" : "No") << Qt::endl;
        out << "  Smart Scan: " << (smartScan ? "Yes" : "No") << Qt::endl;
        out << "  Export: " << exportFormat << Qt::endl;
        out << Qt::endl;
        
        if (!getYesNo("Start scan?", true)) {
            continue;
        }
        
        // Execute scan
        executeScan(mode, target, ports, threads, adaptive, smartScan, exportFormat);
        
        // Ask if continue
        if (!getYesNo("Perform another scan?", false)) {
            break;
        }
    }
    
    return 0;
}

bool InteractiveMode::runQuickPreset(const QString& presetName, QCoreApplication& app) {
    Q_UNUSED(app);
    initializePresets();
    
    if (!quickPresets.contains(presetName)) {
        printStatus("Preset not found: " + presetName, "error");
        return false;
    }
    
    ScanPreset preset = quickPresets[presetName];
    
    printBanner();
    printStatus("Quick Preset: " + preset.description, "info");
    
    QTextStream out(stdout);
    out << Qt::endl;
    
    QString target = getInput("Enter target (IP range, DNS mask, or file): ");
    if (target.isEmpty()) {
        printStatus("Target is required", "error");
        return false;
    }
    
    // Determine mode
    QString mode;
    if (target.contains("/") || target.contains("-")) {
        mode = "ip";
    } else if (target.contains("[") || target.contains("{")) {
        mode = "dns";
    } else {
        mode = "import";
    }
    
    // Build and execute command
    QStringList args;
    if (mode == "ip") {
        args << "--ip" << target;
    } else if (mode == "dns") {
        args << "--dns" << target;
    } else {
        args << "--import" << target;
    }
    
    args << "-p" << preset.ports;
    args << "-t" << QString::number(preset.threads);
    args.append(preset.additionalArgs);
    
    if (colorEnabled) {
        args << "--color";
    }
    
    printStatus("Starting " + preset.description + "...", "info");
    out << Qt::endl;
    
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    QString program = QCoreApplication::applicationFilePath();
    process.start(program, args);
    process.waitForFinished(-1);
    
    return process.exitCode() == 0;
}

