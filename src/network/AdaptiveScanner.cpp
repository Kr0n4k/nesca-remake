#include <AdaptiveScanner.h>
#include <externData.h>
#include <STh.h>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <functional>

// Static member initialization
NetworkMetrics AdaptiveScanner::metrics = {0.0, 0.0, 0, 0, 0, std::chrono::steady_clock::now()};
std::mutex AdaptiveScanner::metricsMutex;
std::deque<double> AdaptiveScanner::responseTimeHistory;
std::deque<bool> AdaptiveScanner::requestOutcomes;
std::atomic<int> AdaptiveScanner::currentThreadCount(0);
std::atomic<int> AdaptiveScanner::targetThreadCount(0);
QMap<QString, QList<PortPriority>> AdaptiveScanner::foundIPPorts;
std::mutex AdaptiveScanner::smartScanMutex;
std::chrono::steady_clock::time_point AdaptiveScanner::lastAdaptiveAdjustment = std::chrono::steady_clock::now();

void AdaptiveScanner::initialize(bool enableAdaptive, bool enableSmartScan) {
    std::lock_guard<std::mutex> lock(metricsMutex);
    responseTimeHistory.clear();
    requestOutcomes.clear();
    metrics = {0.0, 0.0, 0, 0, 0, std::chrono::steady_clock::now()};
    
    if (enableAdaptive && gThreads > 0) {
        targetThreadCount = gThreads.load();
        currentThreadCount = gThreads.load();
    }
    
    if (enableSmartScan) {
        std::lock_guard<std::mutex> smartLock(smartScanMutex);
        foundIPPorts.clear();
    }
    
    lastAdaptiveAdjustment = std::chrono::steady_clock::now();
}

void AdaptiveScanner::updateMetrics(double responseTimeMs, bool success) {
    if (!gAdaptiveScan) return;
    
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    // Add to history
    responseTimeHistory.push_back(responseTimeMs);
    requestOutcomes.push_back(success);
    
    // Keep window size limited
    if (responseTimeHistory.size() > METRICS_WINDOW_SIZE) {
        responseTimeHistory.pop_front();
    }
    if (requestOutcomes.size() > METRICS_WINDOW_SIZE) {
        requestOutcomes.pop_front();
    }
    
    // Update metrics
    if (!responseTimeHistory.empty()) {
        double sum = std::accumulate(responseTimeHistory.begin(), responseTimeHistory.end(), 0.0);
        metrics.avgResponseTime = sum / responseTimeHistory.size();
    }
    
    if (!requestOutcomes.empty()) {
        int failures = std::count(requestOutcomes.begin(), requestOutcomes.end(), false);
        metrics.timeoutRate = static_cast<double>(failures) / requestOutcomes.size();
        metrics.successfulRequests = std::count(requestOutcomes.begin(), requestOutcomes.end(), true);
        metrics.failedRequests = failures;
    }
    
    metrics.lastUpdate = std::chrono::steady_clock::now();
    metrics.activeConnections = cons.load();
}

NetworkMetrics AdaptiveScanner::getMetrics() {
    std::lock_guard<std::mutex> lock(metricsMutex);
    return metrics;
}

int AdaptiveScanner::calculateOptimalThreads() {
    if (!gAdaptiveScan || gThreads <= 0) {
        return gThreads.load();
    }
    
    int currentThreads = gThreads.load();
    NetworkMetrics m = getMetrics();
    
    // If we don't have enough data yet, keep current threads
    if (requestOutcomes.size() < 20) {
        return currentThreads;
    }
    
    int optimal = currentThreads;
    
    // High timeout rate (>20%) - reduce threads
    if (m.timeoutRate > 0.20) {
        optimal = static_cast<int>(currentThreads * 0.8);  // Reduce by 20%
    }
    // Moderate timeout rate (10-20%) - slight reduction
    else if (m.timeoutRate > 0.10) {
        optimal = static_cast<int>(currentThreads * 0.9);  // Reduce by 10%
    }
    // Low timeout rate (<5%) and high response time - increase threads
    else if (m.timeoutRate < 0.05 && m.avgResponseTime > 1000) {
        optimal = static_cast<int>(currentThreads * 1.2);  // Increase by 20%
        if (optimal > MAX_THREADS) optimal = MAX_THREADS;
    }
    // Very low timeout rate (<2%) - can increase more aggressively
    else if (m.timeoutRate < 0.02 && currentThreads < MAX_THREADS) {
        optimal = static_cast<int>(currentThreads * 1.1);  // Increase by 10%
        if (optimal > MAX_THREADS) optimal = MAX_THREADS;
    }
    
    // Clamp to valid range
    if (optimal < MIN_THREADS) optimal = MIN_THREADS;
    if (optimal > MAX_THREADS) optimal = MAX_THREADS;
    
    return optimal;
}

