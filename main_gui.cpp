#include <QApplication>
#include "src/ui/MainWindow.h"
#include <QStyleFactory>
#include <QDir>
#include <QFile>

void applyModernStyle(QApplication &app)
{
    // Set Fusion style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Enhanced dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 35));
    darkPalette.setColor(QPalette::WindowText, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 30));
    darkPalette.setColor(QPalette::AlternateBase, QColor(35, 35, 40));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(40, 40, 45));
    darkPalette.setColor(QPalette::ToolTipText, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::Text, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 50));
    darkPalette.setColor(QPalette::ButtonText, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 100, 100));
    darkPalette.setColor(QPalette::Link, QColor(100, 150, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(100, 150, 255));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    
    app.setPalette(darkPalette);
    
    // Apply modern stylesheet
    QString style = R"(
        /* Main window */
        QMainWindow {
            background-color: #1e1e23;
        }
        
        /* Tab widget */
        QTabWidget::pane {
            border: 1px solid #3e3e43;
            background-color: #25252e;
            border-radius: 4px;
        }
        
        QTabBar::tab {
            background-color: #2d2d32;
            color: #d0d0d0;
            padding: 8px 20px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            border: 1px solid #3e3e43;
            border-bottom: none;
        }
        
        QTabBar::tab:selected {
            background-color: #25252e;
            color: #64a5ff;
            border-bottom: 2px solid #64a5ff;
        }
        
        QTabBar::tab:hover {
            background-color: #35353a;
        }
        
        /* Buttons */
        QPushButton {
            background-color: #3e3e43;
            color: #e0e0e0;
            border: 1px solid #4e4e53;
            border-radius: 4px;
            padding: 6px 16px;
            font-weight: 500;
        }
        
        QPushButton:hover {
            background-color: #4e4e53;
            border-color: #5e5e63;
        }
        
        QPushButton:pressed {
            background-color: #2e2e33;
        }
        
        QPushButton:disabled {
            background-color: #2a2a2f;
            color: #7f7f7f;
            border-color: #3a3a3f;
        }
        
        /* Line edits */
        QLineEdit {
            background-color: #2d2d32;
            color: #e0e0e0;
            border: 1px solid #3e3e43;
            border-radius: 4px;
            padding: 6px 10px;
            selection-background-color: #64a5ff;
            selection-color: white;
        }
        
        QLineEdit:focus {
            border-color: #64a5ff;
            background-color: #2f2f34;
        }
        
        QLineEdit:disabled {
            background-color: #25252a;
            color: #7f7f7f;
        }
        
        /* Spin boxes */
        QSpinBox {
            background-color: #2d2d32;
            color: #e0e0e0;
            border: 1px solid #3e3e43;
            border-radius: 4px;
            padding: 4px 8px;
            selection-background-color: #64a5ff;
        }
        
        QSpinBox:focus {
            border-color: #64a5ff;
        }
        
        QSpinBox:disabled {
            background-color: #25252a;
            color: #7f7f7f;
        }
        
        /* Combo boxes */
        QComboBox {
            background-color: #2d2d32;
            color: #e0e0e0;
            border: 1px solid #3e3e43;
            border-radius: 4px;
            padding: 6px 10px;
            selection-background-color: #64a5ff;
        }
        
        QComboBox:hover {
            border-color: #4e4e53;
        }
        
        QComboBox:focus {
            border-color: #64a5ff;
        }
        
        QComboBox:disabled {
            background-color: #25252a;
            color: #7f7f7f;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 20px;
            background-color: transparent;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #d0d0d0;
            width: 0;
            height: 0;
        }
        
        /* Checkboxes */
        QCheckBox {
            color: #e0e0e0;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #4e4e53;
            border-radius: 3px;
            background-color: #2d2d32;
        }
        
        QCheckBox::indicator:hover {
            border-color: #64a5ff;
        }
        
        QCheckBox::indicator:checked {
            background-color: #64a5ff;
            border-color: #64a5ff;
        }
        
        
        /* Group boxes */
        QGroupBox {
            color: #e0e0e0;
            border: 1px solid #3e3e43;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 12px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 8px;
            background-color: #25252e;
        }
        
        /* Tables */
        QTableWidget {
            background-color: #25252e;
            alternate-background-color: #2a2a2f;
            color: #e0e0e0;
            gridline-color: #3e3e43;
            border: 1px solid #3e3e43;
            border-radius: 4px;
        }
        
        QTableWidget::item {
            padding: 4px;
        }
        
        QTableWidget::item:selected {
            background-color: #64a5ff;
            color: white;
        }
        
        QHeaderView::section {
            background-color: #2d2d32;
            color: #d0d0d0;
            padding: 6px;
            border: none;
            border-right: 1px solid #3e3e43;
            border-bottom: 1px solid #3e3e43;
            font-weight: bold;
        }
        
        /* Progress bar */
        QProgressBar {
            border: 1px solid #3e3e43;
            border-radius: 4px;
            text-align: center;
            color: #e0e0e0;
            background-color: #2d2d32;
        }
        
        QProgressBar::chunk {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #64a5ff, stop: 1 #5090e0);
            border-radius: 3px;
        }
        
        /* Status bar */
        QStatusBar {
            background-color: #25252e;
            color: #d0d0d0;
            border-top: 1px solid #3e3e43;
        }
        
        /* Labels */
        QLabel {
            color: #e0e0e0;
        }
        
        /* Scroll bars */
        QScrollBar:vertical {
            border: none;
            background-color: #25252e;
            width: 12px;
            margin: 0;
        }
        
        QScrollBar::handle:vertical {
            background-color: #4e4e53;
            border-radius: 6px;
            min-height: 20px;
            margin: 2px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #5e5e63;
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        
        QScrollBar:horizontal {
            border: none;
            background-color: #25252e;
            height: 12px;
            margin: 0;
        }
        
        QScrollBar::handle:horizontal {
            background-color: #4e4e53;
            border-radius: 6px;
            min-width: 20px;
            margin: 2px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background-color: #5e5e63;
        }
        
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }
        
        /* Tree widget */
        QTreeWidget {
            background-color: #25252e;
            alternate-background-color: #2a2a2f;
            color: #e0e0e0;
            border: 1px solid #3e3e43;
            border-radius: 4px;
        }
        
        QTreeWidget::item {
            padding: 4px;
        }
        
        QTreeWidget::item:selected {
            background-color: #64a5ff;
            color: white;
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

