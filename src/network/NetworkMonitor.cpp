#include <NetworkMonitor.h>
#include <ResultExporter.h>
#include <DeviceIdentifier.h>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>
#include <iostream>
#include <chrono>

QMutex NetworkMonitor::snapshotMutex;

NetworkMonitor::NetworkMonitor() 
    : monitoringActive(false)
    , scanInterval(300)
    , scanTimer(nullptr)
    , alertOnNewDevice(false)
{
    // Default snapshot directory
    snapshotDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir;
    if (!dir.exists(snapshotDirectory)) {
        dir.mkpath(snapshotDirectory);
    }
    
    scanTimer = new QTimer();
    scanTimer->setSingleShot(false);
}

NetworkMonitor::~NetworkMonitor() {
    stopMonitoring();
    if (scanTimer) {
        delete scanTimer;
    }
}

void NetworkMonitor::setSnapshotDirectory(const QString& dir) {
    QMutexLocker locker(&snapshotMutex);
    snapshotDirectory = dir;
    QDir qdir;
    if (!qdir.exists(snapshotDirectory)) {
        qdir.mkpath(snapshotDirectory);
    }
}

void NetworkMonitor::setScanInterval(int intervalSeconds) {
    scanInterval = intervalSeconds;
    if (scanTimer && scanTimer->isActive()) {
        scanTimer->stop();
        scanTimer->start(scanInterval * 1000);
    }
}

bool NetworkMonitor::startMonitoring(const QString& target, const QString& mode, int intervalSeconds) {
    if (monitoringActive) {
        return false; // Already monitoring
    }
    
    currentTarget = target;
    currentMode = mode;
    scanInterval = intervalSeconds > 0 ? intervalSeconds : scanInterval;
    monitoringActive = true;
    
    // Load last snapshot if exists
    lastSnapshot = loadLastSnapshot(target);
    
    // Create snapshot directory for this target
    QString targetDir = snapshotDirectory + "/" + target.replace("/", "_").replace(" ", "_");
    QDir dir;
    if (!dir.exists(targetDir)) {
        dir.mkpath(targetDir);
    }
    
    QTextStream out(stdout);
    out << "[INFO] Monitoring started for: " << target << Qt::endl;
    out << "[INFO] Scan interval: " << scanInterval << " seconds" << Qt::endl;
    out << "[INFO] Snapshot directory: " << targetDir << Qt::endl;
    
    return true;
}

void NetworkMonitor::stopMonitoring() {
    if (!monitoringActive) {
        return;
    }
    
    if (scanTimer) {
        scanTimer->stop();
    }
    
    // Save current snapshot before stopping
    if (!currentSnapshot.devices.isEmpty()) {
        saveCurrentSnapshot();
    }
    
    monitoringActive = false;
    
    QTextStream out(stdout);
    out << "[INFO] Monitoring stopped" << Qt::endl;
}

void NetworkMonitor::createSnapshot(const QMap<QString, DeviceSnapshot>& devices, const QString& target, const QString& mode) {
    QMutexLocker locker(&snapshotMutex);
    
    // Move current to last
    lastSnapshot = currentSnapshot;
    
    // Create new snapshot
    currentSnapshot.timestamp = QDateTime::currentDateTime();
    currentSnapshot.scanTarget = target;
    currentSnapshot.mode = mode;
    currentSnapshot.devices = devices;
    currentSnapshot.totalDevices = devices.size();
    
    // Compare with last snapshot to calculate diffs
    if (!lastSnapshot.devices.isEmpty()) {
        MonitorDiff diff = compareSnapshots(lastSnapshot, currentSnapshot);
        currentSnapshot.newDevices = diff.totalNew;
        currentSnapshot.removedDevices = diff.totalRemoved;
        currentSnapshot.changedDevices = diff.totalChanged;
    } else {
        currentSnapshot.newDevices = devices.size();
        currentSnapshot.removedDevices = 0;
        currentSnapshot.changedDevices = 0;
    }
}

