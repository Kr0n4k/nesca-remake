#include <ConfigManager.h>
#include <QTextStream>
#include <QStandardPaths>
#include <iostream>

QMap<QString, ScanConfig> ConfigManager::defaultProfiles;

void ConfigManager::initializeDefaultProfiles() {
    if (!defaultProfiles.isEmpty()) return;
    
    // Quick scan profile
    ScanConfig quick;
    quick.threads = 100;
    quick.ports = "80,443,8080";
    quick.timeout = 3000;
    quick.maxRate = 0;
    quick.retries = 0;
    quick.verifySSL = false;
    quick.adaptive = false;
    quick.smartScan = false;
    quick.batchSize = 0;
    quick.userAgent = "";
    quick.tld = ".com";
    quick.shuffle = true;
    defaultProfiles["quick-scan"] = quick;
    
    // Full scan profile
    ScanConfig full;
    full.threads = 200;
    full.ports = "21,22,23,80,443,554,8080,8443,9000";
    full.timeout = 5000;
    full.maxRate = 0;
    full.retries = 1;
    full.verifySSL = false;
    full.adaptive = true;
    full.smartScan = true;
    full.batchSize = 50;
    full.userAgent = "";
    full.tld = ".com";
    full.shuffle = true;
    defaultProfiles["full-scan"] = full;
    
    // Stealth scan profile
    ScanConfig stealth;
    stealth.threads = 50;
    stealth.ports = "80,443";
    stealth.timeout = 10000;
    stealth.maxRate = 10;
    stealth.retries = 2;
    stealth.verifySSL = true;
    stealth.adaptive = true;
    stealth.smartScan = false;
    stealth.batchSize = 10;
    stealth.userAgent = "Mozilla/5.0";
    stealth.tld = ".com";
    stealth.shuffle = true;
    defaultProfiles["stealth-scan"] = stealth;
    
    // IoT scan profile
    ScanConfig iot;
    iot.threads = 150;
    iot.ports = "80,8080,8081,8443";
    iot.timeout = 3000;
    iot.maxRate = 0;
    iot.retries = 0;
    iot.verifySSL = false;
    iot.adaptive = false;
    iot.smartScan = true;
    iot.batchSize = 100;
    iot.userAgent = "";
    iot.tld = ".com";
    iot.shuffle = false;
    defaultProfiles["iot-scan"] = iot;
    
    // Network equipment scan
    ScanConfig network;
    network.threads = 200;
    network.ports = "80,443,8080,8291,8443";
    network.timeout = 5000;
    network.maxRate = 0;
    network.retries = 0;
    network.verifySSL = false;
    network.adaptive = true;
    network.smartScan = true;
    network.batchSize = 0;
    network.userAgent = "";
    network.tld = ".com";
    network.shuffle = true;
    defaultProfiles["network-scan"] = network;
}

ConfigManager::ConfigManager() : configLoaded(false) {
    applyDefaults();
    initializeDefaultProfiles();
}

ConfigManager::~ConfigManager() {
}

void ConfigManager::applyDefaults() {
    // Default scan config
    scanConfig.threads = 100;
    scanConfig.ports = "80,443,8080";
    scanConfig.timeout = 3000;
    scanConfig.maxRate = 0;
    scanConfig.retries = 0;
    scanConfig.verifySSL = false;
    scanConfig.adaptive = false;
    scanConfig.smartScan = false;
    scanConfig.batchSize = 0;
    scanConfig.userAgent = "";
    scanConfig.tld = ".com";
    scanConfig.shuffle = true;
    
    // Default detection config
    detectionConfig.deepScan = false;
    detectionConfig.checkVulnerabilities = false;
    detectionConfig.enableFingerprinting = true;
    detectionConfig.enabledDetectors = QStringList() << "onvif" << "rtsp" << "manufacturer" << "snmp";
    
    // Default export config
    exportConfig.formats = QStringList() << "json";
    exportConfig.autoExport = true;
    exportConfig.outputFile = "";
    exportConfig.filterIP = "";
    exportConfig.filterPorts = QStringList();
    exportConfig.filterTypes = QStringList();
    exportConfig.autoOpen = false;
}

