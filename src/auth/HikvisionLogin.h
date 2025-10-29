#ifndef HIKVISIONLOGIN_H
#define HIKVISIONLOGIN_H

#include <STh.h>
#include <mainResources.h>

// External declarations for Hikvision protocol headers and functions
extern const char headerIVMS[32];
extern const char headerRVI[32];
extern int recvWT(SOCKET socket, char *Buffer, int Len, long Timeout, int *bTimedOut);

class HikVis {
public: static bool isInitialized;

private: lopaStr hikLogin(const char * sDVRIP, int wDVRPort);
		 lopaStr rviLogin(const char * sDVRIP, int wDVRPort);

public:
	static int hikCounter;
	static int rviCounter;
	static int hikPart;
	static int rviPart;
public:
	static bool checkHikk(const char * sDVRIP, int port);
	static bool checkRVI(const char * sDVRIP, int port);
	static bool checkSAFARI(const char * sDVRIP, int port);
	void hikInit();
	void hikCleanup();
	lopaStr HVLobby(const char *ip, const int port);
	lopaStr RVILobby(const char *ip, const int port);
};

#endif // HIKVISIONLOGIN_H