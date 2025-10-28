#ifndef DEVICEIDENTIFIER_H
#define DEVICEIDENTIFIER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QRegularExpression>

struct DeviceInfo {
    QString manufacturer;  // Hikvision, Dahua, Axis, etc.
    QString model;
    QString firmware;
    QString deviceType;    // camera, dvr, nvr, etc.
    QString confidence;    // high, medium, low
};

class DeviceIdentifier {
private:
    static QMap<QString, QString> manufacturerPatterns;
    static QMap<QString, QString> modelPatterns;
    static void initializePatterns();
    
public:
    DeviceIdentifier();
    
    // Identify device from HTTP headers or response body
    static DeviceInfo identify(const QString &headers, const QString &body = "");
    
    // Get device manufacturer
    static QString getManufacturer(const QString &headers, const QString &body = "");
    
    // Get device model
    static QString getModel(const QString &headers, const QString &body = "");
    
    // Check if device is a specific brand
    static bool isBrand(const QString &brand, const QString &headers, const QString &body = "");
    
    // Get full device info as JSON string
    static QString getDeviceInfoJSON(const QString &headers, const QString &body = "");
};

#endif // DEVICEIDENTIFIER_H

