#include <FingerprintScanner.h>
#include <Connector.h>
#include <externData.h>
#include <Utils.h>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <string>
#include <cstring>
#include <cctype>
#include <algorithm>

// ONVIF device detection
bool FingerprintScanner::checkONVIFEndpoint(const char* ip, int port, const char* path) {
    std::string buffer;
    char url[512];
    sprintf(url, "http://%s:%d%s", ip, port, path);
    
    Connector con;
    int res = con.nConnect(url, port, &buffer);
    
    if (res > 0) {
        // Check for ONVIF-specific responses
        std::string lowerBuffer = buffer;
        for (size_t i = 0; i < lowerBuffer.length(); i++) {
            lowerBuffer[i] = std::tolower(lowerBuffer[i]);
        }
        
        // ONVIF indicators
        if (Utils::ustrstr(&lowerBuffer, "onvif") != -1 ||
            Utils::ustrstr(&lowerBuffer, "device_service") != -1 ||
            Utils::ustrstr(&lowerBuffer, "xmlns:tds") != -1 ||
            Utils::ustrstr(&lowerBuffer, "getcapabilities") != -1) {
            return true;
        }
    }
    return false;
}

QString FingerprintScanner::parseONVIFDeviceInfo(const std::string& response) {
    QJsonObject info;
    
    // Try to extract manufacturer
    if (Utils::ustrstr(&response, "manufacturer") != -1) {
        std::size_t pos = response.find("Manufacturer");
        if (pos != std::string::npos) {
            std::size_t start = response.find(">", pos);
            std::size_t end = response.find("<", start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string manufacturer = response.substr(start + 1, end - start - 1);
                info["manufacturer"] = QString::fromStdString(manufacturer);
            }
        }
    }
    
    // Try to extract model
    if (Utils::ustrstr(&response, "model") != -1) {
        std::size_t pos = response.find("Model");
        if (pos != std::string::npos) {
            std::size_t start = response.find(">", pos);
            std::size_t end = response.find("<", start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string model = response.substr(start + 1, end - start - 1);
                info["model"] = QString::fromStdString(model);
            }
        }
    }
    
    // Try to extract firmware version
    if (Utils::ustrstr(&response, "firmwareversion") != -1 || Utils::ustrstr(&response, "firmware") != -1) {
        std::size_t pos = response.find("FirmwareVersion");
        if (pos == std::string::npos) pos = response.find("firmware");
        if (pos != std::string::npos) {
            std::size_t start = response.find(">", pos);
            std::size_t end = response.find("<", start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string version = response.substr(start + 1, end - start - 1);
                info["firmware"] = QString::fromStdString(version);
            }
        }
    }
    
    QJsonDocument doc(info);
    return doc.toJson(QJsonDocument::Compact);
}

FingerprintResult FingerprintScanner::checkONVIF(const char* ip, int port) {
    FingerprintResult result;
    result.detected = false;
    result.confidence = 0;
    
    // Common ONVIF endpoints
    const char* onvifPaths[] = {
        "/onvif/device_service",
        "/onvif/device_service.wsdl",
        "/onvif/device_service?wsdl",
        "/onvif/services",
        "/onvif/wsdl/devicemgmt.wsdl",
        "/wsdl/devicemgmt.wsdl",
        "/onvif/device_service/getCapabilities"
    };
    
    for (size_t i = 0; i < sizeof(onvifPaths) / sizeof(onvifPaths[0]); i++) {
        if (checkONVIFEndpoint(ip, port, onvifPaths[i])) {
            result.detected = true;
            result.deviceType = "onvif";
            result.endpoint = QString("/onvif/device_service");
            
            // Get full response to parse device info
            std::string buffer;
            char url[512];
            sprintf(url, "http://%s:%d%s", ip, port, onvifPaths[i]);
            Connector con;
            int res = con.nConnect(url, port, &buffer);
            
            if (res > 0) {
                result.details = parseONVIFDeviceInfo(buffer);
                QJsonDocument doc = QJsonDocument::fromJson(result.details.toUtf8());
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains("manufacturer")) {
                        result.manufacturer = obj["manufacturer"].toString();
                    }
                    if (obj.contains("model")) {
                        result.model = obj["model"].toString();
                    }
                    if (obj.contains("firmware")) {
                        result.version = obj["firmware"].toString();
                    }
                }
            }
            
            result.confidence = 90;
            break;
        }
    }
    
    return result;
}