bool ConfigManager::loadConfig(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        QTextStream err(stderr);
        err << "[ERROR] Configuration file not found or not readable: " << filePath << Qt::endl;
        return false;
    }
    
    QSettings settings(filePath, QSettings::IniFormat);
    
    if (settings.status() != QSettings::NoError) {
        QTextStream err(stderr);
        err << "[ERROR] Failed to parse configuration file: " << filePath << Qt::endl;
        return false;
    }
    
    configFilePath = filePath;
    
    // Parse sections
    parseScanSection(settings);
    parseDetectionSection(settings);
    parseExportSection(settings);
    
    configLoaded = true;
    
    QTextStream out(stdout);
    out << "[INFO] Configuration loaded from: " << filePath << Qt::endl;
    
    return true;
}

bool ConfigManager::loadProfile(const QString& profileName) {
    initializeDefaultProfiles();
    
    if (defaultProfiles.contains(profileName)) {
        scanConfig = defaultProfiles[profileName];
        
        // Apply default detection/export for profiles
        if (profileName == "full-scan" || profileName == "network-scan") {
            detectionConfig.deepScan = true;
            detectionConfig.enableFingerprinting = true;
        }
        
        if (profileName == "full-scan") {
            exportConfig.formats = QStringList() << "json" << "html";
        }
        
        QTextStream out(stdout);
        out << "[INFO] Profile loaded: " << profileName << Qt::endl;
        return true;
    }
    
    // Try to load from file
    QString profilePath = expandProfilePath(profileName);
    if (QFileInfo(profilePath).exists()) {
        return loadConfig(profilePath);
    }
    
    QTextStream err(stderr);
    err << "[ERROR] Profile not found: " << profileName << Qt::endl;
    err << "[INFO] Available profiles: " << getAvailableProfiles().join(", ") << Qt::endl;
    return false;
}

void ConfigManager::parseScanSection(QSettings& settings) {
    settings.beginGroup("scan");
    
    if (settings.contains("threads")) {
        scanConfig.threads = settings.value("threads").toInt();
    }
    if (settings.contains("ports")) {
        scanConfig.ports = settings.value("ports").toString();
    }
    if (settings.contains("timeout")) {
        scanConfig.timeout = settings.value("timeout").toInt();
    }
    if (settings.contains("max_rate")) {
        scanConfig.maxRate = settings.value("max_rate").toInt();
    }
    if (settings.contains("retries")) {
        scanConfig.retries = settings.value("retries").toInt();
    }
    if (settings.contains("verify_ssl")) {
        scanConfig.verifySSL = settings.value("verify_ssl").toBool();
    }
    if (settings.contains("adaptive")) {
        scanConfig.adaptive = settings.value("adaptive").toBool();
    }
    if (settings.contains("smart_scan")) {
        scanConfig.smartScan = settings.value("smart_scan").toBool();
    }
    if (settings.contains("batch_size")) {
        scanConfig.batchSize = settings.value("batch_size").toInt();
    }
    if (settings.contains("user_agent")) {
        scanConfig.userAgent = settings.value("user_agent").toString();
    }
    if (settings.contains("tld")) {
        scanConfig.tld = settings.value("tld").toString();
    }
    if (settings.contains("shuffle")) {
        scanConfig.shuffle = settings.value("shuffle").toBool();
    }
    
    settings.endGroup();
}

