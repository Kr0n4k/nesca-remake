#ifndef SCANCONFIGURATION_H
#define SCANCONFIGURATION_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

class ScanConfiguration : public QWidget
{
    Q_OBJECT

public:
    explicit ScanConfiguration(QWidget *parent = nullptr);
    
    // Getters
    QString scanMode() const;
    QString target() const;
    QString ports() const;
    int threads() const;
    int timeout() const;
    bool verifySSL() const;
    bool adaptiveScan() const;
    bool smartScan() const;
    bool deepScan() const;
    bool vulnScan() const;
    bool serviceVersion() const;
    bool deviceIdentification() const;
    bool monitoringMode() const;
    int monitoringInterval() const;
    QString profile() const;
    QString exportFormat() const;
    
signals:
    void scanRequested();
    void scanStopped();

private slots:
    void onScanModeChanged(int index);
    void onBrowseFile();
    void onStartScan();
    void onStopScan();
    
private:
    void setupUI();
    void updateFileBrowseVisibility();
    
    // Profiles
    QComboBox *m_profileCombo;
    
    // Scan mode
    QComboBox *m_scanModeCombo;
    QLineEdit *m_targetEdit;
    QPushButton *m_browseButton;
    QLineEdit *m_portsEdit;
    QSpinBox *m_threadsSpin;
    
    // Advanced options
    QGroupBox *m_advancedGroup;
    QSpinBox *m_timeoutSpin;
    QCheckBox *m_verifySSLCheck;
    QCheckBox *m_adaptiveCheck;
    QCheckBox *m_smartScanCheck;
    QCheckBox *m_deepScanCheck;
    QCheckBox *m_vulnScanCheck;
    QCheckBox *m_serviceVersionCheck;
    QCheckBox *m_deviceIdentificationCheck;
    
    // Monitoring options
    QGroupBox *m_monitoringGroup;
    QCheckBox *m_monitoringModeCheck;
    QSpinBox *m_monitoringIntervalSpin;
    
    // Export options
    QGroupBox *m_exportGroup;
    QComboBox *m_exportFormatCombo;
    
    // Buttons
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    
    // Status
    bool m_isScanning;
};

#endif // SCANCONFIGURATION_H

