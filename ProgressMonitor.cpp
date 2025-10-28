#include "ProgressMonitor.h"
#include "externData.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <QString>

ProgressMonitor::ProgressMonitor(QObject *parent) 
    : QThread(parent), shouldStop(false) {
    startTime = std::chrono::steady_clock::now();
}

ProgressMonitor::~ProgressMonitor() {
    shouldStop = true;
    if (isRunning()) {
        wait(1000);
    }
}

void ProgressMonitor::stop() {
    shouldStop = true;
}

void ProgressMonitor::printProgress() {
    extern int found, saved, camerasC1, PieBA, PieOther, PieSSH, Alive;
    extern unsigned long long gTargetsNumber;
    extern long long unsigned int gTargets;
    extern std::atomic<int> cons;
    
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
    
    if (gTargetsNumber == 0) return;
    
    unsigned long long scanned = gTargetsNumber - gTargets;
    double percent = (gTargetsNumber > 0) ? (100.0 * scanned / gTargetsNumber) : 0.0;
    
    // Calculate speed
    double speed = (elapsed > 0) ? (double)scanned / elapsed : 0.0;
    
    // Estimate time remaining
    long long remaining = 0;
    if (speed > 0 && gTargets > 0) {
        remaining = (long long)(gTargets / speed);
    }
    
    // ANSI color codes
    const char* ANSI_RESET = "\033[0m";
    const char* ANSI_CYAN = "\033[36m";
    const char* ANSI_GREEN = "\033[32m";
    const char* ANSI_YELLOW = "\033[33m";
    const char* ANSI_BOLD = "\033[1m";
    
    // Clear line and print progress
    // Use QString for proper formatting with large numbers
    QString progressLine = QString("\r%1%2[PROGRESS]%3 %4% | Scanned: %5%6%7/%8 | Found: %9%10%11 | Saved: %12%13%14")
        .arg(ANSI_BOLD).arg(ANSI_CYAN).arg(ANSI_RESET)
        .arg(percent, 0, 'f', 1)
        .arg(ANSI_GREEN).arg(scanned).arg(ANSI_RESET).arg(gTargetsNumber)
        .arg(ANSI_YELLOW).arg(found).arg(ANSI_RESET)
        .arg(ANSI_GREEN).arg(saved).arg(ANSI_RESET);
    
    printf("%s", progressLine.toUtf8().constData());
    
    if (camerasC1 > 0 || PieBA > 0 || PieSSH > 0) {
        printf(" | ");
        if (camerasC1 > 0) printf("Cam:%d ", camerasC1);
        if (PieBA > 0) printf("Auth:%d ", PieBA);
        if (PieSSH > 0) printf("SSH:%d ", PieSSH);
    }
    
    printf(" | Speed: %.1f IP/s | Threads: %d", speed, cons.load());
    
    if (remaining > 0 && remaining < 3600) {
        int mins = remaining / 60;
        int secs = remaining % 60;
        printf(" | ETA: %dm %ds", mins, secs);
    }
    
    printf("   "); // Extra spaces to clear any leftover characters
    fflush(stdout);
}

void ProgressMonitor::run() {
    while (!shouldStop && globalScanFlag) {
        printProgress();
        msleep(2000); // Update every 2 seconds
    }
    
    // Final progress line
    printProgress();
    printf("\n"); // New line after progress stops
    fflush(stdout);
}

