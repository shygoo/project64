#include "stdafx.h"
#include "Symbols.h"


vector<SYMBOLENTRY> CSymbols::m_Symbols;
int CSymbols::m_NextSymbolId;

CFile CSymbols::m_SymFileHandle;
char* CSymbols::m_SymFileBuffer;

int CSymbols::GetTypeNumber(char* typeName)
{
	const char* name;
	for (int i = 0; (name = SymbolTypes[i]) != NULL; i++)
	{
		if (strcmp(typeName, name) == 0)
		{
			return i;
		}
	}
	return -1;
}

const char* CSymbols::GetTypeName(int typeNumber)
{
	if (typeNumber > 11)
	{
		return NULL;
	}
	return SymbolTypes[typeNumber];
}

// Open symbols file for game and parse into list

CPath CSymbols::GetSymFilePath()
{
	stdstr symFileName;
	symFileName.Format("%s.sym", g_Settings->LoadStringVal(Game_GameName).c_str());

	CPath symFilePath(g_Settings->LoadStringVal(Directory_NativeSave).c_str(), symFileName.c_str());

	if (g_Settings->LoadBool(Setting_UniqueSaveDir))
	{
		symFilePath.AppendDirectory(g_Settings->LoadStringVal(Game_UniqueSaveDir).c_str());
	}
	if (!symFilePath.DirectoryExists())
	{
		symFilePath.DirectoryCreate();
	}

	return symFilePath;
}

void CSymbols::Load()
{
	if (g_Settings->LoadStringVal(Game_GameName).length() == 0)
	{
		// no game is loaded
		MessageBox(NULL, "Game must be loaded", "Symbols", MB_ICONWARNING | MB_OK);
		return;
	}
	
	CPath symFilePath = GetSymFilePath();

	m_SymFileHandle.Open(symFilePath, CFileBase::modeReadWrite);
	m_SymFileHandle.SeekToBegin();

	m_Symbols.clear();

	if (m_SymFileBuffer != NULL)
	{
		free(m_SymFileBuffer);
	}

	uint32_t symFileSize = m_SymFileHandle.GetLength();
	m_SymFileBuffer = (char*)malloc(symFileSize + 1);
	m_SymFileBuffer[symFileSize] = '\0';
	m_SymFileHandle.Read(m_SymFileBuffer, symFileSize);

	char* bufCopy = (char*)malloc(symFileSize + 1);
	strcpy(bufCopy, m_SymFileBuffer);

	char* curToken = strtok(m_SymFileBuffer, ",");

	int listIndex = 0;
	int lineNumber = 1;
	int fieldPhase = 0;
	m_NextSymbolId = 0;

	ParseError errorCode = ERR_SUCCESS;

	uint32_t address;
	char* name;
	char* description;
	int type;

	while (curToken != NULL)
	{
		char usedDelimeter = bufCopy[curToken - m_SymFileBuffer + strlen(curToken)];
		int tokenlen = strlen(curToken);

		switch (fieldPhase)
		{
		case 0: // address
			char* endptr;
			address = strtoull(curToken, &endptr, 16);
			name = NULL;
			description = NULL;
			type = 0;
			if (*endptr != NULL)
			{
				errorCode = ERR_INVALID_ADDR;
				goto error_check;
			}
			break;
		case 1: // type
			type = GetTypeNumber(curToken);
			if (type == -1)
			{
				errorCode = ERR_INVALID_TYPE;
				goto error_check;
			}
			break;
		case 2: // name
			if (tokenlen == 0)
			{
				errorCode = ERR_INVALID_NAME;
				goto error_check;
			}
			name = (char*)malloc(tokenlen + 1);
			strcpy(name, curToken);
			break;
		case 3: // desc
			description = (char*)malloc(tokenlen + 1);
			strcpy(description, curToken);
			break;
		}

		// Check if line has ended
		if (usedDelimeter == '\n' || usedDelimeter == '\0')
		{
			if (fieldPhase < 2)
			{
				errorCode = ERR_MISSING_FIELDS;
				goto error_check;
			}

			Add(type, address, name, description);

			lineNumber++;
			listIndex++;
			fieldPhase = 0;
			curToken = strtok(NULL, ",\n");
			continue;
		}

		// Line hasn't ended
		if (fieldPhase + 1 == 3)
		{
			curToken = strtok(NULL, "\n");
		}
		else
		{
			curToken = strtok(NULL, ",\n");
		}
		fieldPhase++;
	}

error_check:

	free(bufCopy);
	m_SymFileHandle.Close();

	switch (errorCode)
	{
	case ERR_SUCCESS:
		break;
	case ERR_INVALID_ADDR:
		ParseErrorAlert("Invalid address", lineNumber);
		break;
	case ERR_INVALID_TYPE:
		ParseErrorAlert("Invalid type", lineNumber);
		break;
	case ERR_INVALID_NAME:
		ParseErrorAlert("Invalid name", lineNumber);
		break;
	case ERR_MISSING_FIELDS:
		ParseErrorAlert("Missing required field(s)", lineNumber);
		break;
	}
}