void ConfigManager::parseDetectionSection(QSettings& settings) {
    settings.beginGroup("detection");
    
    if (settings.contains("deep_scan")) {
        detectionConfig.deepScan = settings.value("deep_scan").toBool();
    }
    if (settings.contains("check_vulnerabilities")) {
        detectionConfig.checkVulnerabilities = settings.value("check_vulnerabilities").toBool();
    }
    if (settings.contains("enable_fingerprinting")) {
        detectionConfig.enableFingerprinting = settings.value("enable_fingerprinting").toBool();
    }
    if (settings.contains("enabled_detectors")) {
        QString detectorsStr = settings.value("enabled_detectors").toString();
        detectionConfig.enabledDetectors = detectorsStr.split(",", Qt::SkipEmptyParts);
        for (QString& detector : detectionConfig.enabledDetectors) {
            detector = detector.trimmed();
        }
    }
    
    settings.endGroup();
}

void ConfigManager::parseExportSection(QSettings& settings) {
    settings.beginGroup("export");
    
    if (settings.contains("format")) {
        QString formatStr = settings.value("format").toString();
        exportConfig.formats = formatStr.split(",", Qt::SkipEmptyParts);
        for (QString& format : exportConfig.formats) {
            format = format.trimmed();
            if (format == "both") {
                exportConfig.formats.removeOne("both");
                exportConfig.formats << "json" << "csv";
            }
        }
    }
    if (settings.contains("auto_export")) {
        exportConfig.autoExport = settings.value("auto_export").toBool();
    }
    if (settings.contains("output_file")) {
        exportConfig.outputFile = settings.value("output_file").toString();
    }
    if (settings.contains("filter_ip")) {
        exportConfig.filterIP = settings.value("filter_ip").toString();
    }
    if (settings.contains("filter_port")) {
        QString portsStr = settings.value("filter_port").toString();
        exportConfig.filterPorts = portsStr.split(",", Qt::SkipEmptyParts);
        for (QString& port : exportConfig.filterPorts) {
            port = port.trimmed();
        }
    }
    if (settings.contains("filter_type")) {
        QString typesStr = settings.value("filter_type").toString();
        exportConfig.filterTypes = typesStr.split(",", Qt::SkipEmptyParts);
        for (QString& type : exportConfig.filterTypes) {
            type = type.trimmed();
        }
    }
    if (settings.contains("auto_open")) {
        exportConfig.autoOpen = settings.value("auto_open").toBool();
    }
    
    settings.endGroup();
}

QString ConfigManager::expandProfilePath(const QString& profileName) const {
    // Try current directory
    QString currentDir = QDir::currentPath() + "/" + profileName + ".conf";
    if (QFileInfo(currentDir).exists()) {
        return currentDir;
    }
    
    // Try configs directory
    QString configsDir = QDir::currentPath() + "/configs/" + profileName + ".conf";
    if (QFileInfo(configsDir).exists()) {
        return configsDir;
    }
    
    // Try home directory
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.nesca/" + profileName + ".conf";
    if (QFileInfo(homeDir).exists()) {
        return homeDir;
    }
    
    return profileName + ".conf";
}

QStringList ConfigManager::getAvailableProfiles() {
    initializeDefaultProfiles();
    QStringList profiles = defaultProfiles.keys();
    
    // Also check for config files in configs/ directory
    QDir configsDir(QDir::currentPath() + "/configs");
    if (configsDir.exists()) {
        QStringList filters;
        filters << "*.conf";
        QFileInfoList files = configsDir.entryInfoList(filters, QDir::Files);
        for (const QFileInfo& file : files) {
            QString baseName = file.baseName();
            if (!profiles.contains(baseName)) {
                profiles << baseName;
            }
        }
    }
    
    return profiles;
}

