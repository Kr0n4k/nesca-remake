#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QIODevice>
#include <memory>

class ResultExporter;

struct DeviceSnapshot {
    QString ip;
    int port;
    QString deviceType;
    QString manufacturer;
    QString model;
    QString firmware;
    QDateTime firstSeen;
    QDateTime lastSeen;
    int scanCount;
    QJsonObject metadata;
};

struct ScanSnapshot {
    QDateTime timestamp;
    QString scanTarget;
    QString mode;
    QMap<QString, DeviceSnapshot> devices;  // Key: "IP:Port"
    int totalDevices;
    int newDevices;
    int removedDevices;
    int changedDevices;
};

struct MonitorDiff {
    QList<DeviceSnapshot> newDevices;
    QList<DeviceSnapshot> removedDevices;
    QList<QPair<DeviceSnapshot, DeviceSnapshot>> changedDevices;  // old, new
    int totalNew;
    int totalRemoved;
    int totalChanged;
};

class NetworkMonitor {
private:
    static QMutex snapshotMutex;
    QString snapshotDirectory;
    QString currentTarget;
    QString currentMode;
    ScanSnapshot lastSnapshot;
    ScanSnapshot currentSnapshot;
    bool monitoringActive;
    int scanInterval;  // seconds
    QTimer* scanTimer;
    bool alertOnNewDevice;
    QSet<QString> notifiedDevices;  // Track already notified devices
    
    // Snapshot management
    QString getSnapshotPath(const QDateTime& timestamp) const;
    bool saveSnapshot(const ScanSnapshot& snapshot) const;
    ScanSnapshot loadSnapshot(const QString& filePath) const;
    ScanSnapshot loadLastSnapshot(const QString& target) const;
    
    // Comparison
    MonitorDiff compareSnapshots(const ScanSnapshot& oldSnapshot, const ScanSnapshot& newSnapshot) const;
    bool devicesEqual(const DeviceSnapshot& d1, const DeviceSnapshot& d2) const;
    
    // Device key generation
    QString makeDeviceKey(const QString& ip, int port) const;
    QString makeDeviceKey(const DeviceSnapshot& device) const;
    
public:
    NetworkMonitor();
    ~NetworkMonitor();
    
    // Configuration
    void setSnapshotDirectory(const QString& dir);
    void setScanInterval(int intervalSeconds);
    void setAlertOnNewDevice(bool enable) { alertOnNewDevice = enable; }
    
    // Start/stop monitoring
    bool startMonitoring(const QString& target, const QString& mode, int intervalSeconds = 300);
    void stopMonitoring();
    bool isMonitoring() const { return monitoringActive; }
    
    // Snapshot operations
    void createSnapshot(const QMap<QString, DeviceSnapshot>& devices, const QString& target, const QString& mode);
    bool saveCurrentSnapshot();
    ScanSnapshot getLastSnapshot() const { return lastSnapshot; }
    ScanSnapshot getCurrentSnapshot() const { return currentSnapshot; }
    
    // Helper to convert results to snapshots
    void createSnapshotFromExporter(const ResultExporter& exporter, const QString& target, const QString& mode);
    
    // Comparison and diff
    MonitorDiff getDiff() const;
    MonitorDiff compareWithLast() const;
    QString formatDiff(const MonitorDiff& diff) const;
    void printDiff(const MonitorDiff& diff) const;
    
    // Alerts
    void checkAndAlert(const MonitorDiff& diff);
    void clearNotifiedDevices() { notifiedDevices.clear(); }
    
    // Statistics
    int getTotalMonitoredDevices() const;
    QDateTime getLastScanTime() const;
    int getScanCount() const;
    
    // Snapshot history
    QStringList getSnapshotHistory(const QString& target) const;
    MonitorDiff compareSnapshotsAt(int index1, int index2) const;
};

#endif // NETWORKMONITOR_H

