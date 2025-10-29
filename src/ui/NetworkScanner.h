#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <QObject>
#include <QThread>
#include <QStringList>
#include <memory>

class STh;
struct ScanResult;

class NetworkScanner : public QObject
{
    Q_OBJECT

public:
    explicit NetworkScanner(QObject *parent = nullptr);
    ~NetworkScanner();
    
    void startScan(const QString &mode, const QString &target, 
                   const QString &ports, int threads, int timeout,
                   bool verifySSL, bool adaptive, bool smartScan);
    void stopScan();

signals:
    void progress(int found, int total);
    void resultFound(const ScanResult &result);
    void finished();
    void error(const QString &error);

private slots:
    void onScanFinished();
    void onScanError(const QString &error);

private:
    void scanInThread();
    
    STh *m_stt;
    QThread *m_scanThread;
    
    // Scan parameters
    QString m_mode;
    QString m_target;
    QString m_ports;
    int m_threads;
    int m_timeout;
    bool m_verifySSL;
    bool m_adaptive;
    bool m_smartScan;
    
    bool m_isRunning;
};

#endif // NETWORKSCANNER_H

