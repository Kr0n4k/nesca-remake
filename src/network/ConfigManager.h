#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

struct ScanConfig {
    int threads;
    QString ports;
    int timeout;
    int maxRate;
    int retries;
    bool verifySSL;
    bool adaptive;
    bool smartScan;
    int batchSize;
    QString userAgent;
    QString tld;
    bool shuffle;
};

struct DetectionConfig {
    bool deepScan;
    bool checkVulnerabilities;
    bool enableFingerprinting;
    QStringList enabledDetectors;
};

struct ExportConfig {
    QStringList formats;  // json, csv, html, both
    bool autoExport;
    QString outputFile;
    QString filterIP;
    QStringList filterPorts;
    QStringList filterTypes;
    bool autoOpen;
};

class ConfigManager {
private:
    static QMap<QString, ScanConfig> defaultProfiles;
    static void initializeDefaultProfiles();
    
    ScanConfig scanConfig;
    DetectionConfig detectionConfig;
    ExportConfig exportConfig;
    QString configFilePath;
    bool configLoaded;
    
public:
    ConfigManager();
    ~ConfigManager();
    
    // Load configuration from file
    bool loadConfig(const QString& filePath);
    
    // Load pre-defined profile
    bool loadProfile(const QString& profileName);
    
    // Get configuration
    ScanConfig getScanConfig() const { return scanConfig; }
    DetectionConfig getDetectionConfig() const { return detectionConfig; }
    ExportConfig getExportConfig() const { return exportConfig; }
    
    // Set configuration (for override from command line)
    void setScanConfig(const ScanConfig& config) { scanConfig = config; }
    void setDetectionConfig(const DetectionConfig& config) { detectionConfig = config; }
    void setExportConfig(const ExportConfig& config) { exportConfig = config; }
    
    // Get available profiles
    static QStringList getAvailableProfiles();
    
    // Merge command line arguments with config (CLI takes precedence)
    void mergeCommandLineArgs(const QMap<QString, QString>& args);
    
    // Validate configuration
    bool validate() const;
    
    // Save current configuration to file
    bool saveConfig(const QString& filePath) const;
    
private:
    void applyDefaults();
    void parseScanSection(QSettings& settings);
    void parseDetectionSection(QSettings& settings);
    void parseExportSection(QSettings& settings);
    QString expandProfilePath(const QString& profileName) const;
};

#endif // CONFIGMANAGER_H