void AdaptiveScanner::adjustThreadCount() {
    if (!gAdaptiveScan) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastAdaptiveAdjustment).count();
    
    // Only adjust every ADJUSTMENT_INTERVAL_SEC seconds
    if (elapsed < ADJUSTMENT_INTERVAL_SEC * 1000) {
        return;
    }
    
    int optimal = calculateOptimalThreads();
    int current = gThreads.load();
    
    // Only adjust if difference is significant (>10%)
    if (std::abs(optimal - current) > (current * 0.1)) {
        gThreads = optimal;
        targetThreadCount = optimal;
        lastAdaptiveAdjustment = now;
        
        // Log adjustment if debugging
        if (gDebugMode && stt != nullptr) {
            QString msg = QString("[Adaptive] Adjusted threads: %1 -> %2 (timeout rate: %3%%, avg response: %4ms)")
                .arg(current).arg(optimal)
                .arg(metrics.timeoutRate * 100.0, 0, 'f', 1)
                .arg(metrics.avgResponseTime, 0, 'f', 0);
            stt->doEmitionYellowFoundData(msg);
        }
    }
}

int AdaptiveScanner::getOptimalThreadCount() {
    if (!gAdaptiveScan) {
        return gThreads.load();
    }
    
    updateThreadCount();
    return gThreads.load();
}

void AdaptiveScanner::updateThreadCount() {
    if (!gAdaptiveScan) return;
    adjustThreadCount();
}

void AdaptiveScanner::recordRequest(double responseTimeMs, bool success) {
    if (!gAdaptiveScan) return;
    updateMetrics(responseTimeMs, success);
}

void AdaptiveScanner::recordFoundIP(const QString& ip, int port) {
    if (!gSmartScan) return;
    
    std::lock_guard<std::mutex> lock(smartScanMutex);
    
    if (!foundIPPorts.contains(ip)) {
        foundIPPorts[ip] = QList<PortPriority>();
    }
    
    QList<PortPriority>& ports = foundIPPorts[ip];
    
    // Find existing port entry or create new one
    bool found = false;
    for (PortPriority& pp : ports) {
        if (pp.port == port) {
            pp.successCount++;
            pp.priority = pp.successCount * 10;  // Increase priority with each success
            found = true;
            break;
        }
    }
    
    if (!found) {
        PortPriority pp;
        pp.port = port;
        pp.successCount = 1;
        pp.priority = 10;  // Initial priority
        ports.append(pp);
    }
    
    // Sort by priority (descending)
    std::sort(ports.begin(), ports.end(), 
              [](const PortPriority& a, const PortPriority& b) {
                  return a.priority > b.priority;
              });
    
    // Limit to top 20 ports per IP
    if (ports.size() > 20) {
        ports = ports.mid(0, 20);
    }
}

QList<int> AdaptiveScanner::getPrioritizedPorts(const QString& ip) {
    if (!gSmartScan) {
        return QList<int>();
    }
    
    std::lock_guard<std::mutex> lock(smartScanMutex);
    
    if (foundIPPorts.contains(ip)) {
        QList<int> ports;
        for (const PortPriority& pp : foundIPPorts[ip]) {
            ports.append(pp.port);
        }
        return ports;
    }
    
    return QList<int>();
}

bool AdaptiveScanner::shouldPrioritizeIP(const QString& ip) {
    if (!gSmartScan) return false;
    
    std::lock_guard<std::mutex> lock(smartScanMutex);
    return foundIPPorts.contains(ip) && !foundIPPorts[ip].isEmpty();
}

std::vector<QString> AdaptiveScanner::createBatch(const std::vector<QString>& ipList, int batchSize) {
    if (batchSize <= 0 || ipList.empty()) {
        return ipList;
    }
    
    std::vector<QString> batch;
    size_t startIdx = 0;
    size_t endIdx = std::min(static_cast<size_t>(batchSize), ipList.size());
    
    for (size_t i = startIdx; i < endIdx; i++) {
        batch.push_back(ipList[i]);
    }
    
    return batch;
}

void AdaptiveScanner::processBatch(const std::vector<QString>& batch, 
                                    std::function<void(const QString&)> processor) {
    for (const QString& item : batch) {
        if (!globalScanFlag) break;
        processor(item);
    }
}

void AdaptiveScanner::cleanup() {
    std::lock_guard<std::mutex> lock(metricsMutex);
    std::lock_guard<std::mutex> smartLock(smartScanMutex);
    
    responseTimeHistory.clear();
    requestOutcomes.clear();
    foundIPPorts.clear();
}

