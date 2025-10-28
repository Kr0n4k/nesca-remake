#!/bin/bash
# Script to reorganize project into modular structure

cd "$(dirname "$0")"

echo "Reorganizing project into modular structure..."

# Core modules (scanning engine)
mv MainStarter.cpp MainStarter.h src/core/ 2>/dev/null || true
mv Threader.cpp Threader.h src/core/ 2>/dev/null || true
mv STh.cpp STh.h src/core/ 2>/dev/null || true
mv finder.cpp src/core/ 2>/dev/null || true

# Detectors
mv DeviceIdentifier.cpp DeviceIdentifier.h src/detectors/ 2>/dev/null || true
mv FingerprintScanner.cpp FingerprintScanner.h src/detectors/ 2>/dev/null || true

# Authentication modules
mv BasicAuth.cpp BasicAuth.h src/auth/ 2>/dev/null || true
mv FTPAuth.cpp FTPAuth.h src/auth/ 2>/dev/null || true
mv SSHAuth.cpp SSHAuth.h src/auth/ 2>/dev/null || true
mv RTSP.cpp RTSP.h src/auth/ 2>/dev/null || true
mv IPCAuth.cpp IPCAuth.h src/auth/ 2>/dev/null || true
mv HikvisionLogin.cpp HikvisionLogin.h src/auth/ 2>/dev/null || true
mv WebformWorker.cpp WebformWorker.h src/auth/ 2>/dev/null || true

# Exporters
mv ResultExporter.cpp ResultExporter.h src/exporters/ 2>/dev/null || true

# Network utilities
mv Connector.cpp Connector.h src/network/ 2>/dev/null || true
mv Utils.cpp Utils.h src/network/ 2>/dev/null || true
mv BruteUtils.cpp BruteUtils.h src/network/ 2>/dev/null || true
mv AdaptiveScanner.cpp AdaptiveScanner.h src/network/ 2>/dev/null || true
mv FileDownloader.cpp FileDownloader.h src/network/ 2>/dev/null || true
mv FileUpdater.cpp FileUpdater.h src/network/ 2>/dev/null || true
mv IPRandomizer.cpp IPRandomizer.h src/network/ 2>/dev/null || true
mv base64.cpp base64.h src/network/ 2>/dev/null || true
mv ProgressMonitor.cpp ProgressMonitor.h src/network/ 2>/dev/null || true

# UI components
mv nesca_3.cpp nesca_3.h nesca_3.ui nesca_3.qrc nesca_3.pro src/ui/ 2>/dev/null || true
mv DrawerTh_*.cpp DrawerTh_*.h src/ui/ 2>/dev/null || true
mv ActivityDrawerTh_*.cpp ActivityDrawerTh_*.h src/ui/ 2>/dev/null || true
mv CheckKey_Th.cpp CheckKey_Th.h src/ui/ 2>/dev/null || true
mv progressbardrawer.cpp progressbardrawer.h src/ui/ 2>/dev/null || true
mv piestat.cpp piestat.h src/ui/ 2>/dev/null || true
mv msgcheckerthread.cpp msgcheckerthread.h src/ui/ 2>/dev/null || true
mv vercheckerthread.cpp vercheckerthread.h src/ui/ 2>/dev/null || true

# Common headers and resources
mv externData.h externFunctions.h mainResources.h resource.h src/common/ 2>/dev/null || true

echo "Reorganization complete!"
echo "Note: You may need to update include paths in source files."

