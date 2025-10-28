#include <DeviceIdentifier.h>
#include <QJsonObject>
#include <QJsonDocument>

QMap<QString, QString> DeviceIdentifier::manufacturerPatterns;
QMap<QString, QString> DeviceIdentifier::modelPatterns;

void DeviceIdentifier::initializePatterns() {
    if (!manufacturerPatterns.isEmpty()) return; // Already initialized
    
    // Manufacturer patterns (brand -> regex pattern)
    manufacturerPatterns["Hikvision"] = R"(hikvision|hik-?vision|Hikvision|HIKVISION)";
    manufacturerPatterns["Dahua"] = R"(dahua|Dahua|DAHUA|dahua.*technology)";
    manufacturerPatterns["Axis"] = R"(axis|Axis|AXIS|axis.*communications)";
    manufacturerPatterns["Panasonic"] = R"(panasonic|Panasonic|PANASONIC)";
    manufacturerPatterns["Sony"] = R"(sony|Sony|SONY)";
    manufacturerPatterns["Samsung"] = R"(samsung|Samsung|SAMSUNG)";
    manufacturerPatterns["Bosch"] = R"(bosch|Bosch|BOSCH|bosch.*security)";
    manufacturerPatterns["Pelco"] = R"(pelco|Pelco|PELCO)";
    manufacturerPatterns["Vivotek"] = R"(vivotek|Vivotek|VIVOTEK)";
    manufacturerPatterns["Foscam"] = R"(foscam|Foscam|FOSCAM)";
    manufacturerPatterns["Uniview"] = R"(uniview|Uniview|UNIVIEW)";
    manufacturerPatterns["Reolink"] = R"(reolink|Reolink|REOLINK)";
    manufacturerPatterns["TP-Link"] = R"(tp-?link|TP-?Link|TP-LINK)";
    
    // Model patterns (common model identifiers)
    modelPatterns["Hikvision"] = R"(DS-|IPC-|DVR-|NVR-|Hikvision)";
    modelPatterns["Dahua"] = R"(IPC-|DHI-|DH-|Dahua)";
    modelPatterns["Axis"] = R"(P\d+|M\d+|Q\d+|Axis)";
    modelPatterns["Panasonic"] = R"(WV-|BB-|Panasonic)";
    modelPatterns["Sony"] = R"(SNC-|IPELA|Sony)";
}

DeviceIdentifier::DeviceIdentifier() {
    initializePatterns();
}

DeviceInfo DeviceIdentifier::identify(const QString &headers, const QString &body) {
    DeviceInfo info;
    info.confidence = "low";
    
    QString combined = headers + " " + body;
    QString lowerCombined = combined.toLower();
    
    // Try to identify manufacturer
    for (auto it = manufacturerPatterns.constBegin(); it != manufacturerPatterns.constEnd(); ++it) {
        QRegularExpression regex(it.value(), QRegularExpression::CaseInsensitiveOption);
        if (regex.match(combined).hasMatch()) {
            info.manufacturer = it.key();
            info.confidence = "medium";
            break;
        }
    }
    
    // Try to extract model number for identified manufacturers
    if (!info.manufacturer.isEmpty() && modelPatterns.contains(info.manufacturer)) {
        QString pattern = modelPatterns[info.manufacturer];
        QRegularExpression modelRegex(R"(([A-Z]{1,4}[-_]?\d+[A-Z]?[-_]?\d*[A-Z]?)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator i = modelRegex.globalMatch(combined);
        if (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            info.model = match.captured(1);
            info.confidence = "high";
        }
    }
    
    // Try to extract firmware version
    QRegularExpression fwRegex(R"(firmware[:\s]+([\d\.]+)|version[:\s]+([\d\.]+)|v([\d\.]+))", 
                               QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch fwMatch = fwRegex.match(combined);
    if (fwMatch.hasMatch()) {
        info.firmware = fwMatch.captured(1);
        if (info.firmware.isEmpty()) {
            info.firmware = fwMatch.captured(2);
        }
        if (info.firmware.isEmpty()) {
            info.firmware = fwMatch.captured(3);
        }
    }
    
    // Determine device type
    QString lowerHeaders = headers.toLower();
    if (lowerHeaders.contains("ip camera") || lowerCombined.contains("ipcam") || 
        lowerCombined.contains("network camera")) {
        info.deviceType = "camera";
    } else if (lowerCombined.contains("dvr") && !lowerCombined.contains("ip")) {
        info.deviceType = "dvr";
    } else if (lowerCombined.contains("nvr")) {
        info.deviceType = "nvr";
    } else if (lowerCombined.contains("onvif") || lowerCombined.contains("device_service")) {
        info.deviceType = "onvif";
        info.confidence = "high";
    } else if (lowerCombined.contains("rtsp") || lowerCombined.contains(".sdp")) {
        info.deviceType = "rtsp_stream";
    } else if (!info.manufacturer.isEmpty()) {
        info.deviceType = "camera"; // Default for identified cameras
    }
    
    return info;
}

QString DeviceIdentifier::getManufacturer(const QString &headers, const QString &body) {
    DeviceInfo info = identify(headers, body);
    return info.manufacturer;
}

QString DeviceIdentifier::getModel(const QString &headers, const QString &body) {
    DeviceInfo info = identify(headers, body);
    return info.model;
}

bool DeviceIdentifier::isBrand(const QString &brand, const QString &headers, const QString &body) {
    QString combined = headers + " " + body;
    if (manufacturerPatterns.contains(brand)) {
        QRegularExpression regex(manufacturerPatterns[brand], QRegularExpression::CaseInsensitiveOption);
        return regex.match(combined).hasMatch();
    }
    return false;
}

QString DeviceIdentifier::getDeviceInfoJSON(const QString &headers, const QString &body) {
    DeviceInfo info = identify(headers, body);
    QJsonObject obj;
    
    if (!info.manufacturer.isEmpty()) {
        obj["manufacturer"] = info.manufacturer;
    }
    if (!info.model.isEmpty()) {
        obj["model"] = info.model;
    }
    if (!info.firmware.isEmpty()) {
        obj["firmware"] = info.firmware;
    }
    if (!info.deviceType.isEmpty()) {
        obj["device_type"] = info.deviceType;
    }
    if (!info.confidence.isEmpty()) {
        obj["confidence"] = info.confidence;
    }
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

