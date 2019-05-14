#include <stdafx.h>
#include "DisplayListParser.h"

#include "DisplayListOps.h"
#include "DisplayListDecode.h"

uint32_t CHleDmemState::SegmentedToPhysical(uint32_t address)
{
	uint32_t segment = (address >> 24) & 0x0F;
	uint32_t offset = address & 0x00FFFFFF;
	return segments[segment] + offset;
}

uint32_t CHleDmemState::SegmentedToVirtual(uint32_t address)
{
    return SegmentedToPhysical(address) | 0x80000000;
}

/************/

CDisplayListParser::CDisplayListParser(void) :
	m_UCodeChecksum(0),
	m_UCodeVersion(UCODE_UNKNOWN),
	m_UCodeName(NULL),
	m_CommandTable(NULL),
	m_RootDListSize(0),
	m_VertexBufferSize(16)
{
}

CHleDmemState* CDisplayListParser::GetLoggedState(size_t index)
{
    if (index >= m_StateLog.size())
    {
        return NULL;
    }

    return &m_StateLog[index];
}

void CDisplayListParser::Reset(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize)
{
    m_StateLog.clear();

    memset(&m_State, 0, sizeof(m_State));

    uint8_t* ucode = g_MMU->Rdram() + ucodeAddr;

    for (int i = 0; i < 3072; i += sizeof(uint32_t))
    {
        m_UCodeChecksum += *(uint32_t*)&ucode[i];
    }

    const ucode_version_info_t* info = GetUCodeVersionInfo(m_UCodeChecksum);

    if (info == NULL)
    {
        m_UCodeVersion = UCODE_UNKNOWN;
        m_UCodeName = "unknown microcode";
        return;
    }

    m_UCodeVersion = info->version;
    m_UCodeName = info->name;
    m_CommandTable = info->commandTable;

    m_State.address = dlistAddr;
}

// decode and execute command
const char* CDisplayListParser::StepDecode(char* paramsTextBuf, uint32_t* flags)
{
    uint32_t physAddress = m_State.SegmentedToPhysical(m_State.address);

    //MessageBox(NULL, stdstr_f("%08X", physAddress).c_str(), "", MB_OK);

    g_MMU->LW_PAddr(physAddress, m_State.command.w0);
    g_MMU->LW_PAddr(physAddress + 4, m_State.command.w1);

    m_StateLog.push_back(m_State);
    m_State.address += 8;

    uint8_t commandByte = m_State.command.w0 >> 24;

    const dl_cmd_info_t *commandInfo;
    const char* commandName = "?";
    sprintf(paramsTextBuf, "?");

    commandInfo = LookupCommand(Commands_Global, commandByte);

    if (commandInfo == NULL && m_CommandTable != NULL)
    {
        commandInfo = LookupCommand(m_CommandTable, commandByte);
    }

    if (commandInfo == NULL)
    {
        return commandName;
    }

    commandName = commandInfo->commandName;

    if (commandInfo->decodeFunc != NULL)
    {
        // disassemble command
        const char* overrideName = commandInfo->decodeFunc(&m_State, paramsTextBuf);

        if (overrideName != NULL)
        {
            commandName = overrideName;
        }
    }

    if (commandInfo->opFunc != NULL)
    {
        // execute command
        commandInfo->opFunc(&m_State);
    }

    return commandName;
}

const ucode_version_info_t* CDisplayListParser::GetUCodeVersionInfo(uint32_t checksum)
{
    for (int i = 0; UCodeVersions[i].name != NULL; i++)
    {
        if (UCodeVersions[i].checksum == checksum)
        {
            return &UCodeVersions[i];
        }
    }

    return NULL;
}

ucode_version_t CDisplayListParser::GetUCodeVersion(void)
{
	return m_UCodeVersion;
}

const char* CDisplayListParser::GetUCodeName(void)
{
	return m_UCodeName;
}

uint32_t CDisplayListParser::GetUCodeChecksum(void)
{
	return m_UCodeChecksum;
}

bool CDisplayListParser::IsDone(void)
{
    return m_State.bDone;
}

uint32_t CDisplayListParser::GetCommandAddress(void)
{
    return m_State.address;
}

uint32_t CDisplayListParser::GetCommandVirtualAddress(void)
{
    return m_State.SegmentedToVirtual(m_State.address);
}

dl_cmd_t CDisplayListParser::GetRawCommand(void)
{
    uint32_t physAddress = m_State.SegmentedToPhysical(m_State.address);

    dl_cmd_t cmd;
    g_MMU->LW_PAddr(physAddress, cmd.w0);
    g_MMU->LW_PAddr(physAddress + 4, cmd.w1);

    return cmd;
}

int CDisplayListParser::GetStackIndex(void)
{
    return m_State.stackIndex;
}

dl_cmd_info_t* CDisplayListParser::LookupCommand(dl_cmd_info_t* commands, uint8_t cmdByte)
{
	for (int i = 0; commands[i].commandName != NULL; i++)
	{
		if (commands[i].commandByte == cmdByte)
		{
			return &commands[i];
		}
	}

	return NULL;
}

const char* CDisplayListParser::LookupName(name_lut_entry_t* set, uint32_t value)
{
    for (int i = 0; set[i].name != NULL; i++)
    {
        if (set[i].value == value)
        {
            return set[i].name;
        }
    }

    return NULL;
}