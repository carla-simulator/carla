/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    SysUtils.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Tue, 29.05.2005
///
// A few system-specific functions
/****************************************************************************/
#include <config.h>

#include <stdlib.h>
#include "SysUtils.h"

#ifndef WIN32
#include <sys/time.h>
#else
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif


// ===========================================================================
// member method definitions
// ===========================================================================
long
SysUtils::getCurrentMillis() {
#ifndef WIN32
    timeval current;
    gettimeofday(&current, 0);
    long nanosecs =
        (long) current.tv_sec * 1000L + (long) current.tv_usec / 1000L;
    return nanosecs;
#else
    LARGE_INTEGER val, val2;
    BOOL check = QueryPerformanceCounter(&val);
    check = QueryPerformanceFrequency(&val2);
    return (long)(val.QuadPart * 1000 / val2.QuadPart);
#endif
}


#ifdef WIN32
long
SysUtils::getWindowsTicks() {
    return (long) GetTickCount();
}
#endif


unsigned long
SysUtils::runHiddenCommand(const std::string& cmd) {
#ifdef WIN32
    // code inspired by http://www.codeproject.com/Articles/2537/Running-console-applications-silently
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    unsigned long rc;

    memset(&StartupInfo, 0, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(STARTUPINFO);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;

    // "/c" option - Do the command then terminate the command window
    std::string winCmd = "CMD.exe /c " + cmd;
    char* args = new char[winCmd.size() + 1];
    args[0] = 0;
    strcpy(args, winCmd.c_str());
    if (!CreateProcess(nullptr, args, nullptr, nullptr, FALSE,
                       CREATE_NEW_CONSOLE, nullptr, nullptr, &StartupInfo, &ProcessInfo)) {
        delete[] args;
        return (unsigned long)GetLastError();
    }

    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
    if (!GetExitCodeProcess(ProcessInfo.hProcess, &rc)) {
        rc = 0;
    }

    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);

    delete[] args;
    return rc;
#else
    return (unsigned long)system(cmd.c_str());
#endif
}


/****************************************************************************/
