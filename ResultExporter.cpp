#include "ResultExporter.h"
#include "mainResources.h"
#include "Utils.h"
#include <QFileInfo>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

ResultExporter::ResultExporter() {
    outputDir = "./";
}

QString ResultExporter::extractIPFromHTML(const QString &html) {
    // Extract IP from HTML - format: [timestamp] IP:PORT
    QRegularExpression ipRegex(R"(\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b)");
    QRegularExpressionMatch match = ipRegex.match(html);
    if (match.hasMatch()) {
        return match.captured(0);
    }
    return "";
}

QString ResultExporter::extractPortFromHTML(const QString &html) {
    // Extract port - usually after IP with colon
    QRegularExpression portRegex(R"(:(\d{2,5})\b)");
    QRegularExpressionMatchIterator i = portRegex.globalMatch(html);
    QStringList ports;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        ports.append(match.captured(1));
    }
    // Return first valid port (usually after IP)
    if (!ports.isEmpty()) {
        bool ok;
        int port = ports.first().toInt(&ok);
        if (ok && port > 0 && port < 65536) {
            return ports.first();
        }
    }
    return "";
}

QString ResultExporter::extractTimestampFromHTML(const QString &html) {
    // Extract timestamp from [timestamp] format
    QRegularExpression timeRegex(R"(\[([^\]]+)\])");
    QRegularExpressionMatch match = timeRegex.match(html);
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

QString ResultExporter::extractDataFromHTML(const QString &html) {
    // Extract data after timestamp, clean HTML tags
    QString data = html;
    data.remove(QRegularExpression("<[^>]*>"));
    data.remove(QRegularExpression(R"(\[[^\]]+\]\s*)")); // Remove timestamp
    // Clean up IP:PORT prefix if present
    QRegularExpression ipPortRegex(R"(\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}:\d{2,5}\s*)");
    data.remove(ipPortRegex);
    return data.trimmed();
}

QString ResultExporter::extractAuthFromHTML(const QString &html) {
    // Try to extract login:password if present
    QRegularExpression authRegex(R"((?:login|user)[:=]\s*(\S+).*(?:pass|pwd)[:=]\s*(\S+))",
                                 QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = authRegex.match(html);
    if (match.hasMatch()) {
        return match.captured(1) + ":" + match.captured(2);
    }
    return "";
}

bool ResultExporter::parseHTMLFile(const QString &filepath, const QString &type) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // Split by <div id="ipd"> to get individual results
    QStringList entries = content.split("<div id=\"ipd\"");
    
    for (const QString &entry : entries) {
        if (entry.isEmpty() || entry.length() < 20) continue;
        
        QString fullEntry = "<div id=\"ipd\"" + entry;
        fullEntry = fullEntry.split("</div>").first() + "</div>";
        
        ScanResult result;
        result.type = type;
        result.ip = extractIPFromHTML(fullEntry);
        QString portStr = extractPortFromHTML(fullEntry);
        result.port = portStr.isEmpty() ? 0 : portStr.toInt();
        result.timestamp = extractTimestampFromHTML(fullEntry);
        result.data = extractDataFromHTML(fullEntry);
        
        if (type == "auth" || type == "ftp" || type == "ssh") {
            QString auth = extractAuthFromHTML(fullEntry);
            if (!auth.isEmpty()) {
                QStringList parts = auth.split(":");
                if (parts.size() >= 2) {
                    result.login = parts[0];
                    result.password = parts[1];
                }
            }
        }
        
        // Only add if we have at least an IP
        if (!result.ip.isEmpty()) {
            results.push_back(result);
        }
    }
    
    return true;
}

bool ResultExporter::parseHTMLResults(const QString &resultsDir) {
    clear();
    
    QDir dir(resultsDir);
    if (!dir.exists()) {
        return false;
    }
    
    // Find all result subdirectories
    QStringList filters;
    filters << "results_*";
    QStringList entries = dir.entryList(filters, QDir::Dirs | QDir::NoDotAndDotDot);
    
    if (entries.isEmpty()) {
        // Try current directory structure
        QStringList dateFilters;
        dateFilters << "results_*";
        QStringList dateDirs = dir.entryList(dateFilters, QDir::Dirs | QDir::NoDotAndDotDot);
        
        for (const QString &dateDir : dateDirs) {
            QDir subDir(dir.absoluteFilePath(dateDir));
            parseHTMLFile(subDir.absoluteFilePath(QString(TYPE1) + ".html"), "camera");
            parseHTMLFile(subDir.absoluteFilePath(QString(TYPE2) + ".html"), "other");
            parseHTMLFile(subDir.absoluteFilePath(QString(TYPE3) + ".html"), "auth");
            parseHTMLFile(subDir.absoluteFilePath(QString(TYPE4) + ".html"), "ftp");
            parseHTMLFile(subDir.absoluteFilePath(QString(TYPE5) + ".html"), "ssh");
        }
        return !results.empty();
    }
    
    // Parse files in each subdirectory
    for (const QString &entry : entries) {
        QDir subDir(dir.absoluteFilePath(entry));
        parseHTMLFile(subDir.absoluteFilePath(QString(TYPE1) + ".html"), "camera");
        parseHTMLFile(subDir.absoluteFilePath(QString(TYPE2) + ".html"), "other");
        parseHTMLFile(subDir.absoluteFilePath(QString(TYPE3) + ".html"), "auth");
        parseHTMLFile(subDir.absoluteFilePath(QString(TYPE4) + ".html"), "ftp");
        parseHTMLFile(subDir.absoluteFilePath(QString(TYPE5) + ".html"), "ssh");
    }
    
    return !results.empty();
}

bool ResultExporter::matchesFilter(const ScanResult &result, const QString &ipFilter,
                                   const QStringList &portFilters, const QStringList &typeFilters) {
    // IP filter (supports partial match or CIDR-like patterns)
    if (!ipFilter.isEmpty()) {
        if (!result.ip.contains(ipFilter, Qt::CaseInsensitive)) {
            return false;
        }
    }
    
    // Port filter
    if (!portFilters.isEmpty()) {
        bool portMatches = false;
        for (const QString &portFilter : portFilters) {
            bool ok;
            int port = portFilter.toInt(&ok);
            if (ok && result.port == port) {
                portMatches = true;
                break;
            }
        }
        if (!portMatches) return false;
    }
    
    // Type filter
    if (!typeFilters.isEmpty()) {
        if (!typeFilters.contains(result.type, Qt::CaseInsensitive)) {
            return false;
        }
    }
    
    return true;
}

QJsonArray ResultExporter::filterResults(const QString &ipFilter,
                                         const QStringList &portFilters,
                                         const QStringList &typeFilters) {
    QJsonArray filtered;
    
    for (const ScanResult &result : results) {
        if (matchesFilter(result, ipFilter, portFilters, typeFilters)) {
            QJsonObject obj;
            obj["ip"] = result.ip;
            obj["port"] = result.port;
            obj["type"] = result.type;
            obj["timestamp"] = result.timestamp;
            obj["data"] = result.data;
            if (!result.login.isEmpty()) {
                obj["login"] = result.login;
            }
            if (!result.password.isEmpty()) {
                obj["password"] = result.password;
            }
            filtered.append(obj);
        }
    }
    
    return filtered;
}

bool ResultExporter::exportToJSON(const QString &filename, const QString &ipFilter,
                                  const QStringList &portFilters,
                                  const QStringList &typeFilters) {
    QJsonObject root;
    root["export_timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["total_results"] = static_cast<int>(results.size());
    
    QJsonArray filtered = filterResults(ipFilter, portFilters, typeFilters);
    root["filtered_results"] = static_cast<int>(filtered.size());
    root["results"] = filtered;
    
    // Add statistics
    root["statistics"] = getStatistics();
    
    QJsonDocument doc(root);
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    file.close();
    
    return true;
}

bool ResultExporter::exportToCSV(const QString &filename, const QString &ipFilter,
                                 const QStringList &portFilters,
                                 const QStringList &typeFilters) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    
    // CSV Header
    out << "IP,Port,Type,Timestamp,Data,Login,Password\n";
    
    // Write filtered results
    for (const ScanResult &result : results) {
        if (matchesFilter(result, ipFilter, portFilters, typeFilters)) {
            // Escape CSV fields
            auto escapeCSV = [](const QString &str) -> QString {
                QString escaped = str;
                escaped.replace("\"", "\"\"");
                if (escaped.contains(',') || escaped.contains('"') || escaped.contains('\n')) {
                    escaped = "\"" + escaped + "\"";
                }
                return escaped;
            };
            
            out << escapeCSV(result.ip) << ","
                << result.port << ","
                << escapeCSV(result.type) << ","
                << escapeCSV(result.timestamp) << ","
                << escapeCSV(result.data) << ","
                << escapeCSV(result.login) << ","
                << escapeCSV(result.password) << "\n";
        }
    }
    
    file.close();
    return true;
}

QJsonObject ResultExporter::getStatistics() {
    QJsonObject stats;
    
    std::map<QString, int> typeCounts;
    std::map<int, int> portCounts;
    
    for (const ScanResult &result : results) {
        typeCounts[result.type]++;
        portCounts[result.port]++;
    }
    
    QJsonObject typeStats;
    for (const auto &pair : typeCounts) {
        typeStats[pair.first] = pair.second;
    }
    stats["by_type"] = typeStats;
    
    QJsonArray topPorts;
    // Get top 10 ports
    std::vector<std::pair<int, int>> portVec(portCounts.begin(), portCounts.end());
    std::sort(portVec.begin(), portVec.end(), 
              [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
                  return a.second > b.second;
              });
    
    for (size_t i = 0; i < std::min(static_cast<size_t>(10), portVec.size()); ++i) {
        QJsonObject portStat;
        portStat["port"] = portVec[i].first;
        portStat["count"] = portVec[i].second;
        topPorts.append(portStat);
    }
    stats["top_ports"] = topPorts;
    
    stats["total"] = static_cast<int>(results.size());
    
    return stats;
}

void ResultExporter::clear() {
    results.clear();
}

