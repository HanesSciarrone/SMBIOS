/*
 * Copyright (C) 2015-2020
 * June 14, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <unistd.h>
#include <winsock2.h>
#include <windows.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <map>

#include "windowsHelper.h"

constexpr auto BASEBOARD_INFORMATION_TYPE{2};
static const std::map<std::string, DWORD> gs_firmwareTableProviderSignature
{
    {"ACPI", 0x41435049},
    {"FIRM", 0x4649524D},
    {"RSMB", 0x52534D42}
};

typedef struct RawSMBIOSData
{
    BYTE    Used20CallingMethod;
    BYTE    SMBIOSMajorVersion;
    BYTE    SMBIOSMinorVersion;
    BYTE    DmiRevision;
    DWORD   Length;
    BYTE    SMBIOSTableData[];
} RawSMBIOSData, *PRawSMBIOSData;

typedef struct SMBIOSStructureHeader
{
    BYTE Type;
    BYTE FormattedAreaLength;
    WORD Handle;
} SMBIOSStructureHeader;

typedef struct SMBIOSBasboardInfoStructure
{
    BYTE Type;
    BYTE FormattedAreaLength;
    WORD Handle;
    BYTE Manufacturer;
    BYTE Product;
    BYTE Version;
    BYTE SerialNumber;
} SMBIOSBasboardInfoStructure;

static std::string parseRawSmbios(const BYTE* rawData, const DWORD rawDataSize)
{
    std::string serialNumber;
    std::stringstream ss;
    DWORD offset{0};

    while (offset < rawDataSize)
    {
        ss << std::hex << int(*(rawData+offset));
        ss << " ";

        if (!(*(rawData + offset)) && !(*(rawData + offset + 1)))
        {
            offset++;
            ss << std::hex << int(*(rawData+offset));
            ss << std::endl << std::endl;
        }

        offset++;
    }

    ss << std::endl << std::endl << std::endl << std::endl << std::endl;
    std::cout << ss.str();
    offset = 0;

    while (offset < rawDataSize && serialNumber.empty())
    {
        SMBIOSStructureHeader header{};
        memcpy(&header, rawData + offset, sizeof(SMBIOSStructureHeader));
        if (BASEBOARD_INFORMATION_TYPE == header.Type)
        {
            SMBIOSBasboardInfoStructure info{};
            memcpy(&info, rawData + offset, sizeof(SMBIOSBasboardInfoStructure));
            offset += info.FormattedAreaLength;
            std::cout << "info.FormattedAreaLength = " << std::hex << int(info.FormattedAreaLength) << std::endl;
            std::cout << "info.SerialNumber = " << std::hex << int(info.SerialNumber) << std::endl;

            for (BYTE i = 1; i < info.SerialNumber; ++i)
            {
                const char* tmp{reinterpret_cast<const char*>(rawData + offset)};
                std::cout << "Data = " << tmp << std::endl;
                const auto len{ strlen(tmp) };
                offset += len + sizeof(char);
            }
            serialNumber = reinterpret_cast<const char*>(rawData + offset);
        }
        else
        {
            offset += header.FormattedAreaLength;

            // Search for the end of the unformatted structure (\0\0)
            while (true)
            {
                if (!(*(rawData + offset)) && !(*(rawData + offset + 1)))
                {
                    offset += 2;
                    break;
                }

                offset++;
            }
        }
    }
    return serialNumber;
}

int main()
{
    std::string ret;

    while(1)
    {
        static auto pfnGetSystemFirmwareTable{Utils::getSystemFirmwareTableFunctionAddress()};

        if (pfnGetSystemFirmwareTable)
        {
            const auto size {pfnGetSystemFirmwareTable('RSMB', 0, nullptr, 0)};

            if (size)
            {
                const auto spBuff{std::make_unique<unsigned char[]>(size)};

                if (spBuff)
                {
                    // Get raw SMBIOS firmware table
                    if (pfnGetSystemFirmwareTable(gs_firmwareTableProviderSignature.at("RSMB"), 0, spBuff.get(), size) == size)
                    {
                        PRawSMBIOSData smbios{reinterpret_cast<PRawSMBIOSData>(spBuff.get())};
                        // Parse SMBIOS structures
                        ret = parseRawSmbios(smbios->SMBIOSTableData, size);
                        std::cout << "Serial number = " << ret << std::endl << std::endl << std::endl;
                    }
                }
            }
            else
            {
                std::cout << "Size = 0" << std::endl;
            }
        }

        sleep(10);
    }

    return 0;
}