// RTSP stream URLs to check
QStringList FingerprintScanner::getRTSPStreamURLs() {
    return QStringList() 
        << "/live.sdp"
        << "/stream"
        << "/stream1"
        << "/stream2"
        << "/ch1.sdp"
        << "/ch2.sdp"
        << "/video.sdp"
        << "/h264"
        << "/h264/ch1/main/av_stream"
        << "/h264/ch1/sub/av_stream"
        << "/rtsp"
        << "/rtsp/stream"
        << "/live"
        << "/main"
        << "/sub"
        << "/mpeg4"
        << "/mpeg4/ch1/main/av_stream"
        << "/cam/realmonitor"
        << "/cam/realmonitor?channel=1&subtype=0"
        << "/11"
        << "/12"
        << "/Streaming/Channels/1"
        << "/Streaming/Channels/101";
}

FingerprintResult FingerprintScanner::checkRTSPStreams(const char* ip, int port) {
    FingerprintResult result;
    result.detected = false;
    result.confidence = 0;
    
    QStringList streamUrls = getRTSPStreamURLs();
    QStringList detectedStreams;
    
    for (const QString& path : streamUrls) {
        // Try RTSP DESCRIBE on the stream URL
        std::string buffer;
        char rtspUrl[512];
        sprintf(rtspUrl, "rtsp://%s:%d%s", ip, port, path.toLocal8Bit().data());
        
        Connector con;
        int rtspRes = con.checkIsDigestRTSP(rtspUrl, &buffer);
        
        // Also try HTTP GET on RTSP port (some devices respond)
        char httpUrl[512];
        sprintf(httpUrl, "http://%s:%d%s", ip, port, path.toLocal8Bit().data());
        std::string httpBuffer;
        int httpRes = con.nConnect(httpUrl, port, &httpBuffer);
        
        // Check if we got a valid RTSP/SDP response
        bool detected = false;
        if (rtspRes == 2) {  // 200 OK from RTSP
            detected = true;
        } else if (rtspRes >= 0 && buffer.size() > 0) {
            // Check for RTSP response patterns
            std::string lowerBuffer = buffer;
            for (size_t i = 0; i < lowerBuffer.length() && i < 256; i++) {
                lowerBuffer[i] = std::tolower(lowerBuffer[i]);
            }
            if (Utils::ustrstr(&lowerBuffer, "rtsp") != -1 ||
                Utils::ustrstr(&lowerBuffer, "200 ok") != -1 ||
                Utils::ustrstr(&lowerBuffer, "public") != -1 ||
                Utils::ustrstr(&lowerBuffer, "describe") != -1) {
                detected = true;
            }
        } else if (httpRes > 0 && httpBuffer.size() > 0) {
            // Check HTTP response for SDP/RTSP indicators
            std::string lowerHttpBuffer = httpBuffer;
            for (size_t i = 0; i < lowerHttpBuffer.length() && i < 256; i++) {
                lowerHttpBuffer[i] = std::tolower(lowerHttpBuffer[i]);
            }
            if (Utils::ustrstr(&lowerHttpBuffer, "sdp") != -1 ||
                Utils::ustrstr(&lowerHttpBuffer, "rtsp") != -1 ||
                Utils::ustrstr(&lowerHttpBuffer, "application/sdp") != -1 ||
                Utils::ustrstr(&lowerHttpBuffer, "video/") != -1) {
                detected = true;
            }
        }
        
        if (detected) {
            detectedStreams.append(path);
            // Limit to first 5 streams to avoid excessive scanning
            if (detectedStreams.size() >= 5) break;
        }
    }
    
    if (!detectedStreams.isEmpty()) {
        result.detected = true;
        result.deviceType = "rtsp_stream";
        result.endpoint = detectedStreams.first();
        if (detectedStreams.size() > 1) {
            QStringList quotedStreams;
            for (const QString& stream : detectedStreams) {
                quotedStreams.append("\"" + stream + "\"");
            }
            result.details = QString("{\"streams\": [%1]}").arg(quotedStreams.join(", "));
        } else {
            result.details = "{\"streams\": [\"" + detectedStreams.first() + "\"]}";
        }
        result.confidence = 70 + (detectedStreams.size() * 5); // Higher confidence with more streams
        if (result.confidence > 95) result.confidence = 95;
    }
    
    return result;
}

