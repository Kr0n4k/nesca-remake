#ifndef DEEPSCANNER_H
#define DEEPSCANNER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QJsonObject>

struct DeviceEndpoint {
    QString path;
    QString method;  // GET, POST, etc.
    QString description;
    QStringList parameters;
    bool requiresAuth;
};

struct DeepScanResult {
    QString deviceIP;
    int devicePort;
    QString deviceType;
    QString manufacturer;
    QString model;
    QList<DeviceEndpoint> discoveredEndpoints;
    QStringList discoveredPaths;
    QMap<QString, QString> headers;
    QMap<QString, QString> cookies;
    QString firmwareVersion;
    QString hardwareVersion;
    QJsonObject metadata;
};

class DeepScanner {
private:
    static QStringList commonCameraPaths;
    static QStringList commonDVRPaths;
    static QStringList commonRouterPaths;
    static QStringList commonServerPaths;
    static void initializePaths();
    
    static QStringList getPathsForDeviceType(const QString& deviceType);
    static bool checkEndpoint(const QString& ip, int port, const QString& path, 
                             const QString& method = "GET");
    static QString getResponseBody(const QString& ip, int port, const QString& path,
                                  const QString& method = "GET");
    static QMap<QString, QString> extractHeaders(const QString& response);
    static QStringList extractLinks(const QString& html);
    static QStringList extractForms(const QString& html);
    
public:
    DeepScanner();
    
    // Perform deep scan on discovered device
    static DeepScanResult scanDevice(const QString& ip, int port, 
                                     const QString& deviceType = "",
                                     const QString& manufacturer = "");
    
    // Scan subdomains/alternate ports for discovered camera
    static QStringList scanAlternatePorts(const QString& ip, const QStringList& ports);
    static QStringList scanCommonPaths(const QString& ip, int port, 
                                      const QStringList& paths);
    
    // Extract service information
    static QString detectFirmwareVersion(const QString& ip, int port, 
                                        const QString& deviceType);
    static QString detectHardwareVersion(const QString& ip, int port,
                                        const QString& manufacturer);
};

#endif // DEEPSCANNER_H

