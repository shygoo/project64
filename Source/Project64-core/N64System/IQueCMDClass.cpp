
#include "stdafx.h"
#include "IQueCMDClass.h"
#include "SystemGlobals.h"

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

    ByteSwapData();

    params_t* params = (params_t*)m_Data;

    if (memcmp(params->magic, "CAM", sizeof(params->magic)) == 0)
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

    *(uint32_t*)&ram[0x35C] = params->eepromAddress;
    *(uint32_t*)&ram[0x360] = params->eepromSize;
    *(uint32_t*)&ram[0x364] = params->flashAddress;
    *(uint32_t*)&ram[0x368] = params->flashSize;
    *(uint32_t*)&ram[0x36C] = params->sramAddress;
    *(uint32_t*)&ram[0x370] = params->sramSize;
    *(uint32_t*)&ram[0x374] = params->pak0Address;
    *(uint32_t*)&ram[0x378] = params->pak1Address;
    *(uint32_t*)&ram[0x37C] = params->pak2Address;
    *(uint32_t*)&ram[0x380] = params->pak3Address;
    *(uint32_t*)&ram[0x384] = params->pakSize;
    *(uint32_t*)&ram[0x388] = params->romBase;
    *(uint32_t*)&ram[0x300] = params->tvType;
    *(uint32_t*)&ram[0x318] = params->memSize;
    //*(uint32_t*)&ram[0x390] = params->magic; ?
    *(uint32_t*)&ram[0x3B8] = (uint32_t)params->auxDataLimit;
}

void CIQueCMD::ByteSwapData(void)
{
    for (int i = 0; i < m_Size; i += 4)
    {
        m_Data[i] ^= m_Data[i + 3];
        m_Data[i + 3] ^= m_Data[i];
        m_Data[i] ^= m_Data[i + 3];
        m_Data[i + 1] ^= m_Data[i + 2];
        m_Data[i + 2] ^= m_Data[i + 1];
        m_Data[i + 1] ^= m_Data[i + 2];
    }
}