#ifndef SERVICEVERSIONDETECTOR_H
#define SERVICEVERSIONDETECTOR_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>
#include <QRegularExpression>

struct ServiceVersion {
    QString serviceName;
    QString version;
    QString fullVersion;
    QString vendor;
    QString product;
    int confidence;  // 0-100
    QString detectionMethod;  // header, banner, path, etc.
    QJsonObject metadata;
};

struct VersionScanResult {
    QString ip;
    int port;
    QList<ServiceVersion> detectedServices;
    QString primaryService;
    QString primaryVersion;
    QMap<QString, QString> allVersions;  // service -> version
    QJsonObject scanMetadata;
};

class ServiceVersionDetector {
private:
    static QMap<int, QStringList> portToServiceMap;
    static QMap<QString, QRegularExpression> versionPatterns;
    static void initializePatterns();
    
    static ServiceVersion detectFromHTTPHeaders(const QString& ip, int port);
    static ServiceVersion detectFromBanner(const QString& ip, int port);
    static ServiceVersion detectFromPath(const QString& ip, int port, const QString& path);
    static ServiceVersion detectFromResponseBody(const QString& ip, int port);
    
    // Service-specific detection
    static ServiceVersion detectSSHVersion(const QString& ip, int port);
    static ServiceVersion detectHTTPVersion(const QString& ip, int port);
    static ServiceVersion detectFTPVersion(const QString& ip, int port);
    static ServiceVersion detectTelnetVersion(const QString& ip, int port);
    static ServiceVersion detectRTSPVersion(const QString& ip, int port);
    static ServiceVersion detectONVIFVersion(const QString& ip, int port);
    
    // Parse version strings
    static QString extractVersion(const QString& text, const QRegularExpression& pattern);
    static int calculateConfidence(const QString& version, const QString& method);
    
public:
    ServiceVersionDetector();
    
    // Detect service version for a port
    static VersionScanResult detectVersions(const QString& ip, int port);
    
    // Detect all service versions for multiple ports
    static QMap<int, VersionScanResult> detectVersionsForPorts(
        const QString& ip, const QList<int>& ports);
    
    // Quick version check for specific service
    static QString getServiceVersion(const QString& ip, int port, 
                                    const QString& serviceName);
};

#endif // SERVICEVERSIONDETECTOR_H

