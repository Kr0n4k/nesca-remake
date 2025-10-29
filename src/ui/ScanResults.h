#ifndef SCANRESULTS_H
#define SCANRESULTS_H

#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAbstractItemView>

struct ScanResult {
    QString ip;
    int port;
    QString type;  // camera, auth, ftp, ssh, other
    QString timestamp;
    QString data;
    QString login;
    QString password;
};

class ScanResults : public QWidget
{
    Q_OBJECT

public:
    explicit ScanResults(QWidget *parent = nullptr);
    
    void addResult(const ScanResult &result);
    void clearResults();
    void updateStatistics();
    
    int getResultCount() const { return m_results.size(); }

signals:
    void exportRequested(const QString &format);

private slots:
    void onExport();
    void onClear();
    void onResultDoubleClicked(int row, int column);
    void updateStats();

private:
    void setupUI();
    void updateStatisticsDisplay();
    
    QTableWidget *m_resultsTable;
    QTreeWidget *m_detailsTree;
    QSplitter *m_splitter;
    
    QPushButton *m_exportButton;
    QPushButton *m_clearButton;
    
    QLabel *m_statsLabel;
    
    QList<ScanResult> m_results;
    
    // Statistics
    int m_cameraCount;
    int m_authCount;
    int m_ftpCount;
    int m_sshCount;
    int m_otherCount;
};

#endif // SCANRESULTS_H

