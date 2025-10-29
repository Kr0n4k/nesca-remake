#include <Threader.h>

int Threader::gThreadDelay = 1;
int Threader::threadId = 0;
std::mutex Threader::m;
bool Threader::ready = false;
std::condition_variable Threader::cv;
std::queue<std::string> Threader::ipQueue;

void Threader::fireThread(std::string ip, void *func(void)) {

    ipQueue.push(ip);

    if(threadId < gThreads) {
        ++threadId;
        std::thread workerThread(func);
        workerThread.detach();
    }

	ready = true;
	cv.notify_one();
    // Reduced delay for faster thread creation
    if (gThreadDelay > 0) {
        Sleep(gThreadDelay);
    }
}

void Threader::fireThreadBatch(const std::vector<std::string>& ips, void *func(void)) {
    // Add all IPs to queue
    for (const auto& ip : ips) {
        ipQueue.push(ip);
    }
    
    // Create threads up to the limit
    int threadsToCreate = std::min(static_cast<int>(ips.size()), gThreads - threadId);
    for (int i = 0; i < threadsToCreate; ++i) {
        ++threadId;
        std::thread workerThread(func);
        workerThread.detach();
    }
    
    ready = true;
    cv.notify_all();
}

void Threader::cleanUp() {
	ready = true;
	cv.notify_all();
	std::unique_lock<std::mutex> lk(m);
	lk.unlock();
	lk.release();
	Sleep(200);
    threadId = 0;
    std::queue<std::string> empty;
	std::swap(ipQueue, empty);
	ready = false;
}
