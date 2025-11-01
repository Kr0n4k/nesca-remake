#include "ScanConfiguration.h"
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QHBoxLayout>

ScanConfiguration::ScanConfiguration(QWidget *parent)
    : QWidget(parent)
    , m_profileCombo(nullptr)
    , m_scanModeCombo(nullptr)
    , m_targetEdit(nullptr)
    , m_browseButton(nullptr)
    , m_portsEdit(nullptr)
    , m_threadsSpin(nullptr)
    , m_advancedGroup(nullptr)
    , m_timeoutSpin(nullptr)
    , m_verifySSLCheck(nullptr)
    , m_adaptiveCheck(nullptr)
    , m_smartScanCheck(nullptr)
    , m_deepScanCheck(nullptr)
    , m_vulnScanCheck(nullptr)
    , m_serviceVersionCheck(nullptr)
    , m_deviceIdentificationCheck(nullptr)
    , m_monitoringGroup(nullptr)
    , m_monitoringModeCheck(nullptr)
    , m_monitoringIntervalSpin(nullptr)
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
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    
    // Welcome header
    QLabel *welcomeLabel = new QLabel("Nesca v2r - Network Scanner", this);
    welcomeLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #60a5fa; padding-bottom: 10px;");
    mainLayout->addWidget(welcomeLabel);
    
    // Info label
    QLabel *infoLabel = new QLabel("Configure your scan parameters and click Start Scan", this);
    infoLabel->setStyleSheet("font-size: 11px; color: #a0a0a0; padding-bottom: 15px;");
    mainLayout->addWidget(infoLabel);
    
    // Quick guide hint
    QLabel *guideHint = new QLabel("💡 <b>New to scanning?</b> Hover over any field for detailed help", this);
    guideHint->setStyleSheet(
        "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #2a5a80, stop: 1 #1a3a5a);"
        "color: #a0d0ff;"
        "padding: 10px;"
        "border-radius: 6px;"
        "margin-bottom: 10px;"
        "font-size: 12px;"
    );
    guideHint->setWordWrap(true);
    mainLayout->addWidget(guideHint);
    
    // Profile selection
    QFormLayout *profileLayout = new QFormLayout();
    profileLayout->setVerticalSpacing(12);
    m_profileCombo = new QComboBox(this);
    m_profileCombo->addItem("None (Custom)", "");
    m_profileCombo->addItem("🚀 Quick Scan", "quick-scan");
    m_profileCombo->addItem("🔍 Full Scan", "full-scan");
    m_profileCombo->addItem("🕵️ Stealth Scan", "stealth-scan");
    m_profileCombo->addItem("📱 IoT Scan", "iot-scan");
    m_profileCombo->addItem("🌐 Network Scan", "network-scan");
    m_profileCombo->addItem("🛡️ Pentest", "pentest");
    m_profileCombo->setToolTip(
        "<b>Preset Profiles:</b><br>"
        "Choose a pre-configured profile or use Custom.<br><br>"
        "<b>Quick Scan:</b> Fast basic scan<br>"
        "<b>Full Scan:</b> Comprehensive scan<br>"
        "<b>Stealth Scan:</b> Slow, quiet scanning<br>"
        "<b>IoT Scan:</b> Optimized for IoT devices<br>"
        "<b>Network Scan:</b> General network discovery<br>"
        "<b>Pentest:</b> Penetration testing mode"
    );
    profileLayout->addRow("Profile:", m_profileCombo);
    mainLayout->addLayout(profileLayout);
    mainLayout->addWidget(new QLabel("")); // Spacer
    
    // Scan mode selection
    QFormLayout *modeLayout = new QFormLayout();
    modeLayout->setVerticalSpacing(12);
    m_scanModeCombo = new QComboBox(this);
    m_scanModeCombo->addItem("IP Range", "ip");
    m_scanModeCombo->addItem("DNS Scan", "dns");
    m_scanModeCombo->addItem("Import from File", "import");
    m_scanModeCombo->setToolTip(
        "<b>How do I choose the scan mode?</b><br>"
        "• <b>IP Range</b>: Scan a range of IP addresses (e.g., 192.168.1.0/24)<br>"
        "• <b>DNS Scan</b>: Scan a domain name or hostname<br>"
        "• <b>Import from File</b>: Load IPs from a text file"
    );
    
    QLabel *modeHelp = new QLabel("ℹ️ Choose how you want to specify targets", this);
    modeHelp->setStyleSheet("font-size: 10px; color: #888; font-style: italic;");
    modeLayout->addRow("Scan Mode:", m_scanModeCombo);
    modeLayout->addRow("", modeHelp);
    mainLayout->addLayout(modeLayout);
    
    // Target input
    QFormLayout *targetLayout = new QFormLayout();
    targetLayout->setVerticalSpacing(12);
    
    // Target input with browse button
    QHBoxLayout *targetInputLayout = new QHBoxLayout();
    targetInputLayout->setSpacing(8);
    
    m_targetEdit = new QLineEdit(this);
    m_targetEdit->setPlaceholderText("192.168.1.0/24 or 192.168.1.1-192.168.1.255");
    m_targetEdit->setToolTip(
        "<b>Examples for Target field:</b><br>"
        "<b>CIDR notation:</b><br>"
        "• 192.168.1.0/24 (scans 192.168.1.1-254)<br>"
        "• 10.0.0.0/16 (scans 10.0.x.x - 65536 hosts)<br><br>"
        "<b>Range notation:</b><br>"
        "• 192.168.1.1-192.168.1.100<br>"
        "• 192.168.1.1-254<br><br>"
        "<b>Single IP:</b> 192.168.1.1<br>"
        "<b>DNS:</b> example.com<br>"
        "<b>File:</b> /path/to/ips.txt (one IP per line)"
    );
    
    m_browseButton = new QPushButton("Browse...", this);
    m_browseButton->setStyleSheet(
        "QPushButton { padding: 8px 16px; min-width: 80px; }"
    );
    m_browseButton->setToolTip("Browse for a file to import IP addresses from");
    
    targetInputLayout->addWidget(m_targetEdit);
    targetInputLayout->addWidget(m_browseButton);
    
    QLabel *targetHelp = new QLabel("ℹ️ Use CIDR (/24) or range (192.168.1.1-255) notation", this);
    targetHelp->setStyleSheet("font-size: 10px; color: #888; font-style: italic;");
    
    targetLayout->addRow("Target:", targetInputLayout);
    targetLayout->addRow("", targetHelp);
    mainLayout->addLayout(targetLayout);
    
    // Ports
    QFormLayout *portsLayout = new QFormLayout();
    portsLayout->setVerticalSpacing(12);
    m_portsEdit = new QLineEdit(this);
    m_portsEdit->setText("80,81,88,8080,8081,60001,60002,8008,8888,554,9000,3536,21");
    m_portsEdit->setPlaceholderText("80,443,8080");
    m_portsEdit->setToolTip(
        "<b>Common ports to scan:</b><br>"
        "<b>Web:</b> 80 (HTTP), 443 (HTTPS), 8080, 8000, 8443<br>"
        "<b>Streaming:</b> 554 (RTSP), 1935 (RTMP), 9000<br>"
        "<b>Cameras:</b> 37777, 34567, 8000, 88<br>"
        "<b>Other:</b> 21 (FTP), 22 (SSH), 23 (Telnet), 3389 (RDP)<br><br>"
        "<b>Format:</b> Separate with commas (80,443,8080)<br>"
        "Or use ranges: 80-100 or 8000-8100"
    );
    
    QLabel *portsHelp = new QLabel("ℹ️ Separate ports with commas. Default: common web & camera ports", this);
    portsHelp->setStyleSheet("font-size: 10px; color: #888; font-style: italic;");
    portsLayout->addRow("Ports:", m_portsEdit);
    portsLayout->addRow("", portsHelp);
    mainLayout->addLayout(portsLayout);
    
    // Threads
    QFormLayout *threadsLayout = new QFormLayout();
    threadsLayout->setVerticalSpacing(12);
    m_threadsSpin = new QSpinBox(this);
    m_threadsSpin->setRange(1, 2000);
    m_threadsSpin->setValue(100);
    m_threadsSpin->setToolTip(
        "<b>What are threads?</b><br>"
        "Threads control how many connections happen at once.<br><br>"
        "<b>Recommended values:</b><br>"
        "• 50-100: Safe, normal usage<br>"
        "• 100-200: Faster, moderate network<br>"
        "• 200+: Very fast, stable network only<br><br>"
        "<b>Higher = faster</b> but uses more CPU/network. Default: 100"
    );
    
    QLabel *threadsHelp = new QLabel("ℹ️ More threads = faster scan. Recommended: 100", this);
    threadsHelp->setStyleSheet("font-size: 10px; color: #888; font-style: italic;");
    threadsLayout->addRow("Threads:", m_threadsSpin);
    threadsLayout->addRow("", threadsHelp);
    mainLayout->addLayout(threadsLayout);
    
    // Advanced options group
    m_advancedGroup = new QGroupBox("⚙ Advanced Options", this);
    QFormLayout *advancedLayout = new QFormLayout();
    advancedLayout->setVerticalSpacing(10);
    
    m_timeoutSpin = new QSpinBox(this);
    m_timeoutSpin->setRange(1, 60000);
    m_timeoutSpin->setValue(3000);
    m_timeoutSpin->setSuffix(" ms");
    m_timeoutSpin->setToolTip(
        "<b>Connection Timeout:</b><br>"
        "How long to wait for a response before giving up.<br><br>"
        "<b>Recommended:</b><br>"
        "• 1000-3000ms: Fast networks (default)<br>"
        "• 5000ms: Slower/remote networks<br>"
        "• 10000ms+: Very slow or unreliable networks<br><br>"
        "Too low = false negatives. Too high = slow scanning."
    );
    advancedLayout->addRow("Timeout:", m_timeoutSpin);
    
    m_verifySSLCheck = new QCheckBox("Verify SSL certificates", this);
    m_verifySSLCheck->setToolTip(
        "<b>SSL Certificate Verification:</b><br>"
        "Check if SSL certificates are valid and trusted.<br><br>"
        "<b>Checked:</b> Only accept valid certificates (more secure)<br>"
        "<b>Unchecked:</b> Accept any certificate, even self-signed<br><br>"
        "<b>Tip:</b> Uncheck if scanning IoT devices or cameras with self-signed certs"
    );
    advancedLayout->addRow("", m_verifySSLCheck);
    
    m_adaptiveCheck = new QCheckBox("Adaptive scan", this);
    m_adaptiveCheck->setToolTip(
        "<b>Adaptive Scanning:</b><br>"
        "Automatically adjust thread count based on network performance.<br><br>"
        "<b>How it works:</b><br>"
        "• Starts with your set thread count<br>"
        "• Monitors network latency and errors<br>"
        "• Increases threads if network is stable<br>"
        "• Decreases if too many timeouts occur<br><br>"
        "<b>Best for:</b> Large scans on varying network conditions"
    );
    advancedLayout->addRow("", m_adaptiveCheck);
    
    m_smartScanCheck = new QCheckBox("Smart scan", this);
    m_smartScanCheck->setToolTip(
        "<b>Smart Scanning:</b><br>"
        "Prioritize common ports to scan faster.<br><br>"
        "<b>How it works:</b><br>"
        "• First scans most likely ports (80, 443, etc.)<br>"
        "• After finding open ports, focuses on their protocols<br>"
        "• Skips unlikely combinations<br><br>"
        "<b>Effect:</b> Faster results, fewer false positives<br>"
        "<b>Good for:</b> Quick reconnaissance"
    );
    advancedLayout->addRow("", m_smartScanCheck);
    
    m_deepScanCheck = new QCheckBox("Deep scan (endpoint discovery)", this);
    m_deepScanCheck->setToolTip(
        "<b>Deep Scanning:</b><br>"
        "Discover hidden endpoints and API paths.<br><br>"
        "<b>What it does:</b><br>"
        "• Searches for common endpoints (/admin, /api, /config)<br>"
        "• Discovers undocumented API endpoints<br>"
        "• Finds firmware information<br>"
        "• Identifies device capabilities<br><br>"
        "<b>Use when:</b> Penetration testing, security audits"
    );
    advancedLayout->addRow("", m_deepScanCheck);
    
    m_vulnScanCheck = new QCheckBox("Vulnerability scan (CVE check)", this);
    m_vulnScanCheck->setToolTip(
        "<b>Vulnerability Scanning:</b><br>"
        "Check devices against known CVEs and exploits.<br><br>"
        "<b>Supported checks:</b><br>"
        "• CVE-2021-36260 (Hikvision)<br>"
        "• CVE-2021-33045 (Dahua)<br>"
        "• CVE-2017-7921 (Hikvision)<br>"
        "• Plus many more<br><br>"
        "<b>Warning:</b> Slow down scanning significantly<br>"
        "<b>Use for:</b> Security assessment, bug hunting"
    );
    advancedLayout->addRow("", m_vulnScanCheck);
    
    m_serviceVersionCheck = new QCheckBox("Service version detection", this);
    m_serviceVersionCheck->setToolTip(
        "<b>Service Version Detection:</b><br>"
        "Identify exact firmware and service versions.<br><br>"
        "<b>What it detects:</b><br>"
        "• HTTP server versions<br>"
        "• Firmware versions<br>"
        "• Device firmware build dates<br>"
        "• Application versions<br><br>"
        "<b>Use for:</b> Inventory, security auditing"
    );
    advancedLayout->addRow("", m_serviceVersionCheck);
    
    m_deviceIdentificationCheck = new QCheckBox("Device identification", this);
    m_deviceIdentificationCheck->setChecked(true); // Enabled by default
    m_deviceIdentificationCheck->setToolTip(
        "<b>Device Identification:</b><br>"
        "Auto-detect manufacturer and model.<br><br>"
        "<b>Supported brands:</b><br>"
        "• Hikvision, Dahua, Axis<br>"
        "• Panasonic, Sony, Samsung<br>"
        "• Bosch, Pelco, Vivotek<br>"
        "• Foscam, Uniview, Reolink, TP-Link<br><br>"
        "<b>Recommended:</b> Keep enabled for best results"
    );
    advancedLayout->addRow("", m_deviceIdentificationCheck);
    
    m_advancedGroup->setLayout(advancedLayout);
    mainLayout->addWidget(m_advancedGroup);
    
    // Monitoring group
    m_monitoringGroup = new QGroupBox("🔍 Monitoring Mode", this);
    QFormLayout *monitoringLayout = new QFormLayout();
    monitoringLayout->setVerticalSpacing(10);
    
    m_monitoringModeCheck = new QCheckBox("Enable continuous monitoring", this);
    m_monitoringModeCheck->setToolTip(
        "<b>Continuous Monitoring:</b><br>"
        "Repeatedly scan network for changes.<br><br>"
        "<b>How it works:</b><br>"
        "• Scans at specified intervals<br>"
        "• Detects new/removed devices<br>"
        "• Tracks device changes<br>"
        "• Creates snapshots<br><br>"
        "<b>Best for:</b> Network monitoring, security surveillance"
    );
    monitoringLayout->addRow("", m_monitoringModeCheck);
    
    m_monitoringIntervalSpin = new QSpinBox(this);
    m_monitoringIntervalSpin->setRange(60, 86400);
    m_monitoringIntervalSpin->setValue(300); // 5 minutes default
    m_monitoringIntervalSpin->setSuffix(" seconds");
    m_monitoringIntervalSpin->setToolTip(
        "<b>Monitoring Interval:</b><br>"
        "Time between scans.<br><br>"
        "<b>Recommended:</b><br>"
        "• 300s (5min): Normal monitoring<br>"
        "• 600s (10min): Light monitoring<br>"
        "• 3600s (1h): Periodic checks<br><br>"
        "<b>Range:</b> 60-86400 seconds"
    );
    monitoringLayout->addRow("Interval:", m_monitoringIntervalSpin);
    
    QLabel *monitoringHelp = new QLabel("ℹ️ Repeats scan automatically at specified interval", this);
    monitoringHelp->setStyleSheet("font-size: 10px; color: #888; font-style: italic;");
    monitoringLayout->addRow("", monitoringHelp);
    
    m_monitoringGroup->setLayout(monitoringLayout);
    mainLayout->addWidget(m_monitoringGroup);
    
    // Export group
    m_exportGroup = new QGroupBox("📊 Export Options", this);
    QFormLayout *exportLayout = new QFormLayout();
    exportLayout->setVerticalSpacing(10);
    
    m_exportFormatCombo = new QComboBox(this);
    m_exportFormatCombo->addItem("JSON", "json");
    m_exportFormatCombo->addItem("CSV", "csv");
    m_exportFormatCombo->addItem("Both", "both");
    m_exportFormatCombo->setCurrentIndex(0);
    m_exportFormatCombo->setToolTip(
        "<b>Export Format:</b><br>"
        "Choose how scan results are saved.<br><br>"
        "<b>JSON:</b> Structured data format, easy for scripts<br>"
        "<b>CSV:</b> Spreadsheet format (Excel, Google Sheets)<br>"
        "<b>Both:</b> Save in both formats<br><br>"
        "<b>Recommendation:</b> JSON for developers, CSV for users"
    );
    
    QLabel *exportHelp = new QLabel("ℹ️ Results will be saved in your chosen format", this);
    exportHelp->setStyleSheet("font-size: 10px; color: #888; font-style: italic;");
    exportLayout->addRow("Export Format:", m_exportFormatCombo);
    exportLayout->addRow("", exportHelp);
    
    m_exportGroup->setLayout(exportLayout);
    mainLayout->addWidget(m_exportGroup);
    
    // Buttons with enhanced styling
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    
    m_startButton = new QPushButton("▶ Start Scan", this);
    m_startButton->setStyleSheet(
        "QPushButton { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #60a5fa, stop: 1 #4d8fef); color: white; font-weight: bold; padding: 12px 24px; }"
        "QPushButton:hover { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #70b0ff, stop: 1 #5d9fff); }"
        "QPushButton:pressed { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #5080e0, stop: 1 #4080df); }"
    );
    
    m_stopButton = new QPushButton("⏹ Stop Scan", this);
    m_stopButton->setEnabled(false);
    m_stopButton->setStyleSheet(
        "QPushButton { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #ef4444, stop: 1 #dc2626); color: white; font-weight: bold; padding: 12px 24px; }"
        "QPushButton:hover { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #ff5555, stop: 1 #ec3535); }"
        "QPushButton:pressed { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #dd3333, stop: 1 #cc2424); }"
        "QPushButton:disabled { background: #2a2a2f; color: #7f7f7f; }"
    );
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_startButton, &QPushButton::clicked, this, &ScanConfiguration::onStartScan);
    connect(m_stopButton, &QPushButton::clicked, this, &ScanConfiguration::onStopScan);
    connect(m_scanModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ScanConfiguration::onScanModeChanged);
    connect(m_browseButton, &QPushButton::clicked, this, &ScanConfiguration::onBrowseFile);
    
    // Initial visibility update
    updateFileBrowseVisibility();
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

QString ScanConfiguration::profile() const
{
    return m_profileCombo->currentData().toString();
}

bool ScanConfiguration::deepScan() const
{
    return m_deepScanCheck->isChecked();
}

bool ScanConfiguration::vulnScan() const
{
    return m_vulnScanCheck->isChecked();
}

bool ScanConfiguration::serviceVersion() const
{
    return m_serviceVersionCheck->isChecked();
}

bool ScanConfiguration::deviceIdentification() const
{
    return m_deviceIdentificationCheck->isChecked();
}

bool ScanConfiguration::monitoringMode() const
{
    return m_monitoringModeCheck->isChecked();
}

int ScanConfiguration::monitoringInterval() const
{
    return m_monitoringIntervalSpin->value();
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
    
    // Update browse button visibility
    updateFileBrowseVisibility();
}

void ScanConfiguration::onBrowseFile()
{
    // Open file dialog for text files
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select IP Address File"),
        QString(),
        tr("Text Files (*.txt);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        m_targetEdit->setText(fileName);
    }
}

void ScanConfiguration::updateFileBrowseVisibility()
{
    // Show browse button only for import mode
    QString mode = m_scanModeCombo->currentData().toString();
    m_browseButton->setVisible(mode == "import");
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

