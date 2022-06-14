/*
 * Copyright (C) 2015-2020
 * June 14, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef WINDOWS_HELPER_H
#define WINDOWS_HELPER_H


#include <winsock2.h>
#include <windows.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wcast-function-type"

namespace Utils
{

    typedef UINT (WINAPI* GetSystemFirmwareTable_t)(DWORD, DWORD, PVOID, DWORD);
    static GetSystemFirmwareTable_t getSystemFirmwareTableFunctionAddress()
    {
        GetSystemFirmwareTable_t ret{nullptr};
        auto hKernel32 { GetModuleHandle(TEXT("kernel32.dll")) };

        if (hKernel32)
        {
            ret = reinterpret_cast<GetSystemFirmwareTable_t>(GetProcAddress(hKernel32, "GetSystemFirmwareTable"));
        }

        return ret;
    }
}

#pragma GCC diagnostic pop

#endif // WINDOWS_HELPER_H

