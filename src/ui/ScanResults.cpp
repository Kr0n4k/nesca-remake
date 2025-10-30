#include "ScanResults.h"
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>

ScanResults::ScanResults(QWidget *parent)
    : QWidget(parent)
    , m_resultsTable(nullptr)
    , m_detailsTree(nullptr)
    , m_splitter(nullptr)
    , m_exportButton(nullptr)
    , m_clearButton(nullptr)
    , m_statsLabel(nullptr)
    , m_cameraCount(0)
    , m_authCount(0)
    , m_ftpCount(0)
    , m_sshCount(0)
    , m_otherCount(0)
{
    setupUI();
}

void ScanResults::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Results info banner
    QLabel *resultsInfo = new QLabel(
        "📋 <b>Results Table:</b> Double-click any row to view detailed information | "
        "<b>Export:</b> Save results to JSON/CSV | <b>Clear:</b> Remove all results",
        this
    );
    resultsInfo->setStyleSheet(
        "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #1a3a5a, stop: 1 #2a4a6a);"
        "color: #a0d0ff;"
        "padding: 8px;"
        "border-radius: 5px;"
        "margin-bottom: 8px;"
        "font-size: 11px;"
    );
    resultsInfo->setWordWrap(true);
    mainLayout->addWidget(resultsInfo);
    
    // Create splitter
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // Create results table
    m_resultsTable = new QTableWidget(this);
    QStringList headers;
    headers << "IP" << "Port" << "Type" << "Timestamp" << "Data";
    m_resultsTable->setColumnCount(headers.size());
    m_resultsTable->setHorizontalHeaderLabels(headers);
    m_resultsTable->setAlternatingRowColors(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->verticalHeader()->hide();
    
    // Create details tree
    m_detailsTree = new QTreeWidget(this);
    m_detailsTree->setHeaderLabel("Details");
    m_detailsTree->setMaximumWidth(300);
    
    // Add widgets to splitter
    m_splitter->addWidget(m_resultsTable);
    m_splitter->addWidget(m_detailsTree);
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(m_splitter);
    
    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_statsLabel = new QLabel("Found: 0 results", this);
    buttonLayout->addWidget(m_statsLabel);
    
    buttonLayout->addStretch();
    
    m_exportButton = new QPushButton("Export Results", this);
    m_clearButton = new QPushButton("Clear Results", this);
    
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_clearButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_resultsTable, &QTableWidget::itemDoubleClicked, 
            this, [this](QTableWidgetItem* item) {
                this->onResultDoubleClicked(item->row(), item->column());
            });
    connect(m_exportButton, &QPushButton::clicked, this, &ScanResults::onExport);
    connect(m_clearButton, &QPushButton::clicked, this, &ScanResults::onClear);
}

void ScanResults::addResult(const ScanResult &result)
{
    m_results.append(result);
    
    int row = m_resultsTable->rowCount();
    m_resultsTable->insertRow(row);
    
    // Add data
    m_resultsTable->setItem(row, 0, new QTableWidgetItem(result.ip));
    m_resultsTable->setItem(row, 1, new QTableWidgetItem(QString::number(result.port)));
    m_resultsTable->setItem(row, 2, new QTableWidgetItem(result.type));
    m_resultsTable->setItem(row, 3, new QTableWidgetItem(result.timestamp));
    m_resultsTable->setItem(row, 4, new QTableWidgetItem(result.data));
    
    // Update statistics
    if (result.type == "camera") m_cameraCount++;
    else if (result.type == "auth") m_authCount++;
    else if (result.type == "ftp") m_ftpCount++;
    else if (result.type == "ssh") m_sshCount++;
    else m_otherCount++;
    
    // Update statistics display
    updateStats();
}

void ScanResults::clearResults()
{
    m_results.clear();
    m_resultsTable->setRowCount(0);
    m_cameraCount = m_authCount = m_ftpCount = m_sshCount = m_otherCount = 0;
    m_statsLabel->setText("Found: 0 results");
}

void ScanResults::updateStatistics()
{
    updateStats();
}

void ScanResults::onExport()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Results",
        "", "JSON Files (*.json);;CSV Files (*.csv);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Emit signal for export (actual export will be handled by result exporter)
    emit exportRequested(fileName);
    
    QMessageBox::information(this, "Export", 
        QString("Results exported to %1").arg(fileName));
}

void ScanResults::onClear()
{
    clearResults();
}

void ScanResults::onResultDoubleClicked(int row, int column)
{
    if (row >= 0 && row < m_results.size()) {
        const ScanResult &result = m_results[row];
        
        // Clear tree
        m_detailsTree->clear();
        
        // Add details
        QTreeWidgetItem *ipItem = new QTreeWidgetItem(m_detailsTree);
        ipItem->setText(0, "IP: " + result.ip);
        
        QTreeWidgetItem *portItem = new QTreeWidgetItem(m_detailsTree);
        portItem->setText(0, "Port: " + QString::number(result.port));
        
        QTreeWidgetItem *typeItem = new QTreeWidgetItem(m_detailsTree);
        typeItem->setText(0, "Type: " + result.type);
        
        QTreeWidgetItem *timestampItem = new QTreeWidgetItem(m_detailsTree);
        timestampItem->setText(0, "Timestamp: " + result.timestamp);
        
        QTreeWidgetItem *dataItem = new QTreeWidgetItem(m_detailsTree);
        dataItem->setText(0, "Data");
        dataItem->addChild(new QTreeWidgetItem(dataItem));
        dataItem->child(0)->setText(0, result.data);
        
        if (!result.login.isEmpty()) {
            QTreeWidgetItem *loginItem = new QTreeWidgetItem(m_detailsTree);
            loginItem->setText(0, "Login: " + result.login);
        }
        
        if (!result.password.isEmpty()) {
            QTreeWidgetItem *passItem = new QTreeWidgetItem(m_detailsTree);
            passItem->setText(0, "Password: " + result.password);
        }
        
        // Expand all
        m_detailsTree->expandAll();
    }
}

void ScanResults::updateStats()
{
    QString stats = QString("Found: %1 results")
        .arg(m_results.size());
    
    QString typeStats;
    if (m_cameraCount > 0) typeStats += QString(" Cameras: %1").arg(m_cameraCount);
    if (m_authCount > 0) typeStats += QString(" Auth: %1").arg(m_authCount);
    if (m_ftpCount > 0) typeStats += QString(" FTP: %1").arg(m_ftpCount);
    if (m_sshCount > 0) typeStats += QString(" SSH: %1").arg(m_sshCount);
    if (m_otherCount > 0) typeStats += QString(" Other: %1").arg(m_otherCount);
    
    if (!typeStats.isEmpty()) {
        stats += " |" + typeStats;
    }
    
    m_statsLabel->setText(stats);
}

void ScanResults::updateStatisticsDisplay()
{
    updateStats();
}

