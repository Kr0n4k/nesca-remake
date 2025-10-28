#include <DeepScanner.h>
#include <Connector.h>
#include <QRegularExpression>
#include <QTextStream>

QStringList DeepScanner::commonCameraPaths;
QStringList DeepScanner::commonDVRPaths;
QStringList DeepScanner::commonRouterPaths;
QStringList DeepScanner::commonServerPaths;

void DeepScanner::initializePaths() {
    if (!commonCameraPaths.isEmpty()) return;
    
    commonCameraPaths = QStringList()
        << "/" << "/index.html" << "/login.html" << "/web/login.html"
        << "/cgi-bin" << "/cgi-bin/viewer/video.jpg"
        << "/ISAPI" << "/ISAPI/System/deviceInfo"
        << "/onvif/device_service" << "/onvif/Media"
        << "/SDK/webLanguage" << "/RPC2_Login"
        << "/web/" << "/videostream.cgi" << "/snapshot.cgi"
        << "/Streaming/Channels" << "/liveview.html"
        << "/api" << "/api/system" << "/api/device";
    
    commonDVRPaths = QStringList()
        << "/" << "/login.htm" << "/login.jsp"
        << "/web" << "/web/login.jsp"
        << "/cgi-bin" << "/cgi-bin/login.cgi"
        << "/main.html" << "/index.html";
    
    commonRouterPaths = QStringList()
        << "/" << "/cgi-bin" << "/webfig" << "/winbox"
        << "/login.cgi" << "/status.cgi"
        << "/luci" << "/userRpm";
    
    commonServerPaths = QStringList()
        << "/" << "/index.html" << "/status"
        << "/api" << "/api/v1" << "/api/v2"
        << "/health" << "/version" << "/info";
}

DeepScanner::DeepScanner() {
    initializePaths();
}

QStringList DeepScanner::getPathsForDeviceType(const QString& deviceType) {
    initializePaths();
    
    if (deviceType == "camera" || deviceType.contains("camera", Qt::CaseInsensitive)) {
        return commonCameraPaths;
    } else if (deviceType == "dvr" || deviceType == "nvr") {
        return commonDVRPaths;
    } else if (deviceType == "router" || deviceType.contains("router", Qt::CaseInsensitive)) {
        return commonRouterPaths;
    } else {
        return commonServerPaths;
    }
}

bool DeepScanner::checkEndpoint(const QString& ip, int port, const QString& path, const QString& method) {
    QString url;
    if (port == 443) {
        url = QString("https://%1:%2%3").arg(ip).arg(port).arg(path);
    } else {
        url = QString("http://%1:%2%3").arg(ip).arg(port).arg(path);
    }
    
    std::string buffer;
    Connector conn;
    int result = conn.nConnect(url.toLocal8Bit().data(), port, &buffer);
    
    return (result >= 200 && result < 400) || (buffer.size() > 100);
}

QString DeepScanner::getResponseBody(const QString& ip, int port, const QString& path, const QString& method) {
    QString url;
    if (port == 443) {
        url = QString("https://%1:%2%3").arg(ip).arg(port).arg(path);
    } else {
        url = QString("http://%1:%2%3").arg(ip).arg(port).arg(path);
    }
    
    std::string buffer;
    Connector conn;
    conn.nConnect(url.toLocal8Bit().data(), port, &buffer);
    
    return QString::fromStdString(buffer);
}

QMap<QString, QString> DeepScanner::extractHeaders(const QString& response) {
    QMap<QString, QString> headers;
    QStringList lines = response.split("\r\n");
    
    for (const QString& line : lines) {
        if (line.isEmpty() || !line.contains(":")) continue;
        
        int colonPos = line.indexOf(":");
        if (colonPos > 0) {
            QString key = line.left(colonPos).trimmed();
            QString value = line.mid(colonPos + 1).trimmed();
            headers[key.toLower()] = value;
        }
    }
    
    return headers;
}

