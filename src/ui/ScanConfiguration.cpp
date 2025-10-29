#include "ScanConfiguration.h"
#include <QPushButton>

ScanConfiguration::ScanConfiguration(QWidget *parent)
    : QWidget(parent)
    , m_scanModeCombo(nullptr)
    , m_targetEdit(nullptr)
    , m_portsEdit(nullptr)
    , m_threadsSpin(nullptr)
    , m_advancedGroup(nullptr)
    , m_timeoutSpin(nullptr)
    , m_verifySSLCheck(nullptr)
    , m_adaptiveCheck(nullptr)
    , m_smartScanCheck(nullptr)
    , m_exportGroup(nullptr)
    , m_exportFormatCombo(nullptr)
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_isScanning(false)
{
    setupUI();
}

void ScanConfiguration::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Scan mode selection
    QFormLayout *modeLayout = new QFormLayout();
    m_scanModeCombo = new QComboBox(this);
    m_scanModeCombo->addItem("IP Range", "ip");
    m_scanModeCombo->addItem("DNS Scan", "dns");
    m_scanModeCombo->addItem("Import from File", "import");
    m_scanModeCombo->setToolTip("Select scanning mode: IP Range (192.168.1.0/24), DNS Scan (example.com), or Import from file");
    modeLayout->addRow("Scan Mode:", m_scanModeCombo);
    mainLayout->addLayout(modeLayout);
    
    // Target input
    QFormLayout *targetLayout = new QFormLayout();
    m_targetEdit = new QLineEdit(this);
    m_targetEdit->setPlaceholderText("192.168.1.0/24 or 192.168.1.1-192.168.1.255");
    m_targetEdit->setToolTip("Enter target: IP range (CIDR or dash notation), DNS name, or file path for import mode");
    targetLayout->addRow("Target:", m_targetEdit);
    mainLayout->addLayout(targetLayout);
    
    // Ports
    QFormLayout *portsLayout = new QFormLayout();
    m_portsEdit = new QLineEdit(this);
    m_portsEdit->setText("80,81,88,8080,8081,60001,60002,8008,8888,554,9000,3536,21");
    m_portsEdit->setPlaceholderText("80,443,8080");
    m_portsEdit->setToolTip("Comma-separated list of ports to scan. Common ports: 80 (HTTP), 443 (HTTPS), 8080 (HTTP-alt), 554 (RTSP), 21 (FTP), 22 (SSH)");
    portsLayout->addRow("Ports:", m_portsEdit);
    mainLayout->addLayout(portsLayout);
    
    // Threads
    QFormLayout *threadsLayout = new QFormLayout();
    m_threadsSpin = new QSpinBox(this);
    m_threadsSpin->setRange(1, 2000);
    m_threadsSpin->setValue(100);
    m_threadsSpin->setToolTip("Number of parallel threads. Higher values = faster scan but more resources. Default: 100");
    threadsLayout->addRow("Threads:", m_threadsSpin);
    mainLayout->addLayout(threadsLayout);
    
    // Advanced options group
    m_advancedGroup = new QGroupBox("Advanced Options", this);
    QFormLayout *advancedLayout = new QFormLayout();
    
    m_timeoutSpin = new QSpinBox(this);
    m_timeoutSpin->setRange(1, 60000);
    m_timeoutSpin->setValue(3000);
    m_timeoutSpin->setSuffix(" ms");
    m_timeoutSpin->setToolTip("Connection timeout in milliseconds. Increase for slow networks. Default: 3000ms");
    advancedLayout->addRow("Timeout:", m_timeoutSpin);
    
    m_verifySSLCheck = new QCheckBox("Verify SSL certificates", this);
    m_verifySSLCheck->setToolTip("Enable SSL certificate verification (more secure but may reject self-signed certs)");
    advancedLayout->addRow("", m_verifySSLCheck);
    
    m_adaptiveCheck = new QCheckBox("Adaptive scan", this);
    m_adaptiveCheck->setToolTip("Automatically adjust thread count based on network load and performance");
    advancedLayout->addRow("", m_adaptiveCheck);
    
    m_smartScanCheck = new QCheckBox("Smart scan", this);
    m_smartScanCheck->setToolTip("Prioritize popular ports for found IPs to speed up the scan");
    advancedLayout->addRow("", m_smartScanCheck);
    
    m_advancedGroup->setLayout(advancedLayout);
    mainLayout->addWidget(m_advancedGroup);
    
    // Export group
    m_exportGroup = new QGroupBox("Export Options", this);
    QFormLayout *exportLayout = new QFormLayout();
    
    m_exportFormatCombo = new QComboBox(this);
    m_exportFormatCombo->addItem("JSON", "json");
    m_exportFormatCombo->addItem("CSV", "csv");
    m_exportFormatCombo->addItem("Both", "both");
    m_exportFormatCombo->setCurrentIndex(0);
    m_exportFormatCombo->setToolTip("Export format for results: JSON (structured data), CSV (spreadsheet), or Both");
    exportLayout->addRow("Export Format:", m_exportFormatCombo);
    
    m_exportGroup->setLayout(exportLayout);
    mainLayout->addWidget(m_exportGroup);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_startButton = new QPushButton("Start Scan", this);
    m_stopButton = new QPushButton("Stop Scan", this);
    m_stopButton->setEnabled(false);
    
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
    
    // Connect signals
    connect(m_startButton, &QPushButton::clicked, this, &ScanConfiguration::onStartScan);
    connect(m_stopButton, &QPushButton::clicked, this, &ScanConfiguration::onStopScan);
    connect(m_scanModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ScanConfiguration::onScanModeChanged);
}

QString ScanConfiguration::scanMode() const
{
    return m_scanModeCombo->currentData().toString();
}

QString ScanConfiguration::target() const
{
    return m_targetEdit->text();
}

QString ScanConfiguration::ports() const
{
    return m_portsEdit->text();
}

int ScanConfiguration::threads() const
{
    return m_threadsSpin->value();
}

int ScanConfiguration::timeout() const
{
    return m_timeoutSpin->value();
}

bool ScanConfiguration::verifySSL() const
{
    return m_verifySSLCheck->isChecked();
}

bool ScanConfiguration::adaptiveScan() const
{
    return m_adaptiveCheck->isChecked();
}

bool ScanConfiguration::smartScan() const
{
    return m_smartScanCheck->isChecked();
}

QString ScanConfiguration::exportFormat() const
{
    return m_exportFormatCombo->currentData().toString();
}

void ScanConfiguration::onScanModeChanged(int index)
{
    // Update placeholder text based on mode
    QString mode = m_scanModeCombo->itemData(index).toString();
    if (mode == "ip") {
        m_targetEdit->setPlaceholderText("192.168.1.0/24 or 192.168.1.1-192.168.1.255");
    } else if (mode == "dns") {
        m_targetEdit->setPlaceholderText("example.com");
    } else if (mode == "import") {
        m_targetEdit->setPlaceholderText("/path/to/file.txt");
    }
}

void ScanConfiguration::onStartScan()
{
    m_isScanning = true;
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    emit scanRequested();
}

void ScanConfiguration::onStopScan()
{
    m_isScanning = false;
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    emit scanStopped();
}

