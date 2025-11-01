#include "NetworkScanner.h"
#include <STh.h>
#include <externData.h>
#include <QDebug>

// Define global variables that are used by backend (normally defined in main.cpp)
STh *stt = nullptr;
bool globalScanFlag = false;

struct ScanResult {
    QString ip;
    int port;
    QString type;
    QString timestamp;
    QString data;
    QString login;
    QString password;
};

NetworkScanner::NetworkScanner(QObject *parent)
    : QObject(parent)
    , m_stt(nullptr)
    , m_scanThread(nullptr)
    , m_threads(100)
    , m_timeout(3000)
    , m_verifySSL(false)
    , m_adaptive(false)
    , m_smartScan(false)
    , m_deepScan(false)
    , m_vulnScan(false)
    , m_serviceVersion(false)
    , m_deviceIdentification(true)
    , m_monitoringMode(false)
    , m_monitoringInterval(300)
    , m_isRunning(false)
{
}

NetworkScanner::~NetworkScanner()
{
    stopScan();
    
    if (m_stt) {
        delete m_stt;
    }
}

void NetworkScanner::startScan(const QString &mode, const QString &target,
                                const QString &ports, int threads, int timeout,
                                bool verifySSL, bool adaptive, bool smartScan,
                                bool deepScan, bool vulnScan, bool serviceVersion,
                                bool deviceIdentification, bool monitoringMode,
                                int monitoringInterval)
{
    if (m_isRunning) {
        return;
    }
    
    // Store parameters
    m_mode = mode;
    m_target = target;
    m_ports = ports;
    m_threads = threads;
    m_timeout = timeout;
    m_verifySSL = verifySSL;
    m_adaptive = adaptive;
    m_smartScan = smartScan;
    m_deepScan = deepScan;
    m_vulnScan = vulnScan;
    m_serviceVersion = serviceVersion;
    m_deviceIdentification = deviceIdentification;
    m_monitoringMode = monitoringMode;
    m_monitoringInterval = monitoringInterval;
    
    m_isRunning = true;
    
    // Create scanner thread
    m_scanThread = QThread::create([this]() {
        this->scanInThread();
    });
    
    // Connect thread finish signal
    connect(m_scanThread, &QThread::finished, this, &NetworkScanner::onScanFinished);
    connect(m_scanThread, &QThread::finished, m_scanThread, &QObject::deleteLater);
    
    // Start thread
    m_scanThread->start();
}

void NetworkScanner::stopScan()
{
    m_isRunning = false;
    
    if (m_stt) {
        // Stop scanning
        globalScanFlag = false;
    }
    
    if (m_scanThread && m_scanThread->isRunning()) {
        m_scanThread->quit();
        m_scanThread->wait(3000);
    }
}

void NetworkScanner::scanInThread()
{
    try {
        // Create scanner object
        m_stt = new STh();
        
        // Set scan parameters
        if (m_mode == "ip") {
            m_stt->setMode(0);
            m_stt->setTarget(m_target);
        } else if (m_mode == "dns") {
            m_stt->setMode(1);
            m_stt->setTarget(m_target);
        } else if (m_mode == "import") {
            m_stt->setMode(-1);
            m_stt->setTarget(m_target);
        }
        
        m_stt->setPorts(m_ports);
        
        // Set global variables (these might be accessed elsewhere)
        gThreads = m_threads;
        gTimeOut = m_timeout / 1000;  // Convert to seconds
        gVerifySSL = m_verifySSL;
        gAdaptiveScan = m_adaptive;
        gSmartScan = m_smartScan;
        globalScanFlag = true;
        
        // Start scanning
        emit progress(0, 100);
        
        m_stt->start();
        m_stt->wait();
        
        // Emit completion
        emit finished();
        
    } catch (...) {
        emit error("An error occurred during scanning");
    }
}

void NetworkScanner::onScanFinished()
{
    m_isRunning = false;
    emit finished();
}

void NetworkScanner::onScanError(const QString &errorMsg)
{
    m_isRunning = false;
    emit error(errorMsg);
}

