#include <ServiceVersionDetector.h>
#include <Connector.h>
#include <QRegularExpression>
#include <QTextStream>

QMap<int, QStringList> ServiceVersionDetector::portToServiceMap;
QMap<QString, QRegularExpression> ServiceVersionDetector::versionPatterns;

void ServiceVersionDetector::initializePatterns() {
    if (!versionPatterns.isEmpty()) return;
    
    // HTTP Server patterns
    versionPatterns["HTTP"] = QRegularExpression(R"(Server:\s*([^\r\n]+))", QRegularExpression::CaseInsensitiveOption);
    versionPatterns["Apache"] = QRegularExpression(R"(Apache[\/\s]+([\d\.]+))", QRegularExpression::CaseInsensitiveOption);
    versionPatterns["Nginx"] = QRegularExpression(R"(nginx[\/\s]+([\d\.]+))", QRegularExpression::CaseInsensitiveOption);
    versionPatterns["IIS"] = QRegularExpression(R"((?:Microsoft-IIS|IIS)[\/\s]+([\d\.]+))", QRegularExpression::CaseInsensitiveOption);
    
    // SSH patterns
    versionPatterns["SSH"] = QRegularExpression(R"(SSH-[\d\.]+-([^\s]+))", QRegularExpression::CaseInsensitiveOption);
    
    // FTP patterns
    versionPatterns["FTP"] = QRegularExpression(R"((?:220|Welcome)[\s]+([^\r\n]+))", QRegularExpression::CaseInsensitiveOption);
    
    // RTSP patterns
    versionPatterns["RTSP"] = QRegularExpression(R"(RTSP/([\d\.]+))", QRegularExpression::CaseInsensitiveOption);
    
    // Port to service mapping
    portToServiceMap[22] = QStringList() << "SSH";
    portToServiceMap[21] = QStringList() << "FTP";
    portToServiceMap[80] = QStringList() << "HTTP";
    portToServiceMap[443] = QStringList() << "HTTPS" << "HTTP";
    portToServiceMap[554] = QStringList() << "RTSP";
    portToServiceMap[8554] = QStringList() << "RTSP";
    portToServiceMap[8080] = QStringList() << "HTTP";
    portToServiceMap[3389] = QStringList() << "RDP";
    portToServiceMap[23] = QStringList() << "Telnet";
}

ServiceVersionDetector::ServiceVersionDetector() {
    initializePatterns();
}

