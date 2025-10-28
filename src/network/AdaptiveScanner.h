#ifndef ADAPTIVESCANNER_H
#define ADAPTIVESCANNER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>
#include <deque>
#include <functional>
#include <algorithm>

// Network load metrics
struct NetworkMetrics {
    double avgResponseTime;      // Average response time in milliseconds
    double timeoutRate;           // Percentage of timeouts (0.0 - 1.0)
    int activeConnections;        // Current number of active connections
    int successfulRequests;       // Number of successful requests in window
    int failedRequests;           // Number of failed requests in window
    std::chrono::steady_clock::time_point lastUpdate;
};

// Port priority data for smart scan
struct PortPriority {
    int port;
    int priority;  // Higher = more important
    int successCount;  // How many times this port was successful
};

class AdaptiveScanner {
private:
    static NetworkMetrics metrics;
    static std::mutex metricsMutex;
    static std::deque<double> responseTimeHistory;  // Recent response times
    static std::deque<bool> requestOutcomes;        // true = success, false = timeout/error
    static const size_t METRICS_WINDOW_SIZE = 100;  // Number of requests to track
    static std::atomic<int> currentThreadCount;
    static std::atomic<int> targetThreadCount;
    static QMap<QString, QList<PortPriority>> foundIPPorts;  // IP -> prioritized ports
    static std::mutex smartScanMutex;
    static std::chrono::steady_clock::time_point lastAdaptiveAdjustment;
    
    static const int MIN_THREADS = 10;
    static const int MAX_THREADS = 2000;
    static const double TARGET_TIMEOUT_RATE = 0.1;  // 10% timeout rate is acceptable
    static const double ADJUSTMENT_INTERVAL_SEC = 5.0;  // Adjust threads every 5 seconds
    
    // Internal methods
    static void updateMetrics(double responseTime, bool success);
    static int calculateOptimalThreads();
    static void adjustThreadCount();

public:
    // Initialize adaptive scanning
    static void initialize(bool enableAdaptive, bool enableSmartScan);
    
    // Record a network request outcome (called after each request)
    static void recordRequest(double responseTimeMs, bool success);
    
    // Get current network load metrics
    static NetworkMetrics getMetrics();
    
    // Adaptive thread management
    static int getOptimalThreadCount();
    static void updateThreadCount();
    
    // Smart scan: prioritize ports for found IPs
    static void recordFoundIP(const QString& ip, int port);
    static QList<int> getPrioritizedPorts(const QString& ip);
    static bool shouldPrioritizeIP(const QString& ip);
    
    // Batch processing
    static std::vector<QString> createBatch(const std::vector<QString>& ipList, int batchSize);
    static void processBatch(const std::vector<QString>& batch, std::function<void(const QString&)> processor);
    
    // Cleanup
    static void cleanup();
};

#endif // ADAPTIVESCANNER_H

