#include "stdafx.h"
#include "DebugUtil.h"

int HexDigitVal(char c)
{
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A') + 0x0A;
    if (c >= 'a' && c <= 'f') return (c - 'a') + 0x0A;
    return 0;
}

int ParseHexString(char *dst, const char* src)
{
    bool bHiNibble = true;
    uint8_t curByte = 0;
    int size = 0;

    for (int i = 0; src[i] != '\0'; i++)
    {
        if (!isxdigit(src[i]))
        {
            if (!bHiNibble)
            {
                return 0;
            }

            if (isspace(src[i]))
            {
                continue;
            }

            return 0;
        }

        if (bHiNibble)
        {
            curByte = (HexDigitVal(src[i]) << 4) & 0xF0;
            bHiNibble = false;
        }
        else
        {
            curByte |= HexDigitVal(src[i]);
            if (dst != NULL)
            {
                dst[size] = curByte;
            }
            size++;
            bHiNibble = true;
        }
    }

    if (!bHiNibble)
    {
        return 0;
    }

    return size;
}

bool GetUniqueRomName(char *name, size_t maxLength)
{
    char* rom = (char*)g_Rom->GetRomAddress();

    if (rom == NULL)
    {
        return false;
    }

    char cartridgeId[2] = { rom[0x3C ^ 3], rom[0x3D ^ 3] };
    char country = rom[0x3E ^ 3];
    char version = rom[0x3F ^ 3];

    char imageName[0x15];

    for (size_t i = 0; i < 0x14; i++)
    {
        imageName[i] = rom[0x20 + (i ^ 3)];
    }

    imageName[0x14] = '\0';

    // rtrim
    for (size_t i = 0x14; i >= 0; i--)
    {
        if (isspace(imageName[i]))
        {
            imageName[i] = '\0';
        }
        else if (imageName[i] == '\0')
        {
            continue;
        }
        else
        {
            break;
        }
    }

    // space -> underscore
    for (size_t i = 0; imageName[i] != '\0'; i++)
    {
        if (isspace(imageName[i]))
        {
            imageName[i] = '_';
        }
    }

    snprintf(name, maxLength, "%s-%c%c%c%02X",
        imageName, cartridgeId[0], cartridgeId[1], country, version);

    return true;
}