// Manufacturer API endpoints
QMap<QString, QStringList> FingerprintScanner::getManufacturerEndpoints() {
    QMap<QString, QStringList> endpoints;
    
    // Hikvision
    endpoints["Hikvision"] = QStringList()
        << "/ISAPI/System/deviceInfo"
        << "/ISAPI/System/version"
        << "/PSIA/System/deviceInfo"
        << "/cgi-bin/viewer/video.jpg"
        << "/Streaming/channels/1/picture";
    
    // Dahua
    endpoints["Dahua"] = QStringList()
        << "/cgi-bin/magicBox.cgi?action=getDeviceType"
        << "/cgi-bin/global.cgi?action=getCurrentTime"
        << "/RPC2_Login"
        << "/web/deviceinfo"
        << "/cgi-bin/configManager.cgi?action=getConfig&name=SystemInfo";
    
    // Axis
    endpoints["Axis"] = QStringList()
        << "/axis-cgi/param.cgi?action=list"
        << "/axis-cgi/view/param.cgi"
        << "/axis-cgi/jpg/image.cgi"
        << "/axis2/axis2-web/AdministratorAction";
    
    // Panasonic
    endpoints["Panasonic"] = QStringList()
        << "/cgi-bin/nphCameraControl?Direction=Center"
        << "/SnapshotJPEG?Resolution=640x480"
        << "/CgiStart?page=Single&Language=0";
    
    // Sony
    endpoints["Sony"] = QStringList()
        << "/command/inquiry.cgi"
        << "/command/version.cgi"
        << "/sony/camera";
    
    // Bosch
    endpoints["Bosch"] = QStringList()
        << "/config"
        << "/media_service"
        << "/local/display.cgi";
    
    // Vivotek
    endpoints["Vivotek"] = QStringList()
        << "/cgi-bin/viewer/video.jpg"
        << "/cgi-bin/admin/getparam.cgi"
        << "/system/maintain";
    
    // Reolink
    endpoints["Reolink"] = QStringList()
        << "/api.cgi?cmd=GetDevInfo"
        << "/cgi-bin/api.cgi?cmd=GetDevInfo";
    
    // Uniview
    endpoints["Uniview"] = QStringList()
        << "/cgi-bin/main-cgi"
        << "/cgi-bin/main-cgi?json=";
    
    return endpoints;
}

