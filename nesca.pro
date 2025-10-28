#-------------------------------------------------
#
# Project created by QtCreator 2015-02-24T13:25:33
#
#-------------------------------------------------

QT       += core network

# QTextCodec support (needed for finder.cpp)
# In Qt 5.15+ QTextCodec moved to core5compat
# Try adding core5compat if available (for newer Qt), otherwise it should be in core
exists($$[QT_INSTALL_LIBS]/libQt5Core5Compat.so) {
    QT += core5compat
}

# Removed gui, multimedia, widgets for console-only application

CONFIG += c++11
QMAKE_CFLAGS += -Wno-write-strings -g
QMAKE_CXXFLAGS += -Wno-write-strings -Wno-narrowing -fpermissive -g

TARGET = nesca
TEMPLATE = app
#INCLUDEPATH += /opt/Qt5.3.2/5.3/gcc_64/include/QtWidgets/
SOURCES +=\
        main.cpp \
    base64.cpp \
    STh.cpp \
    finder.cpp \
    WebformWorker.cpp \
    Connector.cpp \
    Utils.cpp \
    BruteUtils.cpp \
    BasicAuth.cpp \
    FTPAuth.cpp \
    Threader.cpp \
    SSHAuth.cpp \
    FileUpdater.cpp \
    FileDownloader.cpp \
    MainStarter.cpp \
    IPRandomizer.cpp \
    HikvisionLogin.cpp \
    RTSP.cpp \
    IPCAuth.cpp

# GUI files excluded for console build:
#    nesca_3.cpp \
#    ActivityDrawerTh_HorNet.cpp \
#    CheckKey_Th.cpp \
#    DrawerTh_GridQoSScanner.cpp \
#    DrawerTh_HorNet.cpp \
#    DrawerTh_ME2Scanner.cpp \
#    DrawerTh_QoSScanner.cpp \
#    DrawerTh_VoiceScanner.cpp \
#    msgcheckerthread.cpp \
#    piestat.cpp \
#    progressbardrawer.cpp \
#    vercheckerthread.cpp


HEADERS  += base64.h \
    externData.h \
    externFunctions.h \
    mainResources.h \
    resource.h \
    STh.h \
    Utils.h \
    WebformWorker.h \
    Connector.h \
    BasicAuth.h \
    BruteUtils.h \
    FTPAuth.h \
    Threader.h \
    SSHAuth.h \
    FileUpdater.h \
    FileDownloader.h \
    MainStarter.h \
    IPRandomizer.h \
    HikvisionLogin.h \
    RTSP.h \
    IPCAuth.h

# GUI headers excluded for console build:
#    nesca_3.h \
#    ActivityDrawerTh_HorNet.h \
#    CheckKey_Th.h \
#    DrawerTh_GridQoSScanner.h \
#    DrawerTh_HorNet.h \
#    DrawerTh_ME2Scanner.h \
#    DrawerTh_QoSScanner.h \
#    DrawerTh_VoiceScanner.h \
#    msgcheckerthread.h \
#    piestat.h \
#    progressbardrawer.h \
#    vercheckerthread.h

# FORMS removed - no GUI forms needed for console application
# FORMS    += nesca_3.ui

# RESOURCES removed - UI resources not needed for console
# RESOURCES += \
#     nesca_3.qrc

OTHER_FILES += \
    nesca_3.rc


unix|win32: LIBS += -lssh

unix|win32: LIBS += -lcrypto

unix|win32: LIBS += -lcurl

unix|win32: LIBS += -lm

win32: LIBS += -lws2_32