bool NetworkMonitor::saveCurrentSnapshot() {
    if (currentSnapshot.devices.isEmpty()) {
        return false;
    }
    
    QString filePath = getSnapshotPath(currentSnapshot.timestamp);
    QDir dir = QFileInfo(filePath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }
    
    QJsonObject json;
    json["timestamp"] = currentSnapshot.timestamp.toString(Qt::ISODate);
    json["scanTarget"] = currentSnapshot.scanTarget;
    json["mode"] = currentSnapshot.mode;
    json["totalDevices"] = currentSnapshot.totalDevices;
    json["newDevices"] = currentSnapshot.newDevices;
    json["removedDevices"] = currentSnapshot.removedDevices;
    json["changedDevices"] = currentSnapshot.changedDevices;
    
    QJsonObject devicesJson;
    for (auto it = currentSnapshot.devices.constBegin(); it != currentSnapshot.devices.constEnd(); ++it) {
        const DeviceSnapshot& dev = it.value();
        QJsonObject devJson;
        devJson["ip"] = dev.ip;
        devJson["port"] = dev.port;
        devJson["deviceType"] = dev.deviceType;
        devJson["manufacturer"] = dev.manufacturer;
        devJson["model"] = dev.model;
        devJson["firmware"] = dev.firmware;
        devJson["firstSeen"] = dev.firstSeen.toString(Qt::ISODate);
        devJson["lastSeen"] = dev.lastSeen.toString(Qt::ISODate);
        devJson["scanCount"] = dev.scanCount;
        if (!dev.metadata.isEmpty()) {
            devJson["metadata"] = dev.metadata;
        }
        devicesJson[it.key()] = devJson;
    }
    json["devices"] = devicesJson;
    
    QJsonDocument doc(json);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

ScanSnapshot NetworkMonitor::loadSnapshot(const QString& filePath) const {
    ScanSnapshot snapshot;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return snapshot;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return snapshot;
    }
    
    QJsonObject json = doc.object();
    
    snapshot.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    snapshot.scanTarget = json["scanTarget"].toString();
    snapshot.mode = json["mode"].toString();
    snapshot.totalDevices = json["totalDevices"].toInt();
    snapshot.newDevices = json["newDevices"].toInt();
    snapshot.removedDevices = json["removedDevices"].toInt();
    snapshot.changedDevices = json["changedDevices"].toInt();
    
    QJsonObject devicesJson = json["devices"].toObject();
    for (auto it = devicesJson.constBegin(); it != devicesJson.constEnd(); ++it) {
        QJsonObject devJson = it.value().toObject();
        DeviceSnapshot dev;
        dev.ip = devJson["ip"].toString();
        dev.port = devJson["port"].toInt();
        dev.deviceType = devJson["deviceType"].toString();
        dev.manufacturer = devJson["manufacturer"].toString();
        dev.model = devJson["model"].toString();
        dev.firmware = devJson["firmware"].toString();
        dev.firstSeen = QDateTime::fromString(devJson["firstSeen"].toString(), Qt::ISODate);
        dev.lastSeen = QDateTime::fromString(devJson["lastSeen"].toString(), Qt::ISODate);
        dev.scanCount = devJson["scanCount"].toInt();
        dev.metadata = devJson["metadata"].toObject();
        snapshot.devices[it.key()] = dev;
    }
    
    return snapshot;
}

ScanSnapshot NetworkMonitor::loadLastSnapshot(const QString& target) const {
    QString cleanTarget = target;
    cleanTarget.replace("/", "_");
    cleanTarget.replace(" ", "_");
    QString targetDir = snapshotDirectory + "/" + cleanTarget;
    QDir dir(targetDir);
    
    if (!dir.exists()) {
        return ScanSnapshot();
    }
    
    QStringList filters;
    filters << "snapshot_*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);
    
    if (files.isEmpty()) {
        return ScanSnapshot();
    }
    
    // Return most recent snapshot
    return loadSnapshot(files.last().absoluteFilePath());
}

QString NetworkMonitor::getSnapshotPath(const QDateTime& timestamp) const {
    QString cleanTarget = currentTarget;
    cleanTarget.replace("/", "_");
    cleanTarget.replace(" ", "_");
    QString targetDir = snapshotDirectory + "/" + cleanTarget;
    QString fileName = QString("snapshot_%1.json").arg(timestamp.toString("yyyyMMdd_HHmmss"));
    return targetDir + "/" + fileName;
}

QString NetworkMonitor::makeDeviceKey(const QString& ip, int port) const {
    return QString("%1:%2").arg(ip).arg(port);
}

QString NetworkMonitor::makeDeviceKey(const DeviceSnapshot& device) const {
    return makeDeviceKey(device.ip, device.port);
}

bool NetworkMonitor::devicesEqual(const DeviceSnapshot& d1, const DeviceSnapshot& d2) const {
    return d1.ip == d2.ip &&
           d1.port == d2.port &&
           d1.deviceType == d2.deviceType &&
           d1.manufacturer == d2.manufacturer &&
           d1.model == d2.model &&
           d1.firmware == d2.firmware;
}