QStringList DeepScanner::extractLinks(const QString& html) {
    QStringList links;
    QRegularExpression regex(R"(href\s*=\s*["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
    
    QRegularExpressionMatchIterator i = regex.globalMatch(html);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString link = match.captured(1);
        if (!link.isEmpty() && !links.contains(link)) {
            links.append(link);
        }
    }
    
    return links;
}

QStringList DeepScanner::extractForms(const QString& html) {
    QStringList forms;
    QRegularExpression regex(R"(action\s*=\s*["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
    
    QRegularExpressionMatchIterator i = regex.globalMatch(html);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString action = match.captured(1);
        if (!action.isEmpty() && !forms.contains(action)) {
            forms.append(action);
        }
    }
    
    return forms;
}

DeepScanResult DeepScanner::scanDevice(const QString& ip, int port, const QString& deviceType, const QString& manufacturer) {
    DeepScanResult result;
    result.deviceIP = ip;
    result.devicePort = port;
    result.deviceType = deviceType;
    result.manufacturer = manufacturer;
    
    // Get paths for device type
    QStringList paths = getPathsForDeviceType(deviceType);
    
    // Scan common paths
    for (const QString& path : paths) {
        if (checkEndpoint(ip, port, path)) {
            result.discoveredPaths.append(path);
            
            DeviceEndpoint endpoint;
            endpoint.path = path;
            endpoint.method = "GET";
            endpoint.description = QString("Discovered endpoint: %1").arg(path);
            result.discoveredEndpoints.append(endpoint);
        }
    }
    
    // Get main page and extract info
    QString mainPage = getResponseBody(ip, port, "/");
    if (!mainPage.isEmpty()) {
        result.headers = extractHeaders(mainPage);
        
        // Extract links and forms
        QStringList links = extractLinks(mainPage);
        QStringList forms = extractForms(mainPage);
        
        result.discoveredPaths.append(links);
        
        for (const QString& form : forms) {
            DeviceEndpoint endpoint;
            endpoint.path = form;
            endpoint.method = "POST";
            endpoint.description = "Form action";
            result.discoveredEndpoints.append(endpoint);
        }
    }
    
    // Detect firmware version
    result.firmwareVersion = detectFirmwareVersion(ip, port, deviceType);
    
    return result;
}

QStringList DeepScanner::scanAlternatePorts(const QString& ip, const QStringList& ports) {
    QStringList openPorts;
    
    for (const QString& portStr : ports) {
        bool ok;
        int port = portStr.toInt(&ok);
        if (!ok) continue;
        
        if (checkEndpoint(ip, port, "/")) {
            openPorts.append(portStr);
        }
    }
    
    return openPorts;
}

QStringList DeepScanner::scanCommonPaths(const QString& ip, int port, const QStringList& paths) {
    QStringList foundPaths;
    
    for (const QString& path : paths) {
        if (checkEndpoint(ip, port, path)) {
            foundPaths.append(path);
        }
    }
    
    return foundPaths;
}

QString DeepScanner::detectFirmwareVersion(const QString& ip, int port, const QString& deviceType) {
    QString version;
    
    // Try device info endpoints
    QStringList infoPaths = QStringList()
        << "/ISAPI/System/deviceInfo"
        << "/cgi-bin/magicBox.cgi?action=getDeviceType"
        << "/RPC2_Login"
        << "/api/device/info"
        << "/web/device";
    
    for (const QString& path : infoPaths) {
        QString response = getResponseBody(ip, port, path);
        if (!response.isEmpty()) {
            // Try to extract version from response
            QRegularExpression versionRegex(R"((?:version|firmware|ver)[\s:=]+([\d\.]+))", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = versionRegex.match(response);
            if (match.hasMatch()) {
                version = match.captured(1);
                break;
            }
        }
    }
    
    return version;
}

QString DeepScanner::detectHardwareVersion(const QString& ip, int port, const QString& manufacturer) {
    QString version = detectFirmwareVersion(ip, port, "camera");
    // Could add manufacturer-specific logic here
    return version;
}

