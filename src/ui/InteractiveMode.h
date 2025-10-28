#ifndef INTERACTIVEMODE_H
#define INTERACTIVEMODE_H

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QCoreApplication>

struct ScanPreset {
    QString name;
    QString description;
    QString ports;
    QStringList additionalArgs;
    int threads;
    bool adaptive;
    bool smartScan;
};

class InteractiveMode {
private:
    static QStringList mainMenuOptions;
    static QMap<QString, ScanPreset> quickPresets;
    static void initializePresets();
    
    static void printBanner();
    static void printMenu(const QStringList& options, const QString& title = "Menu");
    static int getChoice(int maxOption);
    static QString getInput(const QString& prompt);
    static bool getYesNo(const QString& prompt, bool defaultValue = true);
    
public:
    InteractiveMode();
    
    // Run interactive mode
    static int run(QCoreApplication& app, int argc, char *argv[]);
    
    // Quick presets
    static bool runQuickPreset(const QString& presetName, QCoreApplication& app);
    
    // Color output utilities (can be used standalone)
    static void enableColorOutput(bool enable) { colorEnabled = enable; }
    static bool isColorEnabled() { return colorEnabled; }
    static QString colorize(const QString& text, const QString& color);
    static void printProgressBar(int current, int total, int width = 50);
    static void printStatus(const QString& message, const QString& status = "info");
    
private:
    static bool colorEnabled;
    static int scanModeMenu();
    static int scanSettingsMenu(QString& ports, int& threads, bool& adaptive, bool& smartScan);
    static int exportSettingsMenu(QString& format, bool& autoExport);
    static void executeScan(const QString& mode, const QString& target, 
                           const QString& ports, int threads, bool adaptive, 
                           bool smartScan, const QString& exportFormat);
};

#endif // INTERACTIVEMODE_H