MonitorDiff NetworkMonitor::compareSnapshots(const ScanSnapshot& oldSnapshot, const ScanSnapshot& newSnapshot) const {
    MonitorDiff diff;
    diff.totalNew = 0;
    diff.totalRemoved = 0;
    diff.totalChanged = 0;
    
    QSet<QString> oldKeys;
    QSet<QString> newKeys;
    
    // Collect keys
    for (auto it = oldSnapshot.devices.constBegin(); it != oldSnapshot.devices.constEnd(); ++it) {
        oldKeys.insert(it.key());
    }
    for (auto it = newSnapshot.devices.constBegin(); it != newSnapshot.devices.constEnd(); ++it) {
        newKeys.insert(it.key());
    }
    
    // Find new devices
    QSet<QString> newDeviceKeys = newKeys - oldKeys;
    for (const QString& key : newDeviceKeys) {
        diff.newDevices.append(newSnapshot.devices[key]);
        diff.totalNew++;
    }
    
    // Find removed devices
    QSet<QString> removedDeviceKeys = oldKeys - newKeys;
    for (const QString& key : removedDeviceKeys) {
        diff.removedDevices.append(oldSnapshot.devices[key]);
        diff.totalRemoved++;
    }
    
    // Find changed devices
    QSet<QString> commonKeys = oldKeys & newKeys;
    for (const QString& key : commonKeys) {
        const DeviceSnapshot& oldDev = oldSnapshot.devices[key];
        const DeviceSnapshot& newDev = newSnapshot.devices[key];
        
        if (!devicesEqual(oldDev, newDev)) {
            diff.changedDevices.append(qMakePair(oldDev, newDev));
            diff.totalChanged++;
        }
    }
    
    return diff;
}

MonitorDiff NetworkMonitor::getDiff() const {
    return compareSnapshots(lastSnapshot, currentSnapshot);
}

MonitorDiff NetworkMonitor::compareWithLast() const {
    if (lastSnapshot.devices.isEmpty() || currentSnapshot.devices.isEmpty()) {
        return MonitorDiff();
    }
    return compareSnapshots(lastSnapshot, currentSnapshot);
}

QString NetworkMonitor::formatDiff(const MonitorDiff& diff) const {
    QString output;
    QTextStream stream(&output);
    
    stream << Qt::endl;
    stream << "=== Network Changes ===" << Qt::endl;
    stream << "New devices: " << diff.totalNew << Qt::endl;
    stream << "Removed devices: " << diff.totalRemoved << Qt::endl;
    stream << "Changed devices: " << diff.totalChanged << Qt::endl;
    stream << Qt::endl;
    
    if (!diff.newDevices.isEmpty()) {
        stream << "--- New Devices ---" << Qt::endl;
        for (const DeviceSnapshot& dev : diff.newDevices) {
            stream << "  + " << dev.ip << ":" << dev.port;
            if (!dev.manufacturer.isEmpty()) {
                stream << " (" << dev.manufacturer;
                if (!dev.model.isEmpty()) {
                    stream << " " << dev.model;
                }
                stream << ")";
            }
            if (!dev.deviceType.isEmpty()) {
                stream << " [" << dev.deviceType << "]";
            }
            stream << Qt::endl;
        }
        stream << Qt::endl;
    }
    
    if (!diff.removedDevices.isEmpty()) {
        stream << "--- Removed Devices ---" << Qt::endl;
        for (const DeviceSnapshot& dev : diff.removedDevices) {
            stream << "  - " << dev.ip << ":" << dev.port;
            if (!dev.manufacturer.isEmpty()) {
                stream << " (" << dev.manufacturer;
                if (!dev.model.isEmpty()) {
                    stream << " " << dev.model;
                }
                stream << ")";
            }
            stream << Qt::endl;
        }
        stream << Qt::endl;
    }
    
    if (!diff.changedDevices.isEmpty()) {
        stream << "--- Changed Devices ---" << Qt::endl;
        for (const auto& pair : diff.changedDevices) {
            const DeviceSnapshot& oldDev = pair.first;
            const DeviceSnapshot& newDev = pair.second;
            stream << "  ~ " << newDev.ip << ":" << newDev.port << Qt::endl;
            
            if (oldDev.manufacturer != newDev.manufacturer ||
                oldDev.model != newDev.model ||
                oldDev.firmware != newDev.firmware) {
                stream << "    Old: " << oldDev.manufacturer << " " << oldDev.model;
                if (!oldDev.firmware.isEmpty()) {
                    stream << " (" << oldDev.firmware << ")";
                }
                stream << Qt::endl;
                stream << "    New: " << newDev.manufacturer << " " << newDev.model;
                if (!newDev.firmware.isEmpty()) {
                    stream << " (" << newDev.firmware << ")";
                }
                stream << Qt::endl;
            }
        }
        stream << Qt::endl;
    }
    
    return output;
}

void NetworkMonitor::printDiff(const MonitorDiff& diff) const {
    QTextStream out(stdout);
    out << formatDiff(diff);
}

