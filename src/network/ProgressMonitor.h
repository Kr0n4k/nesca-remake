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
    bool useColor;
    bool showProgressBar;
    std::chrono::steady_clock::time_point startTime;
    
public:
    ProgressMonitor(QObject *parent = nullptr);
    ~ProgressMonitor();
    
    void setColorOutput(bool enable) { useColor = enable; }
    void setProgressBar(bool enable) { showProgressBar = enable; }
    
    void run() override;
    void stop();
    void printProgress();
    void printProgressBar(int current, int total, int width = 50);
    
signals:
    void progressUpdated();
};

#endif // PROGRESSMONITOR_H

