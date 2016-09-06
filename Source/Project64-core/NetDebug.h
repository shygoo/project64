#pragma once

#include <stdint.h>

#define NETDEBUG_PORT ((u_short)8080)

int dbgInit();

int dbgEBPIndex(uint32_t address);
bool dbgEBPExists(uint32_t address);
void dbgEBPAdd(uint32_t address);
bool dbgEBPRemove(uint32_t address);
void dbgEBPClear();

int dbgRBPIndex(uint32_t address);
bool dbgRBPExists(uint32_t address);
void dbgRBPAdd(uint32_t address);
bool dbgRBPRemove(uint32_t address);
void dbgRBPClear();

int dbgWBPIndex(uint32_t address);
bool dbgWBPExists(uint32_t address);
void dbgWBPAdd(uint32_t address);
bool dbgWBPRemove(uint32_t address);
void dbgWBPClear();

void dbgPause();
void dbgUnpause();