void NetworkMonitor::checkAndAlert(const MonitorDiff& diff) {
    if (!alertOnNewDevice) {
        return;
    }
    
    if (diff.totalNew > 0) {
        for (const DeviceSnapshot& dev : diff.newDevices) {
            QString key = makeDeviceKey(dev);
            if (!notifiedDevices.contains(key)) {
                QTextStream out(stdout);
                const char* ANSI_RED = "\033[31m";
                const char* ANSI_BOLD = "\033[1m";
                const char* ANSI_RESET = "\033[0m";
                
                out << Qt::endl;
                out << ANSI_BOLD << ANSI_RED << "[ALERT] NEW DEVICE DETECTED!" << ANSI_RESET << Qt::endl;
                out << "  IP: " << dev.ip << ":" << dev.port << Qt::endl;
                if (!dev.manufacturer.isEmpty()) {
                    out << "  Manufacturer: " << dev.manufacturer << Qt::endl;
                }
                if (!dev.model.isEmpty()) {
                    out << "  Model: " << dev.model << Qt::endl;
                }
                if (!dev.deviceType.isEmpty()) {
                    out << "  Type: " << dev.deviceType << Qt::endl;
                }
                if (!dev.firmware.isEmpty()) {
                    out << "  Firmware: " << dev.firmware << Qt::endl;
                }
                out << "  First seen: " << dev.firstSeen.toString(Qt::ISODate) << Qt::endl;
                out << Qt::endl;
                
                notifiedDevices.insert(key);
            }
        }
    }
}

int NetworkMonitor::getTotalMonitoredDevices() const {
    return currentSnapshot.totalDevices;
}

QDateTime NetworkMonitor::getLastScanTime() const {
    return currentSnapshot.timestamp;
}

int NetworkMonitor::getScanCount() const {
    // This would require tracking scan count separately
    // For now, estimate from snapshot count
    QString cleanTarget = currentTarget;
    cleanTarget.replace("/", "_");
    cleanTarget.replace(" ", "_");
    QString targetDir = snapshotDirectory + "/" + cleanTarget;
    QDir dir(targetDir);
    if (dir.exists()) {
        QStringList filters;
        filters << "snapshot_*.json";
        return dir.entryList(filters, QDir::Files).size();
    }
    return 0;
}

QStringList NetworkMonitor::getSnapshotHistory(const QString& target) const {
    QString cleanTarget = target;
    cleanTarget.replace("/", "_");
    cleanTarget.replace(" ", "_");
    QString targetDir = snapshotDirectory + "/" + cleanTarget;
    QDir dir(targetDir);
    
    if (!dir.exists()) {
        return QStringList();
    }
    
    QStringList filters;
    filters << "snapshot_*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);
    
    QStringList result;
    for (const QFileInfo& file : files) {
        result.append(file.absoluteFilePath());
    }
    
    return result;
}

MonitorDiff NetworkMonitor::compareSnapshotsAt(int index1, int index2) const {
    QStringList history = getSnapshotHistory(currentTarget);
    
    if (index1 < 0 || index1 >= history.size() ||
        index2 < 0 || index2 >= history.size()) {
        return MonitorDiff();
    }
    
    ScanSnapshot snap1 = loadSnapshot(history[index1]);
    ScanSnapshot snap2 = loadSnapshot(history[index2]);
    
    return compareSnapshots(snap1, snap2);
}

void NetworkMonitor::createSnapshotFromExporter(const ResultExporter& exporter, const QString& target, const QString& mode) {
    std::vector<ScanResult> results = exporter.getResults();
    QMap<QString, DeviceSnapshot> devices;
    
    QDateTime now = QDateTime::currentDateTime();
    
    for (const ScanResult& result : results) {
        DeviceSnapshot dev;
        dev.ip = result.ip;
        dev.port = result.port;
        dev.deviceType = result.type;
        dev.lastSeen = now;
        dev.scanCount = 1;
        
        // Try to identify device from result data
        if (!result.data.isEmpty()) {
            DeviceInfo info = DeviceIdentifier::identify("", result.data);
            if (!info.manufacturer.isEmpty()) {
                dev.manufacturer = info.manufacturer;
                dev.model = info.model;
                dev.firmware = info.firmware;
                if (dev.deviceType.isEmpty()) {
                    dev.deviceType = info.deviceType;
                }
            }
        }
        
        QString key = makeDeviceKey(dev.ip, dev.port);
        
        if (devices.contains(key)) {
            // Update existing
            DeviceSnapshot& existing = devices[key];
            existing.lastSeen = now;
            existing.scanCount++;
            // Merge metadata
            if (!dev.manufacturer.isEmpty()) {
                existing.manufacturer = dev.manufacturer;
            }
            if (!dev.model.isEmpty()) {
                existing.model = dev.model;
            }
            if (!dev.firmware.isEmpty()) {
                existing.firmware = dev.firmware;
            }
        } else {
            dev.firstSeen = now;
            devices[key] = dev;
        }
    }
    
    createSnapshot(devices, target, mode);
}

