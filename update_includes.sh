#!/bin/bash
# Script to update include paths for modular structure

cd "$(dirname "$0")"

echo "Updating include paths for modular structure..."

# Function to update includes in a file
update_includes() {
    local file="$1"
    if [ ! -f "$file" ]; then return; fi
    
    # Core headers
    sed -i 's|#include "MainStarter\.h"|#include "core/MainStarter.h"|g' "$file"
    sed -i 's|#include <MainStarter\.h>|#include <core/MainStarter.h>|g' "$file"
    sed -i 's|#include "Threader\.h"|#include "core/Threader.h"|g' "$file"
    sed -i 's|#include <Threader\.h>|#include <core/Threader.h>|g' "$file"
    sed -i 's|#include "STh\.h"|#include "core/STh.h"|g' "$file"
    sed -i 's|#include <STh\.h>|#include <core/STh.h>|g' "$file"
    
    # Detectors
    sed -i 's|#include "DeviceIdentifier\.h"|#include "detectors/DeviceIdentifier.h"|g' "$file"
    sed -i 's|#include <DeviceIdentifier\.h>|#include <detectors/DeviceIdentifier.h>|g' "$file"
    sed -i 's|#include "FingerprintScanner\.h"|#include "detectors/FingerprintScanner.h"|g' "$file"
    sed -i 's|#include <FingerprintScanner\.h>|#include <detectors/FingerprintScanner.h>|g' "$file"
    
    # Auth modules
    sed -i 's|#include "BasicAuth\.h"|#include "auth/BasicAuth.h"|g' "$file"
    sed -i 's|#include <BasicAuth\.h>|#include <auth/BasicAuth.h>|g' "$file"
    sed -i 's|#include "FTPAuth\.h"|#include "auth/FTPAuth.h"|g' "$file"
    sed -i 's|#include <FTPAuth\.h>|#include <auth/FTPAuth.h>|g' "$file"
    sed -i 's|#include "SSHAuth\.h"|#include "auth/SSHAuth.h"|g' "$file"
    sed -i 's|#include <SSHAuth\.h>|#include <auth/SSHAuth.h>|g' "$file"
    sed -i 's|#include "RTSP\.h"|#include "auth/RTSP.h"|g' "$file"
    sed -i 's|#include <RTSP\.h>|#include <auth/RTSP.h>|g' "$file"
    sed -i 's|#include "IPCAuth\.h"|#include "auth/IPCAuth.h"|g' "$file"
    sed -i 's|#include <IPCAuth\.h>|#include <auth/IPCAuth.h>|g' "$file"
    sed -i 's|#include "HikvisionLogin\.h"|#include "auth/HikvisionLogin.h"|g' "$file"
    sed -i 's|#include <HikvisionLogin\.h>|#include <auth/HikvisionLogin.h>|g' "$file"
    sed -i 's|#include "WebformWorker\.h"|#include "auth/WebformWorker.h"|g' "$file"
    sed -i 's|#include <WebformWorker\.h>|#include <auth/WebformWorker.h>|g' "$file"
    
    # Exporters
    sed -i 's|#include "ResultExporter\.h"|#include "exporters/ResultExporter.h"|g' "$file"
    sed -i 's|#include <ResultExporter\.h>|#include <exporters/ResultExporter.h>|g' "$file"
    
    # Network utilities
    sed -i 's|#include "Connector\.h"|#include "network/Connector.h"|g' "$file"
    sed -i 's|#include <Connector\.h>|#include <network/Connector.h>|g' "$file"
    sed -i 's|#include "Utils\.h"|#include "network/Utils.h"|g' "$file"
    sed -i 's|#include <Utils\.h>|#include <network/Utils.h>|g' "$file"
    sed -i 's|#include "BruteUtils\.h"|#include "network/BruteUtils.h"|g' "$file"
    sed -i 's|#include <BruteUtils\.h>|#include <network/BruteUtils.h>|g' "$file"
    sed -i 's|#include "AdaptiveScanner\.h"|#include "network/AdaptiveScanner.h"|g' "$file"
    sed -i 's|#include <AdaptiveScanner\.h>|#include <network/AdaptiveScanner.h>|g' "$file"
    sed -i 's|#include "FileDownloader\.h"|#include "network/FileDownloader.h"|g' "$file"
    sed -i 's|#include <FileDownloader\.h>|#include <network/FileDownloader.h>|g' "$file"
    sed -i 's|#include "FileUpdater\.h"|#include "network/FileUpdater.h"|g' "$file"
    sed -i 's|#include <FileUpdater\.h>|#include <network/FileUpdater.h>|g' "$file"
    sed -i 's|#include "IPRandomizer\.h"|#include "network/IPRandomizer.h"|g' "$file"
    sed -i 's|#include <IPRandomizer\.h>|#include <network/IPRandomizer.h>|g' "$file"
    sed -i 's|#include "base64\.h"|#include "network/base64.h"|g' "$file"
    sed -i 's|#include <base64\.h>|#include <network/base64.h>|g' "$file"
    sed -i 's|#include "ProgressMonitor\.h"|#include "network/ProgressMonitor.h"|g' "$file"
    sed -i 's|#include <ProgressMonitor\.h>|#include <network/ProgressMonitor.h>|g' "$file"
    
    # Common headers
    sed -i 's|#include "externData\.h"|#include "common/externData.h"|g' "$file"
    sed -i 's|#include <externData\.h>|#include <common/externData.h>|g' "$file"
    sed -i 's|#include "externFunctions\.h"|#include "common/externFunctions.h"|g' "$file"
    sed -i 's|#include <externFunctions\.h>|#include <common/externFunctions.h>|g' "$file"
    sed -i 's|#include "mainResources\.h"|#include "common/mainResources.h"|g' "$file"
    sed -i 's|#include <mainResources\.h>|#include <common/mainResources.h>|g' "$file"
}

# Update all source files
find src/ -name "*.cpp" -o -name "*.h" | while read file; do
    update_includes "$file"
done

# Update root level files
update_includes "main.cpp"
update_includes "main_console.cpp"

echo "Include paths updated!"