void CSymbols::Save()
{
	int nSymbols = m_Symbols.size();
	
	char* symfile;
	int symfile_size = 0;
	int symfile_idx = 0;

	// Determine file size
	for (int i = 0; i < nSymbols; i++)
	{
		SYMBOLENTRY symbol = m_Symbols[i];

		symfile_size += 11; // address 8, required commas 2, newline 1
		symfile_size += strlen(symbol.name);
		symfile_size += strlen(SymbolTypes[symbol.type]);

		if (symbol.description != NULL)
		{
			symfile_size += 1; // comma
			symfile_size += strlen(symbol.description);
		}
	}

	if (symfile_size == 0)
	{
		return;
	}

	symfile = (char*) malloc(symfile_size + 1);
	symfile[symfile_size] = '\0';

	// Write out
	for (int i = 0; i < nSymbols; i++)
	{
		SYMBOLENTRY symbol = m_Symbols[i];
		symfile_idx += sprintf(&symfile[symfile_idx], "%08X,%s,%s", symbol.address, SymbolTypes[symbol.type], symbol.name);
		if (symbol.description != NULL)
		{
			symfile_idx += sprintf(&symfile[symfile_idx], ",%s", symbol.description);
		}
		symfile_idx += sprintf(&symfile[symfile_idx], "\n");
	}
	
	m_SymFileHandle.Open(GetSymFilePath(), CFileBase::modeReadWrite);
	m_SymFileHandle.Write(symfile, symfile_size);
	m_SymFileHandle.SetEndOfFile();
	m_SymFileHandle.Close();

	free(symfile);
}

void CSymbols::GetValueString(char* dest, int type, uint32_t address)
{
	uint8_t val8;
	uint16_t val16;
	uint32_t val32;
	uint64_t val64;
	float valf;
	double vald;

	switch (type)
	{
	case TYPE_U8:
		g_MMU->LB_VAddr(address, val8);
		sprintf(dest, "%uhh", val8);
		break;
	case TYPE_U16:
		g_MMU->LH_VAddr(address, val16);
		sprintf(dest, "%uh", val16);
		break;
	case TYPE_U32:
		g_MMU->LW_VAddr(address, val32);
		sprintf(dest, "%ud", val32);
		break;
	case TYPE_U64:
		g_MMU->LD_VAddr(address, val64);
		sprintf(dest, "%ull", val64);
		break;
	case TYPE_S8:
		g_MMU->LB_VAddr(address, val8);
		sprintf(dest, "%ihh", val8);
		break;
	case TYPE_S16:
		g_MMU->LH_VAddr(address, val16);
		sprintf(dest, "%ih", val16);
		break;
	case TYPE_S32:
		g_MMU->LW_VAddr(address, val32);
		sprintf(dest, "%id", val32);
		break;
	case TYPE_S64:
		g_MMU->LD_VAddr(address, val64);
		sprintf(dest, "%ill", val64);
		break;
	case TYPE_FLOAT:
		g_MMU->LW_VAddr(address, val32);
		valf = *(float*)&val32;
		sprintf(dest, "%f", valf);
		break;
	case TYPE_DOUBLE:
		g_MMU->LD_VAddr(address, val64);
		vald = *(double*)&val64;
		sprintf(dest, "%f", vald);
		break;
	}

}

void CSymbols::ParseErrorAlert(char* message, int lineNumber)
{
	stdstr messageFormatted = stdstr_f("%s\nLine %d", message, lineNumber);
	MessageBox(NULL, messageFormatted.c_str(), "Parse error", MB_OK | MB_ICONWARNING);
}

SYMBOLENTRY* CSymbols::GetEntryById(int id)
{
	int len = m_Symbols.size();
	for (int i = 0; i < len; i++)
	{
		if (m_Symbols[i].id == id)
		{
			return &m_Symbols[i];
		}
	}
}

const char* CSymbols::GetNameByAddress(uint32_t address)
{
	int len = GetCount();
	for (uint32_t i = 0; i < len; i++)
	{
		if (m_Symbols[i].address == address)
		{
			return m_Symbols[i].name;
		}
	}
	return NULL;
}

void CSymbols::Add(int type, uint32_t address, char* name, char* description)
{
	SYMBOLENTRY symbol;
	symbol.type = type;
	symbol.address = address;
	symbol.name = name;
	symbol.description = description;
	symbol.id = m_NextSymbolId++;
	m_Symbols.push_back(symbol);
}

void CSymbols::RemoveEntryById(int id)
{
	for (int i = 0; i < m_Symbols.size(); i++)
	{
		if (m_Symbols[i].id == id)
		{
			free(m_Symbols[i].description);
			free(m_Symbols[i].name);
			m_Symbols.erase(m_Symbols.begin() + i);
			break;
		}
	}
}

int CSymbols::GetCount()
{
	return m_Symbols.size();
}

SYMBOLENTRY* CSymbols::GetEntryByIndex(int index)
{
	if (index < 0 || index >= GetCount())
	{
		return NULL;
	}
	return &m_Symbols[index];
}

char* CSymbols::GetNameByIndex(int index)
{
	SYMBOLENTRY* lpSymbol = GetEntryByIndex(index);
	if (lpSymbol == NULL)
	{
		return NULL;
	}
	return lpSymbol->name;
}

char* CSymbols::GetTypeStrByIndex(int index)
{
	SYMBOLENTRY* lpSymbol = GetEntryByIndex(index);
	if (lpSymbol == NULL)
	{
		return NULL;
	}
	return SymbolTypes[lpSymbol->type];
}

char* CSymbols::GetDescriptionByIndex(int index)
{
	SYMBOLENTRY* lpSymbol = GetEntryByIndex(index);
	if (lpSymbol == NULL)
	{
		return NULL;
	}
	return lpSymbol->description;
}

uint32_t CSymbols::GetAddressByIndex(int index)
{
	SYMBOLENTRY* lpSymbol = GetEntryByIndex(index);
	if (lpSymbol == NULL)
	{
		return NULL;
	}
	return lpSymbol->address;
}

int CSymbols::GetIdByIndex(int index)
{
	SYMBOLENTRY* lpSymbol = GetEntryByIndex(index);
	if (lpSymbol == NULL)
	{
		return NULL;
	}
	return lpSymbol->id;
}