ServiceVersion ServiceVersionDetector::detectFromHTTPHeaders(const QString& ip, int port) {
    ServiceVersion result;
    result.serviceName = "HTTP";
    result.confidence = 0;
    
    std::string buffer;
    QString url;
    if (port == 443) {
        url = QString("https://%1:%2/").arg(ip).arg(port);
    } else {
        url = QString("http://%1:%2/").arg(ip).arg(port);
    }
    
    Connector conn;
    int httpCode = conn.nConnect(url.toLocal8Bit().data(), port, &buffer);
    
    if (httpCode > 0 && buffer.size() > 0) {
        QString response = QString::fromStdString(buffer);
        
        // Extract Server header
        QRegularExpression serverRegex(R"(Server:\s*([^\r\n]+))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = serverRegex.match(response);
        
        if (match.hasMatch()) {
            QString serverHeader = match.captured(1);
            result.fullVersion = serverHeader;
            
            // Try to parse specific server types
            if (serverHeader.contains("Apache", Qt::CaseInsensitive)) {
                result.serviceName = "Apache";
                QRegularExpressionMatch apacheMatch = versionPatterns["Apache"].match(serverHeader);
                if (apacheMatch.hasMatch()) {
                    result.version = apacheMatch.captured(1);
                    result.confidence = 90;
                }
            } else if (serverHeader.contains("nginx", Qt::CaseInsensitive)) {
                result.serviceName = "nginx";
                QRegularExpressionMatch nginxMatch = versionPatterns["Nginx"].match(serverHeader);
                if (nginxMatch.hasMatch()) {
                    result.version = nginxMatch.captured(1);
                    result.confidence = 90;
                }
            } else if (serverHeader.contains("IIS", Qt::CaseInsensitive) || 
                      serverHeader.contains("Microsoft-HTTPAPI", Qt::CaseInsensitive)) {
                result.serviceName = "IIS";
                QRegularExpressionMatch iisMatch = versionPatterns["IIS"].match(serverHeader);
                if (iisMatch.hasMatch()) {
                    result.version = iisMatch.captured(1);
                    result.confidence = 90;
                }
            } else {
                result.version = serverHeader.split("/").last().trimmed();
                result.confidence = 70;
            }
        }
        
        result.detectionMethod = "header";
    }
    
    return result;
}

ServiceVersion ServiceVersionDetector::detectFromBanner(const QString& ip, int port) {
    ServiceVersion result;
    result.confidence = 0;
    
    // This would connect to the port and read banner
    // Implementation depends on protocol (SSH, FTP, Telnet, etc.)
    
    return result;
}

ServiceVersion ServiceVersionDetector::detectSSHVersion(const QString& ip, int port) {
    ServiceVersion result;
    result.serviceName = "SSH";
    result.confidence = 0;
    
    std::string buffer;
    Connector conn;
    // SSH detection would typically use raw socket connection
    // For now, return empty result
    result.detectionMethod = "banner";
    
    return result;
}

ServiceVersion ServiceVersionDetector::detectHTTPVersion(const QString& ip, int port) {
    return detectFromHTTPHeaders(ip, port);
}

ServiceVersion ServiceVersionDetector::detectFTPVersion(const QString& ip, int port) {
    ServiceVersion result;
    result.serviceName = "FTP";
    result.confidence = 0;
    
    std::string buffer;
    QString url = QString("ftp://%1:%2/").arg(ip).arg(port);
    Connector conn;
    int res = conn.nConnect(url.toLocal8Bit().data(), port, &buffer);
    
    if (res > 0 && buffer.size() > 0) {
        QString response = QString::fromStdString(buffer);
        QRegularExpressionMatch match = versionPatterns["FTP"].match(response);
        
        if (match.hasMatch()) {
            result.fullVersion = match.captured(1);
            result.version = extractVersion(result.fullVersion, versionPatterns["FTP"]);
            result.confidence = 80;
            result.detectionMethod = "banner";
        }
    }
    
    return result;
}

ServiceVersion ServiceVersionDetector::detectTelnetVersion(const QString& ip, int port) {
    ServiceVersion result;
    result.serviceName = "Telnet";
    result.confidence = 50;
    result.detectionMethod = "banner";
    return result;
}

ServiceVersion ServiceVersionDetector::detectRTSPVersion(const QString& ip, int port) {
    ServiceVersion result;
    result.serviceName = "RTSP";
    result.confidence = 0;
    
    std::string buffer;
    QString url = QString("rtsp://%1:%2/").arg(ip).arg(port);
    Connector conn;
    int res = conn.nConnect(url.toLocal8Bit().data(), port, &buffer);
    
    if (res > 0 && buffer.size() > 0) {
        QString response = QString::fromStdString(buffer);
        QRegularExpressionMatch match = versionPatterns["RTSP"].match(response);
        
        if (match.hasMatch()) {
            result.version = match.captured(1);
            result.confidence = 80;
            result.detectionMethod = "banner";
        }
    }
    
    return result;
}

ServiceVersion ServiceVersionDetector::detectONVIFVersion(const QString& ip, int port) {
    ServiceVersion result;
    result.serviceName = "ONVIF";
    result.confidence = 0;
    
    std::string buffer;
    QString url = QString("http://%1:%2/onvif/device_service").arg(ip).arg(port);
    Connector conn;
    int res = conn.nConnect(url.toLocal8Bit().data(), port, &buffer);
    
    if (res > 0 && buffer.size() > 0) {
        QString response = QString::fromStdString(buffer);
        QRegularExpression versionRegex(R"(version[=:]\s*([\d\.]+))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = versionRegex.match(response);
        
        if (match.hasMatch()) {
            result.version = match.captured(1);
            result.confidence = 85;
            result.detectionMethod = "path";
        }
    }
    
    return result;
}

QString ServiceVersionDetector::extractVersion(const QString& text, const QRegularExpression& pattern) {
    QRegularExpressionMatch match = pattern.match(text);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return "";
}

int ServiceVersionDetector::calculateConfidence(const QString& version, const QString& method) {
    if (version.isEmpty()) return 0;
    
    int confidence = 50;
    
    if (method == "header") confidence += 20;
    else if (method == "banner") confidence += 15;
    else if (method == "path") confidence += 10;
    
    // Increase confidence if version looks valid (contains numbers)
    if (version.contains(QRegularExpression(R"(\d)"))) {
        confidence += 20;
    }
    
    return qMin(confidence, 100);
}

ServiceVersion ServiceVersionDetector::detectFromPath(const QString& ip, int port, const QString& path) {
    ServiceVersion result;
    result.confidence = 0;
    
    QString url;
    if (port == 443) {
        url = QString("https://%1:%2%3").arg(ip).arg(port).arg(path);
    } else {
        url = QString("http://%1:%2%3").arg(ip).arg(port).arg(path);
    }
    
    std::string buffer;
    Connector conn;
    conn.nConnect(url.toLocal8Bit().data(), port, &buffer);
    
    QString response = QString::fromStdString(buffer);
    if (!response.isEmpty()) {
        result.detectionMethod = "path";
        result.confidence = 60;
    }
    
    return result;
}

ServiceVersion ServiceVersionDetector::detectFromResponseBody(const QString& ip, int port) {
    return detectFromHTTPHeaders(ip, port);
}

VersionScanResult ServiceVersionDetector::detectVersions(const QString& ip, int port) {
    VersionScanResult result;
    result.ip = ip;
    result.port = port;
    
    initializePatterns();
    
    // Determine service type based on port
    QStringList possibleServices = portToServiceMap.value(port);
    if (possibleServices.isEmpty()) {
        possibleServices = QStringList() << "HTTP";
    }
    
    // Try different detection methods
    if (port == 80 || port == 443 || port == 8080) {
        ServiceVersion httpVer = detectHTTPVersion(ip, port);
        if (httpVer.confidence > 0) {
            result.detectedServices.append(httpVer);
            result.allVersions[httpVer.serviceName] = httpVer.version;
            if (result.primaryService.isEmpty()) {
                result.primaryService = httpVer.serviceName;
                result.primaryVersion = httpVer.version;
            }
        }
    } else if (port == 21) {
        ServiceVersion ftpVer = detectFTPVersion(ip, port);
        if (ftpVer.confidence > 0) {
            result.detectedServices.append(ftpVer);
            result.allVersions[ftpVer.serviceName] = ftpVer.version;
            result.primaryService = ftpVer.serviceName;
            result.primaryVersion = ftpVer.version;
        }
    } else if (port == 554 || port == 8554) {
        ServiceVersion rtspVer = detectRTSPVersion(ip, port);
        if (rtspVer.confidence > 0) {
            result.detectedServices.append(rtspVer);
            result.allVersions[rtspVer.serviceName] = rtspVer.version;
            result.primaryService = rtspVer.serviceName;
            result.primaryVersion = rtspVer.version;
        }
    }
    
    // Try ONVIF detection for camera ports
    if (port == 80 || port == 8080) {
        ServiceVersion onvifVer = detectONVIFVersion(ip, port);
        if (onvifVer.confidence > 0) {
            result.detectedServices.append(onvifVer);
            result.allVersions[onvifVer.serviceName] = onvifVer.version;
        }
    }
    
    return result;
}

QMap<int, VersionScanResult> ServiceVersionDetector::detectVersionsForPorts(
    const QString& ip, const QList<int>& ports) {
    
    QMap<int, VersionScanResult> results;
    
    for (int port : ports) {
        results[port] = detectVersions(ip, port);
    }
    
    return results;
}

QString ServiceVersionDetector::getServiceVersion(const QString& ip, int port, const QString& serviceName) {
    VersionScanResult result = detectVersions(ip, port);
    
    for (const ServiceVersion& sv : result.detectedServices) {
        if (sv.serviceName == serviceName) {
            return sv.version;
        }
    }
    
    return "";
}

