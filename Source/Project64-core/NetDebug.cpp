#include <stdafx.h>
#include <winsock2.h>
#include <Windows.h>

#include <Project64-core/N64System/Mips/RegisterClass.h>
#include <Project64-core/N64System/SystemGlobals.h>

#include <process.h>
#include <iostream>

#include "NetDebug.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEBUG_INVALID_BP -1

#define DBG_BP_R 1
#define DBG_BP_W 2
#define DBG_BP_E 4

static std::vector<uint32_t> dbgReadBreakpoints = {};
static std::vector<uint32_t> dbgWriteBreakpoints = {};
static std::vector<uint32_t> dbgExecBreakpoints = {}; //  0x802CB1C0 , 0x802CB1C0

static bool dbgPaused = 0;

static SOCKET serverSocket;

static uint32_t getGPRVal(int regno) {
	return g_Reg->m_GPR[regno].UW[0];
}

static uint32_t getPC() {
	return g_Reg->m_PROGRAM_COUNTER;
}
/*
static void reportState(SOCKET clientSocket) {
	char message[1024];
	int len = 0;
	
	len += sprintf(message, "PC %08X\n", getPC());

	for (int i = 0; i < 32; i++) {
		len += sprintf(message + len, "R%-2d %08X ", i, getGPRVal(i));
		if ((i + 1) % 4 == 0)
			len += sprintf(message + len, "\n");
	}
	send(clientSocket, message, len, 0);
	
}

static void sessionThread(void* lpClientSocket) {
	char recvBuf[256];
	SOCKET clientSocket = *(SOCKET*)lpClientSocket;

	while (1) {
		char cmd;
		uint32_t addr = 0;

		if (recv(clientSocket, &cmd, 1, 0) <= 0)
			break;
		
		if (cmd < 6) { // fetch address if cmd < 2
			if (recv(clientSocket, (char*)&addr, 4, 0) <= 0)
				break;
		}

		switch(cmd){
			// address required
			case 0:	dbgRBPAdd(addr); break;
			case 1: dbgRBPRemove(addr); break;
			case 2: dbgWBPAdd(addr); break;
			case 3: dbgWBPRemove(addr); break;
			case 4: dbgEBPAdd(addr); break;
			case 5: dbgEBPRemove(addr); break;
			// no address
			case 6:
				dbgEBPClear();
				dbgRBPClear();
				dbgWBPClear();
				break;
			case 7: dbgUnpause(); break;
			case 8: // get registers
				reportState(clientSocket);
				break;
		}

		static char* stdResponse = "[NetDebug OK]\n";
		send(clientSocket, stdResponse, strlen(stdResponse), 0);

	}

	sprintf(recvBuf, "socket closed with error %d", GetLastError());

	MessageBox(NULL, recvBuf, "Socket closed", MB_OK);

	closesocket(clientSocket);
	free(lpClientSocket);
}

static void listenerThread(void* param) {
	SOCKET clientSocket;
	struct sockaddr_in client;
	int addrSize = sizeof(struct sockaddr_in);

	while(1) {
		clientSocket = accept(serverSocket, (struct sockaddr*)&client, &addrSize);

		if (clientSocket == INVALID_SOCKET)
			break;

		void* lpClientSocket = malloc(sizeof(SOCKET));
		*(SOCKET*)lpClientSocket = clientSocket;
		_beginthread(sessionThread, 512, lpClientSocket);
	}

}*/

int dbgInit() {
	/*
	// initialise winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return 0;
	}

	//SOCKET sock;
	struct sockaddr_in server;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(NETDEBUG_PORT);

	if (bind(serverSocket, (struct sockaddr*) &server, sizeof(server))) {
		return INVALID_SOCKET;
	}

	listen(serverSocket, 3);

	_beginthread(listenerThread, 0, NULL);


	MessageBox(NULL, "net debugger started", "dbgInit", MB_OK);
	*/
	return 1;
}

// exec breakpoint management

int dbgEBPIndex(uint32_t address) {
	for (u_int i = 0; i < dbgExecBreakpoints.size(); i++) {
		if (dbgExecBreakpoints[i] == address) {
			return i;
		}
	}
	return -1;
}

bool dbgEBPExists(uint32_t address) {
	return dbgEBPIndex(address) != DEBUG_INVALID_BP;
}

void dbgEBPAdd(uint32_t address) {
	dbgExecBreakpoints.push_back(address);
}

bool dbgEBPRemove(uint32_t address) {
	int index = dbgEBPIndex(address);
	if (index != DEBUG_INVALID_BP) {
		dbgExecBreakpoints.erase(dbgExecBreakpoints.begin() + index);
		return true;
	}
	return false;
}

void dbgEBPClear() {
	dbgExecBreakpoints.clear();
}

// read breakpoint management

int dbgRBPIndex(uint32_t address) {
	for (u_int i = 0; i < dbgReadBreakpoints.size(); i++) {
		if (dbgReadBreakpoints[i] == address) {
			return i;
		}
	}
	return -1;
}

bool dbgRBPExists(uint32_t address) {
	return dbgRBPIndex(address) != DEBUG_INVALID_BP;
}

void dbgRBPAdd(uint32_t address) {
	dbgReadBreakpoints.push_back(address);
}

bool dbgRBPRemove(uint32_t address) {
	int index = dbgRBPIndex(address);
	if (index != DEBUG_INVALID_BP) {
		dbgReadBreakpoints.erase(dbgReadBreakpoints.begin() + index);
		return true;
	}
	return false;
}

void dbgRBPClear() {
	dbgReadBreakpoints.clear();
}

// write breakpoint management

int dbgWBPIndex(uint32_t address) {
	for (u_int i = 0; i < dbgWriteBreakpoints.size(); i++) {
		if (dbgWriteBreakpoints[i] == address) {
			return i;
		}
	}
	return -1;
}

bool dbgWBPExists(uint32_t address) {
	return dbgWBPIndex(address) != DEBUG_INVALID_BP;
}

void dbgWBPAdd(uint32_t address) {
	dbgWriteBreakpoints.push_back(address);
}

bool dbgWBPRemove(uint32_t address) {
	int index = dbgWBPIndex(address);
	if (index != DEBUG_INVALID_BP) {
		dbgWriteBreakpoints.erase(dbgWriteBreakpoints.begin() + index);
		return true;
	}
	return false;
}

void dbgWBPClear() {
	dbgWriteBreakpoints.clear();
}

////

void dbgPause(uint32_t address) {
	// Pause calling thread until dbgUnpause()
	char notification[48];
	sprintf(notification, "*** CPU PAUSED @ %08X ***", address);
	g_Notify->DisplayMessage(5, notification);
	dbgPaused = true;
	while (dbgPaused) {
		Sleep(10);
	}
}

void dbgUnpause() {
	g_Notify->DisplayMessage(5, MSG_CPU_RESUMED);
	dbgPaused = false;
}