FingerprintResult FingerprintScanner::checkManufacturerAPIs(const char* ip, int port) {
    FingerprintResult result;
    result.detected = false;
    result.confidence = 0;
    
    QMap<QString, QStringList> endpoints = getManufacturerEndpoints();
    
    for (auto it = endpoints.constBegin(); it != endpoints.constEnd(); ++it) {
        QString manufacturer = it.key();
        QStringList paths = it.value();
        
        for (const QString& path : paths) {
            std::string buffer;
            char url[512];
            sprintf(url, "http://%s:%d%s", ip, port, path.toLocal8Bit().data());
            
            Connector con;
            int res = con.nConnect(url, port, &buffer);
            
            if (res > 0) {
            // Check for manufacturer-specific patterns
            std::string lowerBuffer = buffer;
            size_t maxLen = (buffer.length() < 4096) ? buffer.length() : 4096; // Limit size
            for (size_t i = 0; i < maxLen; i++) {
                lowerBuffer[i] = std::tolower(lowerBuffer[i]);
            }
                
                bool matched = false;
                QString manufacturerLower = manufacturer.toLower();
                
                if (Utils::ustrstr(&lowerBuffer, manufacturerLower.toLocal8Bit().data()) != -1) {
                    matched = true;
                } else if (manufacturer == "Hikvision" && 
                          (Utils::ustrstr(&lowerBuffer, "deviceinfo") != -1 ||
                           Utils::ustrstr(&lowerBuffer, "isapi") != -1)) {
                    matched = true;
                } else if (manufacturer == "Dahua" && 
                          (Utils::ustrstr(&lowerBuffer, "dh-") != -1 ||
                           Utils::ustrstr(&lowerBuffer, "rpc2") != -1)) {
                    matched = true;
                } else if (manufacturer == "Axis" && 
                          Utils::ustrstr(&lowerBuffer, "axis") != -1) {
                    matched = true;
                }
                
                if (matched) {
                    result.detected = true;
                    result.deviceType = "api_camera";
                    result.manufacturer = manufacturer;
                    result.endpoint = path;
                    result.confidence = 85;
                    
                    // Try to extract model/version from response
                    QRegularExpression modelRegex(R"(([A-Z]{1,4}[-_]?\d+[A-Z]?[-_]?\d*[A-Z]?))", 
                                                  QRegularExpression::CaseInsensitiveOption);
                    QRegularExpressionMatch match = modelRegex.match(QString::fromStdString(buffer));
                    if (match.hasMatch()) {
                        result.model = match.captured(1);
                        result.confidence = 90;
                    }
                    
                    return result;
                }
            }
        }
    }
    
    return result;
}

// SNMP community strings
QStringList FingerprintScanner::getSNMPCommunityStrings() {
    return QStringList() 
        << "public"
        << "private"
        << "community"
        << "manager"
        << "public123"
        << "private123"
        << "read"
        << "write"
        << "ro"
        << "rw"
        << "admin"
        << "";
}

QString FingerprintScanner::querySNMP(const char* ip, const char* community, int port, const char* oid) {
    // Simple SNMP GET request implementation
    // SNMP v1 GET request structure
    unsigned char snmpRequest[512];
    int pos = 0;
    
    // SNMP Message (Sequence)
    snmpRequest[pos++] = 0x30; // SEQUENCE
    snmpRequest[pos++] = 0x00; // Length (will be filled later)
    
    // Version (INTEGER 0 for SNMPv1)
    snmpRequest[pos++] = 0x02;
    snmpRequest[pos++] = 0x01;
    snmpRequest[pos++] = 0x00;
    
    // Community (OCTET STRING)
    int communityLen = strlen(community);
    snmpRequest[pos++] = 0x04;
    snmpRequest[pos++] = communityLen;
    memcpy(&snmpRequest[pos], community, communityLen);
    pos += communityLen;
    
    // PDU (GET Request)
    snmpRequest[pos++] = 0xA0; // GetRequest
    snmpRequest[pos++] = 0x00; // Length
    
    // Request ID
    snmpRequest[pos++] = 0x02;
    snmpRequest[pos++] = 0x01;
    snmpRequest[pos++] = 0x01;
    
    // Error status
    snmpRequest[pos++] = 0x02;
    snmpRequest[pos++] = 0x01;
    snmpRequest[pos++] = 0x00;
    
    // Error index
    snmpRequest[pos++] = 0x02;
    snmpRequest[pos++] = 0x01;
    snmpRequest[pos++] = 0x00;
    
    // Variable bindings (simplified - would need full OID encoding)
    // For now, return empty as full SNMP implementation requires proper ASN.1 encoding
    
    // Note: Full SNMP implementation would require proper socket UDP connection
    // and ASN.1 BER encoding/decoding. This is a placeholder structure.
    
    return QString();
}

