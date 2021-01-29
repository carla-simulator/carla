//
// Copyright (C) 2006-2016 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/veins.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#else
#include <sys/wait.h>
#endif

#include "veins/modules/mobility/traci/TraCILauncher.h"

using veins::TraCILauncher;

TraCILauncher::TraCILauncher(std::string commandLine)
{
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFOA);

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    char cmdline[32768];
    strncpy(cmdline, commandLine.c_str(), sizeof(cmdline));
    bool bSuccess = CreateProcess(0, cmdline, 0, 0, 1, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, 0, 0, &si, &pi);
    if (!bSuccess) {
        std::string msg = "undefined error";

        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID != 0) {

            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);

            std::string message(messageBuffer, size);
            LocalFree(messageBuffer);
            msg = message;
        }

        msg = std::string() + "Error launching TraCI server (\"" + commandLine + "\"): " + msg + ". Make sure you have set $PATH correctly.";

        throw cRuntimeError(msg.c_str());
    }
    else {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

#else
    pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_IGN);
        int r = system(commandLine.c_str());
        if (r == -1) {
            throw cRuntimeError("Running \"%s\" failed during system()", commandLine.c_str());
        }
        if (WEXITSTATUS(r) != 0) {
            throw cRuntimeError("Error launching TraCI server (\"%s\"): exited with code %d. Make sure you have set $PATH correctly.", commandLine.c_str(), WEXITSTATUS(r));
        }
        exit(1);
    }
#endif
}

TraCILauncher::~TraCILauncher()
{
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#else
    if (pid) {
        // send SIGINT
        kill(pid, 15);
    }
#endif
}
