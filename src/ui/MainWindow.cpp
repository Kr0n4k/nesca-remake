#include "MainWindow.h"
#include "ScanConfiguration.h"
#include "ScanResults.h"
#include "NetworkScanner.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_scanConfig(nullptr)
    , m_scanResults(nullptr)
    , m_scanner(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_statsLabel(nullptr)
    , m_statsTimer(nullptr)
    , m_totalFound(0)
    , m_totalScanned(0)
{
    setupUI();
    createMenuBar();
    createStatusBar();
    connectSignals();
    
    setWindowTitle(tr("Nesca v2r - Network Scanner"));
    setMinimumSize(900, 650);
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    if (m_statsTimer) {
        m_statsTimer->stop();
    }
    if (m_scanner) {
        delete m_scanner;
    }
}

void MainWindow::setupUI()
{
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);
    
    // Create configuration widget
    m_scanConfig = new ScanConfiguration(this);
    m_tabWidget->addTab(m_scanConfig, tr("Scan Configuration"));
    
    // Create results widget
    m_scanResults = new ScanResults(this);
    m_tabWidget->addTab(m_scanResults, tr("Results"));
    
    // Create scanner
    m_scanner = new NetworkScanner(this);
}

void MainWindow::createMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QAction *openLogAction = fileMenu->addAction(tr("&Open Logs..."));
    openLogAction->setShortcut(QKeySequence::Open);
    openLogAction->setToolTip("Open log files from previous scans");
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // View menu
    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    QAction *showConfig = viewMenu->addAction(tr("Show &Configuration"));
    showConfig->setCheckable(true);
    showConfig->setChecked(true);
    connect(showConfig, &QAction::toggled, [this](bool checked) {
        if (checked) {
            m_tabWidget->setCurrentIndex(0);
        }
    });
    
    QAction *showResults = viewMenu->addAction(tr("Show &Results"));
    showResults->setCheckable(true);
    connect(showResults, &QAction::toggled, [this](bool checked) {
        if (checked) {
            m_tabWidget->setCurrentIndex(1);
        }
    });
    
    // Tools menu
    QMenu *toolsMenu = menuBar->addMenu(tr("&Tools"));
    QAction *cliModeAction = toolsMenu->addAction(tr("&CLI Mode..."));
    cliModeAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    cliModeAction->setToolTip("Generate CLI command from current settings");
    connect(cliModeAction, &QAction::triggered, [this]() {
        if (m_scanConfig) {
            QString cmd = QString("--ip %1 -p %2 -t %3")
                .arg(m_scanConfig->target())
                .arg(m_scanConfig->ports())
                .arg(m_scanConfig->threads());
            if (m_scanConfig->verifySSL()) cmd += " --verify-ssl";
            if (m_scanConfig->adaptiveScan()) cmd += " --adaptive";
            if (m_scanConfig->smartScan()) cmd += " --smart-scan";
            
            QMessageBox::information(this, tr("CLI Command"), 
                tr("CLI command for current settings:\n\n./nesca %1\n\nCopy this command to use in terminal.").arg(cmd));
        }
    });
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    QAction *helpAction = helpMenu->addAction(tr("&User Guide"));
    helpAction->setShortcut(QKeySequence::HelpContents);
    connect(helpAction, &QAction::triggered, [this]() {
        QMessageBox::information(this, tr("User Guide"),
            tr("QUICK START:\n\n"
               "1. Select scan mode (IP Range/DNS/Import)\n"
               "2. Enter target (e.g., 192.168.1.0/24)\n"
               "3. Configure ports and threads\n"
               "4. Click 'Start Scan'\n\n"
               "TIPS:\n"
               "• Use Ctrl+Shift+C to view CLI equivalent\n"
               "• Hover over fields for tooltips\n"
               "• Double-click results for details\n"
               "• Export results to JSON/CSV"));
    });
    
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("About Nesca v2r"),
            tr("Nesca v2r - Network Scanner\n\n"
               "A powerful network scanning tool for discovering IP cameras, "
               "servers, and network devices.\n\n"
               "Version 2.0\n\n"
               "© 2025 NESCA Project\n"
               "Built with Qt 5.15+"));
    });
}

