#pragma once
#include "stdafx.h"

class CIQueCMD
{
    typedef struct
    {
        uint32_t eepromAddress;
        uint32_t eepromSize;
        uint32_t flashAddress;
        uint32_t flashSize;
        uint32_t sramAddress;
        uint32_t sramSize;
        uint32_t pak0Address;
        uint32_t pak1Address;
        uint32_t pak2Address;
        uint32_t pak3Address;
        uint32_t pakSize;
        uint32_t romBase;
        uint32_t tvType;
        uint32_t memSize;
        uint32_t unk38;
        uint32_t unk3C;
        char magic[3];
        char auxDataLimit;
    } params_t;

    uint8_t* m_Data;
    size_t m_Size;
    CFile m_File;

    void ByteSwapData(void);

public:
    CIQueCMD();
    ~CIQueCMD();

    void UnloadCMD(void);
    bool LoadCMD(const char* cmdPath);
    void CopyBootParamsToRDRAM(uint8_t *ram);
    bool HaveData(void);
};