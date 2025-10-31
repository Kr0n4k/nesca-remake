#-------------------------------------------------
#
# Nesca v2r - GUI Version
# Project for GUI application
#
#-------------------------------------------------

QT       += core network gui widgets

# QTextCodec support (needed for finder.cpp)
exists($$[QT_INSTALL_LIBS]/libQt5Core5Compat.so) {
    QT += core5compat
}

QMAKE_CFLAGS += -Wno-write-strings -g
QMAKE_CXXFLAGS += -std=c++17 -Wno-write-strings -Wno-narrowing -fpermissive -g

TARGET = nesca-gui
TEMPLATE = app

# Include paths for modular structure
INCLUDEPATH += $$PWD/src/common \
               $$PWD/src/core \
               $$PWD/src/detectors \
               $$PWD/src/auth \
               $$PWD/src/exporters \
               $$PWD/src/network \
               $$PWD/src/ui

# Common headers (shared across modules)
HEADERS += src/common/externData.h \
           src/common/externFunctions.h \
           src/common/mainResources.h \
           src/common/resource.h

# Main GUI entry point
SOURCES += main_gui.cpp

# Core scanning engine
SOURCES += src/core/MainStarter.cpp \
           src/core/Threader.cpp \
           src/core/STh.cpp \
           src/core/finder.cpp

HEADERS += src/core/MainStarter.h \
           src/core/Threader.h \
           src/core/STh.h

# Device detectors
SOURCES += src/detectors/DeviceIdentifier.cpp \
           src/detectors/FingerprintScanner.cpp

HEADERS += src/detectors/DeviceIdentifier.h \
           src/detectors/FingerprintScanner.h

# Authentication modules
SOURCES += src/auth/BasicAuth.cpp \
           src/auth/FTPAuth.cpp \
           src/auth/SSHAuth.cpp \
           src/auth/RTSP.cpp \
           src/auth/IPCAuth.cpp \
           src/auth/HikvisionLogin.cpp \
           src/auth/WebformWorker.cpp \
           src/auth/WebSocketAuth.cpp \
           src/auth/QuicAuth.cpp \
           src/auth/ShodanAuth.cpp

HEADERS += src/auth/BasicAuth.h \
           src/auth/FTPAuth.h \
           src/auth/SSHAuth.h \
           src/auth/RTSP.h \
           src/auth/IPCAuth.h \
           src/auth/HikvisionLogin.h \
           src/auth/WebformWorker.h \
           src/auth/WebSocketAuth.h \
           src/auth/QuicAuth.h \
           src/auth/ShodanAuth.h

# Exporters
SOURCES += src/exporters/ResultExporter.cpp

HEADERS += src/exporters/ResultExporter.h

# Network utilities
SOURCES += src/network/Connector.cpp \
           src/network/Utils.cpp \
           src/network/BruteUtils.cpp \
           src/network/AdaptiveScanner.cpp \
           src/network/FileDownloader.cpp \
           src/network/FileUpdater.cpp \
           src/network/IPRandomizer.cpp \
           src/network/base64.cpp \
           src/network/ProgressMonitor.cpp \
           src/network/ConfigManager.cpp \
           src/network/NetworkMonitor.cpp \
           src/ui/InteractiveMode.cpp \
           src/detectors/DeepScanner.cpp \
           src/detectors/VulnerabilityScanner.cpp \
           src/detectors/ServiceVersionDetector.cpp

HEADERS += src/network/Connector.h \
           src/network/Utils.h \
           src/network/BruteUtils.h \
           src/network/AdaptiveScanner.h \
           src/network/FileDownloader.h \
           src/network/FileUpdater.h \
           src/network/IPRandomizer.h \
           src/network/base64.h \
           src/network/ProgressMonitor.h \
           src/network/ConfigManager.h \
           src/network/NetworkMonitor.h \
           src/ui/InteractiveMode.h \
           src/detectors/DeepScanner.h \
           src/detectors/VulnerabilityScanner.h \
           src/detectors/ServiceVersionDetector.h

# GUI components
SOURCES += src/ui/MainWindow.cpp \
           src/ui/ScanConfiguration.cpp \
           src/ui/ScanResults.cpp \
           src/ui/NetworkScanner.cpp

HEADERS += src/ui/MainWindow.h \
           src/ui/ScanConfiguration.h \
           src/ui/ScanResults.h \
           src/ui/NetworkScanner.h

OTHER_FILES += \
    nesca_3.rc

unix|win32: LIBS += -lssh
unix|win32: LIBS += -lcrypto
unix|win32: LIBS += -lcurl
unix|win32: LIBS += -lnghttp2
unix|win32: LIBS += -lm
win32: LIBS += -lws2_32

