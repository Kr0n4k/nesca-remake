#ifndef FINGERPRINTSCANNER_H
#define FINGERPRINTSCANNER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>
#include <string>

struct FingerprintResult {
    bool detected;
    QString deviceType;      // "onvif", "rtsp_stream", "api_camera", "snmp_device"
    QString manufacturer;    // Manufacturer name
    QString model;           // Model number
    QString version;         // Firmware/API version
    QString endpoint;        // Detected endpoint URL
    QString details;         // Additional details (JSON string)
    int confidence;          // 0-100 confidence level
};

class FingerprintScanner {
public:
    // ONVIF device detection
    static FingerprintResult checkONVIF(const char* ip, int port = 80);
    
    // RTSP stream auto-detection
    static FingerprintResult checkRTSPStreams(const char* ip, int port = 554);
    
    // Popular manufacturer API endpoints
    static FingerprintResult checkManufacturerAPIs(const char* ip, int port);
    
    // SNMP scanning
    static FingerprintResult checkSNMP(const char* ip, int port = 161);
    
    // MikroTik RouterOS / Winbox detection
    static FingerprintResult checkMikroTik(const char* ip, int port = 80);
    
    // Ubiquiti UniFi / AirOS detection
    static FingerprintResult checkUbiquiti(const char* ip, int port = 80);
    
    // TP-Link device detection (routers, cameras, IoT)
    static FingerprintResult checkTPLink(const char* ip, int port = 80);
    
    // IoT Smart Devices detection (smart bulbs, plugs, etc.)
    static FingerprintResult checkIoTDevices(const char* ip, int port = 80);
    
    // Comprehensive fingerprint scan
    static FingerprintResult scanDevice(const char* ip, int port);
    
    // Get all detected endpoints
    static QStringList getDetectedEndpoints(const char* ip, int port);
    
private:
    // ONVIF helper methods
    static bool checkONVIFEndpoint(const char* ip, int port, const char* path);
    static QString parseONVIFDeviceInfo(const std::string& response);
    
    // RTSP stream URLs to check
    static QStringList getRTSPStreamURLs();
    
    // Manufacturer API endpoints
    static QMap<QString, QStringList> getManufacturerEndpoints();
    
    // SNMP community strings (common defaults)
    static QStringList getSNMPCommunityStrings();
    static QString querySNMP(const char* ip, const char* community, int port, const char* oid);
};

#endif // FINGERPRINTSCANNER_H

