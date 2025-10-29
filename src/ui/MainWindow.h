#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>

class ScanConfiguration;
class ScanResults;
class NetworkScanner;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startScan();
    void stopScan();
    void onScanProgress(int found, int total);
    void onScanComplete();
    void onScanError(const QString &error);
    void updateStatistics();

private:
    void setupUI();
    void createMenuBar();
    void createStatusBar();
    void connectSignals();
    
    // UI Components
    QTabWidget *m_tabWidget;
    ScanConfiguration *m_scanConfig;
    ScanResults *m_scanResults;
    
    // Backend
    NetworkScanner *m_scanner;
    
    // Status bar
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_statsLabel;
    
    // Timer for statistics updates
    QTimer *m_statsTimer;
    
    // Statistics
    int m_totalFound;
    int m_totalScanned;
};

#endif // MAINWINDOW_H