void ConfigManager::mergeCommandLineArgs(const QMap<QString, QString>& args) {
    if (args.contains("threads")) {
        scanConfig.threads = args["threads"].toInt();
    }
    if (args.contains("ports")) {
        scanConfig.ports = args["ports"];
    }
    if (args.contains("timeout")) {
        scanConfig.timeout = args["timeout"].toInt();
    }
    if (args.contains("max-rate")) {
        scanConfig.maxRate = args["max-rate"].toInt();
    }
    if (args.contains("retries")) {
        scanConfig.retries = args["retries"].toInt();
    }
    if (args.contains("verify-ssl")) {
        scanConfig.verifySSL = true;
    }
    if (args.contains("adaptive")) {
        scanConfig.adaptive = true;
    }
    if (args.contains("smart-scan")) {
        scanConfig.smartScan = true;
    }
    if (args.contains("batch-size")) {
        scanConfig.batchSize = args["batch-size"].toInt();
    }
    if (args.contains("user-agent")) {
        scanConfig.userAgent = args["user-agent"];
    }
    if (args.contains("tld")) {
        scanConfig.tld = args["tld"];
    }
    if (args.contains("export")) {
        QString format = args["export"];
        exportConfig.formats.clear();
        if (format == "both") {
            exportConfig.formats << "json" << "csv";
        } else {
            exportConfig.formats << format;
        }
    }
    if (args.contains("no-export")) {
        exportConfig.autoExport = false;
        exportConfig.formats.clear();
    }
}

bool ConfigManager::validate() const {
    if (scanConfig.threads < 1 || scanConfig.threads > 10000) {
        return false;
    }
    if (scanConfig.timeout < 1 || scanConfig.timeout > 60000) {
        return false;
    }
    if (scanConfig.maxRate < 0 || scanConfig.maxRate > 100000) {
        return false;
    }
    if (scanConfig.retries < 0 || scanConfig.retries > 10) {
        return false;
    }
    if (scanConfig.batchSize < 0 || scanConfig.batchSize > 10000) {
        return false;
    }
    return true;
}

bool ConfigManager::saveConfig(const QString& filePath) const {
    QSettings settings(filePath, QSettings::IniFormat);
    
    // Save scan section
    settings.beginGroup("scan");
    settings.setValue("threads", scanConfig.threads);
    settings.setValue("ports", scanConfig.ports);
    settings.setValue("timeout", scanConfig.timeout);
    settings.setValue("max_rate", scanConfig.maxRate);
    settings.setValue("retries", scanConfig.retries);
    settings.setValue("verify_ssl", scanConfig.verifySSL);
    settings.setValue("adaptive", scanConfig.adaptive);
    settings.setValue("smart_scan", scanConfig.smartScan);
    settings.setValue("batch_size", scanConfig.batchSize);
    if (!scanConfig.userAgent.isEmpty()) {
        settings.setValue("user_agent", scanConfig.userAgent);
    }
    settings.setValue("tld", scanConfig.tld);
    settings.setValue("shuffle", scanConfig.shuffle);
    settings.endGroup();
    
    // Save detection section
    settings.beginGroup("detection");
    settings.setValue("deep_scan", detectionConfig.deepScan);
    settings.setValue("check_vulnerabilities", detectionConfig.checkVulnerabilities);
    settings.setValue("enable_fingerprinting", detectionConfig.enableFingerprinting);
    settings.setValue("enabled_detectors", detectionConfig.enabledDetectors.join(","));
    settings.endGroup();
    
    // Save export section
    settings.beginGroup("export");
    QStringList formats = exportConfig.formats;
    if (formats.contains("json") && formats.contains("csv")) {
        formats.removeAll("json");
        formats.removeAll("csv");
        formats << "both";
    }
    settings.setValue("format", formats.join(","));
    settings.setValue("auto_export", exportConfig.autoExport);
    if (!exportConfig.outputFile.isEmpty()) {
        settings.setValue("output_file", exportConfig.outputFile);
    }
    if (!exportConfig.filterIP.isEmpty()) {
        settings.setValue("filter_ip", exportConfig.filterIP);
    }
    if (!exportConfig.filterPorts.isEmpty()) {
        settings.setValue("filter_port", exportConfig.filterPorts.join(","));
    }
    if (!exportConfig.filterTypes.isEmpty()) {
        settings.setValue("filter_type", exportConfig.filterTypes.join(","));
    }
    settings.setValue("auto_open", exportConfig.autoOpen);
    settings.endGroup();
    
    settings.sync();
    return settings.status() == QSettings::NoError;
}