void MainWindow::createStatusBar()
{
    QStatusBar *statusBar = this->statusBar();
    
    m_statusLabel = new QLabel(tr("Ready"));
    statusBar->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    statusBar->addPermanentWidget(m_progressBar);
    
    m_statsLabel = new QLabel(tr("Found: 0"));
    statusBar->addPermanentWidget(m_statsLabel);
}

void MainWindow::connectSignals()
{
    // Connect scan configuration signals
    connect(m_scanConfig, &ScanConfiguration::scanRequested, this, &MainWindow::startScan);
    connect(m_scanConfig, &ScanConfiguration::scanStopped, this, &MainWindow::stopScan);
    
    // Connect scanner signals
    if (m_scanner) {
        connect(m_scanner, &NetworkScanner::progress, this, &MainWindow::onScanProgress);
        connect(m_scanner, &NetworkScanner::finished, this, &MainWindow::onScanComplete);
        connect(m_scanner, &NetworkScanner::error, this, &MainWindow::onScanError);
        connect(m_scanner, &NetworkScanner::resultFound, m_scanResults, &ScanResults::addResult);
    }
    
    // Create statistics update timer
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateStatistics);
}

void MainWindow::startScan()
{
    if (!m_scanner) {
        return;
    }
    
    // Get scan parameters
    QString mode = m_scanConfig->scanMode();
    QString target = m_scanConfig->target();
    QString ports = m_scanConfig->ports();
    int threads = m_scanConfig->threads();
    int timeout = m_scanConfig->timeout();
    bool verifySSL = m_scanConfig->verifySSL();
    bool adaptive = m_scanConfig->adaptiveScan();
    bool smartScan = m_scanConfig->smartScan();
    bool deepScan = m_scanConfig->deepScan();
    bool vulnScan = m_scanConfig->vulnScan();
    bool serviceVersion = m_scanConfig->serviceVersion();
    bool deviceIdentification = m_scanConfig->deviceIdentification();
    bool monitoringMode = m_scanConfig->monitoringMode();
    int monitoringInterval = m_scanConfig->monitoringInterval();
    
    // Validate inputs
    if (target.isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Please enter a target."));
        return;
    }
    
    if (ports.isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Please enter ports to scan."));
        return;
    }
    
    // Clear previous results
    m_scanResults->clearResults();
    m_totalFound = 0;
    m_totalScanned = 0;
    
    // Start scanner
    m_scanner->startScan(mode, target, ports, threads, timeout, verifySSL, adaptive, smartScan,
                         deepScan, vulnScan, serviceVersion, deviceIdentification,
                         monitoringMode, monitoringInterval);
    
    // Update UI
    m_statusLabel->setText(tr("Scanning..."));
    m_progressBar->setValue(0);
    m_statsTimer->start(1000); // Update every second
    
    // Switch to results tab
    m_tabWidget->setCurrentIndex(1);
}

void MainWindow::stopScan()
{
    if (m_scanner) {
        m_scanner->stopScan();
    }
    
    m_statsTimer->stop();
    m_statusLabel->setText(tr("Scan stopped"));
}

void MainWindow::onScanProgress(int found, int total)
{
    m_totalFound = found;
    m_totalScanned = total;
    
    if (total > 0) {
        int percent = (found * 100) / total;
        m_progressBar->setValue(percent);
    }
    
    m_statsLabel->setText(tr("Found: %1").arg(found));
}

void MainWindow::onScanComplete()
{
    m_statsTimer->stop();
    m_statusLabel->setText(tr("Scan complete"));
    m_progressBar->setValue(100);
    
    // Update results statistics
    m_scanResults->updateStatistics();
    
    QMessageBox::information(this, tr("Scan Complete"), 
        tr("Scan completed. Found %1 results.").arg(m_scanResults->getResultCount()));
}

void MainWindow::onScanError(const QString &error)
{
    m_statsTimer->stop();
    m_statusLabel->setText(tr("Error"));
    
    QMessageBox::critical(this, tr("Scan Error"), error);
}

void MainWindow::updateStatistics()
{
    m_statsLabel->setText(tr("Found: %1").arg(m_scanResults->getResultCount()));
}

