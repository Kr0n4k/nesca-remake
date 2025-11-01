#-------------------------------------------------
#
# Modular Project Structure
# Project created by QtCreator 2015-02-24T13:25:33
#
#-------------------------------------------------

QT       += core network

# QTextCodec support (needed for finder.cpp)
# In Qt 5.15+ QTextCodec moved to core5compat
exists($$[QT_INSTALL_LIBS]/libQt5Core5Compat.so) {
    QT += core5compat
}

# Removed gui, multimedia, widgets for console-only application

QMAKE_CFLAGS += -Wno-write-strings -g
QMAKE_CXXFLAGS += -std=c++17 -Wno-write-strings -Wno-narrowing -fpermissive -g

DEFINES += USE_ASIO_CONNECTOR

TARGET = nesca
TEMPLATE = app

# Include paths for modular structure
INCLUDEPATH += $$PWD/src/common \
               $$PWD/src/core \
               $$PWD/src/detectors \
               $$PWD/src/auth \
               $$PWD/src/exporters \
               $$PWD/src/network \
               $$PWD/src/ui \
               /usr/include

# Common headers (shared across modules)
HEADERS += src/common/externData.h \
           src/common/externFunctions.h \
           src/common/mainResources.h \
           src/common/resource.h

# Main entry point (stays in root)
SOURCES += main.cpp

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
           src/auth/ShodanAuth.cpp \
           src/auth/CensysAuth.cpp

HEADERS += src/auth/BasicAuth.h \
           src/auth/FTPAuth.h \
           src/auth/SSHAuth.h \
           src/auth/RTSP.h \
           src/auth/IPCAuth.h \
           src/auth/HikvisionLogin.h \
           src/auth/WebformWorker.h \
           src/auth/WebSocketAuth.h \
           src/auth/QuicAuth.h \
           src/auth/ShodanAuth.h \
           src/auth/CensysAuth.h

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
           src/detectors/ServiceVersionDetector.cpp \
           src/network/AsyncConnector.cpp
# Boost 1.89+ incompatible
#           src/network/AsyncHttpClient.cpp \
#           src/network/DistServer.cpp

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
           src/detectors/ServiceVersionDetector.h \
           src/network/AsyncConnector.h
# Boost 1.89+ incompatible
#           src/network/AsyncHttpClient.h \
#           src/network/DistServer.h

# UI components (optional, for GUI build)
# Uncomment for GUI version:
#SOURCES += src/ui/nesca_3.cpp \
#           src/ui/ActivityDrawerTh_HorNet.cpp \
#           src/ui/CheckKey_Th.cpp \
#           src/ui/DrawerTh_GridQoSScanner.cpp \
#           src/ui/DrawerTh_HorNet.cpp \
#           src/ui/DrawerTh_ME2Scanner.cpp \
#           src/ui/DrawerTh_QoSScanner.cpp \
#           src/ui/DrawerTh_VoiceScanner.cpp \
#           src/ui/msgcheckerthread.cpp \
#           src/ui/piestat.cpp \
#           src/ui/progressbardrawer.cpp \
#           src/ui/vercheckerthread.cpp

#HEADERS += src/ui/nesca_3.h \
#           src/ui/ActivityDrawerTh_HorNet.h \
#           src/ui/CheckKey_Th.h \
#           src/ui/DrawerTh_GridQoSScanner.h \
#           src/ui/DrawerTh_HorNet.h \
#           src/ui/DrawerTh_ME2Scanner.h \
#           src/ui/DrawerTh_QoSScanner.h \
#           src/ui/DrawerTh_VoiceScanner.h \
#           src/ui/msgcheckerthread.h \
#           src/ui/piestat.h \
#           src/ui/progressbardrawer.h \
#           src/ui/vercheckerthread.h

#FORMS += src/ui/nesca_3.ui
#RESOURCES += src/ui/nesca_3.qrc

# Alternative console entry point (commented out - use main.cpp instead)
#SOURCES += main_console.cpp

OTHER_FILES += \
    nesca_3.rc

unix|win32: LIBS += -lssh
unix|win32: LIBS += -lcrypto
unix|win32: LIBS += -lssl
unix|win32: LIBS += -lcurl
unix|win32: LIBS += -lnghttp2
unix|win32: LIBS += -lm
# Boost 1.89 is header-only for asio/beast
# unix|win32: LIBS += -lboost_system
win32: LIBS += -lws2_32

