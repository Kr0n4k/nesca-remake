#ifndef RESULTEXPORTER_H
#define RESULTEXPORTER_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>
#include <QDir>
#include <vector>
#include <map>

struct ScanResult {
    QString ip;
    int port;
    QString type;  // "camera", "auth", "ftp", "ssh", "other"
    QString timestamp;
    QString data;  // Additional data (title, path, etc.)
    QString login;
    QString password;
    QString additionalInfo;
};

class ResultExporter {
private:
    std::vector<ScanResult> results;
    QString outputDir;
    
    // Helper methods
    QString extractIPFromHTML(const QString &html);
    QString extractPortFromHTML(const QString &html);
    QString extractTimestampFromHTML(const QString &html);
    QString extractDataFromHTML(const QString &html);
    QString extractAuthFromHTML(const QString &html);
    
    bool matchesFilter(const ScanResult &result, const QString &ipFilter, 
                      const QStringList &portFilters, const QStringList &typeFilters);
    
public:
    ResultExporter();
    
    // Export methods
    bool exportToJSON(const QString &filename, const QString &ipFilter = "",
                     const QStringList &portFilters = QStringList(),
                     const QStringList &typeFilters = QStringList());
    bool exportToCSV(const QString &filename, const QString &ipFilter = "",
                    const QStringList &portFilters = QStringList(),
                    const QStringList &typeFilters = QStringList());
    
    // Parse HTML result files
    bool parseHTMLResults(const QString &resultsDir);
    bool parseHTMLFile(const QString &filepath, const QString &type);
    
    // Filter methods
    QJsonArray filterResults(const QString &ipFilter = "",
                            const QStringList &portFilters = QStringList(),
                            const QStringList &typeFilters = QStringList());
    
    // Statistics
    QJsonObject getStatistics();
    
    // Clear results
    void clear();
    
    // Get all results
    std::vector<ScanResult> getResults() const { return results; }
};

#endif // RESULTEXPORTER_H

