#include <winsock2.h>
#include <windows.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

constexpr auto BASEBOARD_INFORMATION_TYPE{ 2 };
constexpr auto FILE_NAME { "Output" };

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
    DWORD offset{0};

    while (offset < rawDataSize && serialNumber.empty())
    {
        SMBIOSStructureHeader header{};
        memcpy(&header, rawData + offset, sizeof(SMBIOSStructureHeader));

        if (BASEBOARD_INFORMATION_TYPE == header.Type)
        {
            SMBIOSBasboardInfoStructure info{};
            memcpy(&info, rawData + offset, sizeof(SMBIOSBasboardInfoStructure));
            offset += info.FormattedAreaLength;

            for (BYTE i = 1; i < info.SerialNumber; ++i)
            {
                const char* tmp{reinterpret_cast<const char*>(rawData + offset)};
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
    std::stringstream name;
    auto index { 1ull };
    std::unique_ptr<unsigned char[]> spBuffer;

    while(1)
    {
        name.str("");
        name << FILE_NAME << index << ".txt";
        std::ifstream file { name.str(), std::ios_base::binary };

        if (file.is_open())
        {
            auto buffer { file.rdbuf() };

            if (nullptr != buffer)
            {
                const auto size { buffer->pubseekoff(0, file.end, file.in) };
                buffer->pubseekpos(0, file.in);
                spBuffer = std::make_unique<unsigned char[]>(size);
                buffer->sgetn(reinterpret_cast<char*>(spBuffer.get()), size);

                PRawSMBIOSData smbios{ reinterpret_cast<PRawSMBIOSData>(spBuffer.get()) };
                // Parse SMBIOS structures
                ret = parseRawSmbios(smbios->SMBIOSTableData, size);
                std::cout << "Serial number = " << ret << std::endl;
            }
        }
        else
        {
            std::cout << "Fail open file " << name.str() << std::endl;
            break;
        }

        index++;
    }

    return 0;
}