FingerprintResult FingerprintScanner::checkSNMP(const char* ip, int port) {
    FingerprintResult result;
    result.detected = false;
    result.confidence = 0;
    
    // SNMP requires UDP socket connection and proper ASN.1 encoding
    // This is a simplified check - full implementation would require net-snmp library
    
    // For now, we'll check if SNMP port is open via TCP connection test
    // Real SNMP is UDP, but some devices expose SNMP over HTTP
    QStringList snmpHttpEndpoints = QStringList()
        << "/snmp"
        << "/snmp/index.html"
        << "/cgi-bin/snmp"
        << "/snmp/device.cgi";
    
    for (const QString& path : snmpHttpEndpoints) {
        std::string buffer;
        char url[512];
        sprintf(url, "http://%s:%d%s", ip, port, path.toLocal8Bit().data());
        
        Connector con;
        int res = con.nConnect(url, port, &buffer);
        
        if (res > 0 && (Utils::ustrstr(&buffer, "snmp") != -1 ||
                       Utils::ustrstr(&buffer, "mib") != -1)) {
            result.detected = true;
            result.deviceType = "snmp_device";
            result.endpoint = path;
            result.confidence = 60;
            
            // Try community strings via UDP would go here
            // For now, mark as detected via HTTP
            
            return result;
        }
    }
    
    // Note: Full UDP SNMP scanning would require:
    // 1. Creating UDP socket
    // 2. Proper ASN.1 BER encoding
    // 3. Sending SNMP GET requests
    // 4. Parsing SNMP responses
    // This would be better implemented with net-snmp library or similar
    
    return result;
}

FingerprintResult FingerprintScanner::scanDevice(const char* ip, int port) {
    FingerprintResult result;
    result.detected = false;
    result.confidence = 0;
    
    // Try ONVIF first (common on port 80)
    if (port == 80 || port == 8080) {
        result = checkONVIF(ip, port);
        if (result.detected) {
            return result;
        }
    }
    
    // Try manufacturer APIs
    result = checkManufacturerAPIs(ip, port);
    if (result.detected) {
        return result;
    }
    
    // Try RTSP streams (common on port 554)
    if (port == 554 || port == 8554) {
        result = checkRTSPStreams(ip, port);
        if (result.detected) {
            return result;
        }
    }
    
    // Try SNMP (common on port 161)
    if (port == 161 || port == 162) {
        result = checkSNMP(ip, port);
        if (result.detected) {
            return result;
        }
    }
    
    return result;
}

QStringList FingerprintScanner::getDetectedEndpoints(const char* ip, int port) {
    QStringList endpoints;
    
    FingerprintResult result;
    
    // Check ONVIF
    result = checkONVIF(ip, port);
    if (result.detected) {
        endpoints.append("ONVIF: " + result.endpoint);
    }
    
    // Check RTSP
    result = checkRTSPStreams(ip, port);
    if (result.detected) {
        endpoints.append("RTSP: " + result.endpoint);
        if (!result.details.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(result.details.toUtf8());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("streams")) {
                    QJsonArray streams = obj["streams"].toArray();
                    for (const QJsonValue& stream : streams) {
                        endpoints.append("  └─ " + stream.toString());
                    }
                }
            }
        }
    }
    
    // Check manufacturer APIs
    result = checkManufacturerAPIs(ip, port);
    if (result.detected) {
        endpoints.append(result.manufacturer + " API: " + result.endpoint);
    }
    
    // Check SNMP
    result = checkSNMP(ip, port);
    if (result.detected) {
        endpoints.append("SNMP: " + result.endpoint);
    }
    
    return endpoints;
}

