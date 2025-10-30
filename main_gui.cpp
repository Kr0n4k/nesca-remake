#include <QApplication>
#include "src/ui/MainWindow.h"
#include <QStyleFactory>
#include <QDir>
#include <QFile>

void applyModernStyle(QApplication &app)
{
    // Set Fusion style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Enhanced dark palette with modern colors
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(28, 28, 33));
    darkPalette.setColor(QPalette::WindowText, QColor(240, 240, 240));
    darkPalette.setColor(QPalette::Base, QColor(23, 23, 28));
    darkPalette.setColor(QPalette::AlternateBase, QColor(33, 33, 38));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(45, 45, 50));
    darkPalette.setColor(QPalette::ToolTipText, QColor(240, 240, 240));
    darkPalette.setColor(QPalette::Text, QColor(240, 240, 240));
    darkPalette.setColor(QPalette::Button, QColor(43, 43, 48));
    darkPalette.setColor(QPalette::ButtonText, QColor(240, 240, 240));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 100, 100));
    darkPalette.setColor(QPalette::Link, QColor(96, 165, 250));
    darkPalette.setColor(QPalette::Highlight, QColor(96, 165, 250));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    
    app.setPalette(darkPalette);
    
    // Apply modern stylesheet with enhanced colors and effects
    QString style = R"(
        /* Main window */
        QMainWindow {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #1c1c21, stop: 1 #16161a);
        }
        
        /* Tab widget */
        QTabWidget::pane {
            border: 1px solid #3a3a3f;
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #242429, stop: 1 #202024);
            border-radius: 6px;
        }
        
        QTabBar::tab {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b30, stop: 1 #252529);
            color: #d0d0d0;
            padding: 10px 22px;
            margin-right: 3px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            border: 1px solid #3a3a3f;
            border-bottom: none;
            min-width: 80px;
        }
        
        QTabBar::tab:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #242429, stop: 1 #202024);
            color: #60a5fa;
            border-bottom: 3px solid #60a5fa;
            border-color: #3a3a3f #3a3a3f #60a5fa #3a3a3f;
        }
        
        QTabBar::tab:hover:!selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #323237, stop: 1 #2d2d31);
        }
        
        /* Buttons */
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #3c3c41, stop: 1 #36363b);
            color: #e8e8e8;
            border: 1px solid #4a4a4f;
            border-radius: 5px;
            padding: 8px 18px;
            font-weight: 600;
            min-height: 24px;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #4c4c51, stop: 1 #46464b);
            border-color: #5a5a5f;
        }
        
        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2c2c31, stop: 1 #26262b);
            border-color: #3a3a3f;
        }
        
        QPushButton:disabled {
            background: #2a2a2f;
            color: #7f7f7f;
            border-color: #3a3a3f;
        }
        
        /* Line edits */
        QLineEdit {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b2f, stop: 1 #27272b);
            color: #e8e8e8;
            border: 1px solid #3a3a3f;
            border-radius: 5px;
            padding: 8px 12px;
            selection-background-color: #60a5fa;
            selection-color: white;
        }
        
        QLineEdit:focus {
            border: 2px solid #60a5fa;
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2e2e32, stop: 1 #2a2a2e);
        }
        
        QLineEdit:disabled {
            background: #25252a;
            color: #7f7f7f;
        }
        
        /* Spin boxes */
        QSpinBox {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b2f, stop: 1 #27272b);
            color: #e8e8e8;
            border: 1px solid #3a3a3f;
            border-radius: 5px;
            padding: 6px 10px;
            selection-background-color: #60a5fa;
        }
        
        QSpinBox:focus {
            border: 2px solid #60a5fa;
        }
        
        QSpinBox:disabled {
            background: #25252a;
            color: #7f7f7f;
        }
        
        /* Combo boxes */
        QComboBox {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b2f, stop: 1 #27272b);
            color: #e8e8e8;
            border: 1px solid #3a3a3f;
            border-radius: 5px;
            padding: 8px 12px;
            selection-background-color: #60a5fa;
            min-height: 24px;
        }
        
        QComboBox:hover {
            border-color: #4a4a4f;
        }
        
        QComboBox:focus {
            border: 2px solid #60a5fa;
        }
        
        QComboBox:disabled {
            background: #25252a;
            color: #7f7f7f;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 24px;
            background: transparent;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #d0d0d0;
            width: 0;
            height: 0;
        }
        
        /* Checkboxes */
        QCheckBox {
            color: #e8e8e8;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 20px;
            height: 20px;
            border: 2px solid #4a4a4f;
            border-radius: 4px;
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b2f, stop: 1 #27272b);
        }
        
        QCheckBox::indicator:hover {
            border-color: #60a5fa;
        }
        
        QCheckBox::indicator:checked {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #60a5fa, stop: 1 #4d8fef);
            border-color: #60a5fa;
        }
        
        
        /* Group boxes */
        QGroupBox {
            color: #e8e8e8;
            border: 1px solid #3a3a3f;
            border-radius: 8px;
            margin-top: 16px;
            padding-top: 16px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 0 12px;
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #242429, stop: 1 #202024);
            font-weight: 600;
        }
        
        /* Tables */
        QTableWidget {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #242429, stop: 1 #202024);
            alternate-background-color: #29292e;
            color: #e8e8e8;
            gridline-color: #3a3a3f;
            border: 1px solid #3a3a3f;
            border-radius: 6px;
        }
        
        QTableWidget::item {
            padding: 6px;
        }
        
        QTableWidget::item:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #60a5fa, stop: 1 #4d8fef);
            color: white;
        }
        
        QHeaderView::section {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b2f, stop: 1 #27272b);
            color: #d8d8d8;
            padding: 8px;
            border: none;
            border-right: 1px solid #3a3a3f;
            border-bottom: 1px solid #3a3a3f;
            font-weight: 700;
        }
        
        /* Progress bar */
        QProgressBar {
            border: 1px solid #3a3a3f;
            border-radius: 6px;
            text-align: center;
            color: #e8e8e8;
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b2f, stop: 1 #27272b);
            height: 20px;
        }
        
        QProgressBar::chunk {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #60a5fa, stop: 0.5 #4d8fef, stop: 1 #4090e8);
            border-radius: 5px;
        }
        
        /* Status bar */
        QStatusBar {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #242429, stop: 1 #202024);
            color: #d8d8d8;
            border-top: 2px solid #3a3a3f;
        }
        
        QStatusBar::item {
            border: none;
        }
        
        /* Labels */
        QLabel {
            color: #e8e8e8;
        }
        
        /* Scroll bars */
        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 14px;
            margin: 0;
        }
        
        QScrollBar::handle:vertical {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #4a4a4f, stop: 1 #46464b);
            border-radius: 7px;
            min-height: 30px;
            margin: 2px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #5a5a5f, stop: 1 #56565b);
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        
        QScrollBar:horizontal {
            border: none;
            background: transparent;
            height: 14px;
            margin: 0;
        }
        
        QScrollBar::handle:horizontal {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #4a4a4f, stop: 1 #46464b);
            border-radius: 7px;
            min-width: 30px;
            margin: 2px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #5a5a5f, stop: 1 #56565b);
        }
        
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }
        
        /* Tree widget */
        QTreeWidget {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #242429, stop: 1 #202024);
            alternate-background-color: #29292e;
            color: #e8e8e8;
            border: 1px solid #3a3a3f;
            border-radius: 6px;
        }
        
        QTreeWidget::item {
            padding: 6px;
        }
        
        QTreeWidget::item:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #60a5fa, stop: 1 #4d8fef);
            color: white;
        }
        
        /* Menu bar */
        QMenuBar {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #242429, stop: 1 #202024);
            color: #e8e8e8;
            border-bottom: 1px solid #3a3a3f;
        }
        
        QMenuBar::item {
            padding: 6px 12px;
            border-radius: 4px;
        }
        
        QMenuBar::item:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #323237, stop: 1 #2d2d31);
        }
        
        QMenu {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2b2b2f, stop: 1 #27272b);
            border: 1px solid #3a3a3f;
            border-radius: 6px;
        }
        
        QMenu::item {
            padding: 8px 20px;
            border-radius: 4px;
        }
        
        QMenu::item:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #60a5fa, stop: 1 #4d8fef);
            color: white;
        }
        
        QMenu::separator {
            background: #3a3a3f;
            height: 1px;
            margin: 4px 0px;
        }
    )";
    
    app.setStyleSheet(style);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Nesca");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("NESCA");
    
    // Apply modern dark theme
    applyModernStyle(app);
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}

