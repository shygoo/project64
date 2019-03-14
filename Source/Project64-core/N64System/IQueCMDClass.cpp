
#include "stdafx.h"
#include "IQueCMDClass.h"
#include "SystemGlobals.h"
#include "Mips/MemoryVirtualMem.h"

#if defined(_MSC_VER)
#define bswap32(word) _byteswap_ulong(word)
#elif defined(__GNUC__)
#define bswap32(word) __builtin_bswap32(word)
#else
#define bswap32(word) (word & 0x000000FFul) << 24 |\
                      (word & 0x0000FF00ul) << 8  |\
                      (word & 0x00FF0000ul) >> 8  |\
                      (word & 0xFF000000ul) >> 24
#endif

CIQueCMD::CIQueCMD() :
    m_Data(NULL),
    m_Size(0)
{}

CIQueCMD::~CIQueCMD()
{
    UnloadCMD();
}

bool CIQueCMD::HaveData(void)
{
    return (m_Data != NULL);
}

void CIQueCMD::UnloadCMD(void)
{
    if (m_Data != NULL)
    {
        delete[] m_Data;
        m_Data = NULL;
    }
    if (m_File.IsOpen())
    {
        m_File.Close();
    }
}

bool CIQueCMD::LoadCMD(const char* cmdPath)
{
    UnloadCMD();

    if (!m_File.Open(cmdPath, CFileBase::modeRead))
    {
        WriteTrace(TraceN64System, TraceError,
            stdstr_f("Failed to load iQue CMD file (tried %s)", cmdPath).c_str());

        goto err_false;
    }

    m_Size = m_File.GetLength();

    if (m_Size < sizeof(params_t))
    {
        WriteTrace(TraceN64System, TraceError, "Supplied iQue CMD file is too small");
        goto err_false;
    }

    m_Data = new uint8_t[m_Size];

    if (m_Data == NULL)
    {
        WriteTrace(TraceN64System, TraceError, "Failed to allocate iQue CMD data");
        goto err_false;
    }

    m_File.SeekToBegin();
    m_File.Read(m_Data, m_Size);

    params_t* params = (params_t*)m_Data;

    if (memcmp(params->magic, "CAM", sizeof(params->magic)) != 0)
    {
        WriteTrace(TraceN64System, TraceError, "Invalid iQue CMD file? \"CAM\" magic missing");
        goto err_false;
    }

    return true;

err_false:
    UnloadCMD();
    return false;
}

void CIQueCMD::CopyBootParamsToRDRAM(uint8_t *ram)
{
    if (!HaveData())
    {
        return;
    }

    params_t* params = (params_t*)m_Data;

    *(uint32_t*)&ram[0x35C] = bswap32(params->eepromAddress);
    *(uint32_t*)&ram[0x360] = bswap32(params->eepromSize);
    *(uint32_t*)&ram[0x364] = bswap32(params->flashAddress);
    *(uint32_t*)&ram[0x368] = bswap32(params->flashSize);
    *(uint32_t*)&ram[0x36C] = bswap32(params->sramAddress);
    *(uint32_t*)&ram[0x370] = bswap32(params->sramSize);
    *(uint32_t*)&ram[0x374] = bswap32(params->pak0Address);
    *(uint32_t*)&ram[0x378] = bswap32(params->pak1Address);
    *(uint32_t*)&ram[0x37C] = bswap32(params->pak2Address);
    *(uint32_t*)&ram[0x380] = bswap32(params->pak3Address);
    *(uint32_t*)&ram[0x384] = bswap32(params->pakSize);
    *(uint32_t*)&ram[0x388] = bswap32(params->romBase);
    *(uint32_t*)&ram[0x300] = bswap32(params->tvType);
    *(uint32_t*)&ram[0x318] = bswap32(params->memSize);
    //*(uint32_t*)&ram[0x390] = params->magic; ?
    *(uint32_t*)&ram[0x3B8] = (uint32_t)params->auxDataLimit;
}

void CIQueCMD::DumpSaveDataFromRDRAM(uint8_t* ram)
{
    if (!HaveData())
    {
        return;
    }

    params_t* params = (params_t*)m_Data;

    uint32_t eepromAddress = bswap32(params->eepromAddress) & 0x00FFFFFF;
    uint32_t eepromSize = bswap32(params->eepromSize);
    uint32_t flashAddress = bswap32(params->flashAddress) & 0x00FFFFFF;
    uint32_t flashSize = bswap32(params->flashSize);
    uint32_t sramAddress = bswap32(params->sramAddress) & 0x00FFFFFF;
    uint32_t sramSize = bswap32(params->sramSize);
    uint32_t pak0Address = bswap32(params->pak0Address) & 0x00FFFFFF;
    uint32_t pak1Address = bswap32(params->pak1Address) & 0x00FFFFFF;
    uint32_t pak2Address = bswap32(params->pak2Address) & 0x00FFFFFF;
    uint32_t pak3Address = bswap32(params->pak3Address) & 0x00FFFFFF;
    uint32_t pakSize = bswap32(params->pakSize);

    // todo can save data can be in a TLB segment?

    if (eepromAddress != 0)
    {
        if ((eepromSize == 0x200 || eepromSize == 0x800) &&
            (eepromAddress + eepromSize) <= g_MMU->RdramSize())
        {
            uint8_t *eepData = g_MMU->Rdram() + eepromAddress;

            FILE* fp = fopen("Save/ique_test.iqeep", "wb");
            fwrite(eepData, 1, eepromSize, fp);
            fclose(fp);
        }
    }

    if (flashAddress != 0)
    {
        if (flashSize == 0x20000 &&
            (flashAddress + flashSize) <= g_MMU->RdramSize())
        {
            uint8_t* flashData = g_MMU->Rdram() + flashAddress;

            FILE* fp = fopen("Save/ique_test.iqfla", "wb");
            fwrite(flashData, 1, eepromSize, fp);
            fclose(fp);
        }
    }

    if (sramAddress != 0)
    {
        if (sramSize == 0x8000 &&
            (sramAddress + sramSize) <= g_MMU->RdramSize())
        {
            uint8_t* sramData = g_MMU->Rdram() + sramAddress;
            FILE* fp = fopen("Save/ique_test.iqsra", "wb");
            fwrite(sramData, 1, sramSize, fp);
            fclose(fp);
        }
    }

    if (pakSize != 0)
    {
        if (pak0Address != 0 && (pak0Address + pakSize) <= g_MMU->RdramSize())
        {
            uint8_t* pak0Data = g_MMU->Rdram() + pak0Address;
            FILE* fp = fopen("Save/ique_test.iqpak0", "wb");
            fwrite(pak0Data, 1, sramSize, fp);
            fclose(fp);
        }

        //todo
    }

}