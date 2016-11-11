#pragma once
#include "stdafx.h"

typedef struct {
	int type;
	uint32_t address;
	char* name;
	char* description;
	int id;
} SYMBOLENTRY;

class CSymbols
{
public:
	typedef enum {
		ERR_SUCCESS,
		ERR_INVALID_TYPE,
		ERR_INVALID_ADDR,
		ERR_INVALID_NAME,
		ERR_MISSING_FIELDS,
	} ParseError;

	typedef enum {
		TYPE_CODE,
		TYPE_DATA,
		TYPE_U8,
		TYPE_U16,
		TYPE_U32,
		TYPE_U64,
		TYPE_S8,
		TYPE_S16,
		TYPE_S32,
		TYPE_S64,
		TYPE_FLOAT,
		TYPE_DOUBLE
	} DataType;

	static constexpr char* SymbolTypes[] = {
		"code", // 0
		"data", // 1
		"u8",
		"u16",
		"u32",
		"u64",
		"s8",
		"s16",
		"s32",
		"s64",
		"float",
		"double",
		NULL
	};

private:
	static vector<SYMBOLENTRY> m_Symbols;
	static int m_NextSymbolId;
	
	static CFile m_SymFileHandle;
	static char* m_SymFileBuffer;
	
public:
	static void Load();
	static void Save();
	static void ParseErrorAlert(char* message, int lineNumber);
	
	static void Add(int type, uint32_t address, char* name, char* description = NULL);
	
	static const char* GetTypeName(int typeNumber);
	static int GetTypeNumber(char* typeName);
	static void GetValueString(char* dest, int type, uint32_t address);

	static int GetCount();

	static SYMBOLENTRY* GetEntryById(int id);
	static SYMBOLENTRY* GetEntryByIndex(int id);

	static const char* GetNameByAddress(uint32_t address);

	static int GetIdByIndex(int index);
	static char* GetNameByIndex(int index);
	static char* GetTypeStrByIndex(int index);
	static char* GetDescriptionByIndex(int index);
	static uint32_t GetAddressByIndex(int index);

};