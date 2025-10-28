#ifndef PROGRESSMONITOR_H
#define PROGRESSMONITOR_H

#include <QThread>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>
#include <chrono>

class ProgressMonitor : public QThread {
    Q_OBJECT
    
private:
    bool shouldStop;
    std::chrono::steady_clock::time_point startTime;
    
public:
    ProgressMonitor(QObject *parent = nullptr);
    ~ProgressMonitor();
    
    void run() override;
    void stop();
    void printProgress();
    
signals:
    void progressUpdated();
};

#endif // PROGRESSMONITOR_H

