/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    FXSingleEventThread.cpp
/// @author  unknown_author
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Jakob Erdmann
/// @date    2004-03-19
///
//
/****************************************************************************/

/* =========================================================================
 * included modules
 * ======================================================================= */
#include <config.h>

#include <utils/common/StdDefs.h>
#include "MFXInterThreadEventClient.h"
#include "FXSingleEventThread.h"
#include "fxexdefs.h"
#ifndef WIN32
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#else
#include <process.h>
#endif
#include <chrono>
#include <thread>

#ifndef WIN32
# define PIPE_READ 0
# define PIPE_WRITE 1
#endif

using namespace FXEX;

// Message map
FXDEFMAP(FXSingleEventThread) FXSingleEventThreadMap[] = {
    FXMAPFUNC(SEL_IO_READ, FXSingleEventThread::ID_THREAD_EVENT, FXSingleEventThread::onThreadSignal),
    FXMAPFUNC(SEL_THREAD, 0, FXSingleEventThread::onThreadEvent),
};
FXIMPLEMENT(FXSingleEventThread, FXObject, FXSingleEventThreadMap, ARRAYNUMBER(FXSingleEventThreadMap))



FXSingleEventThread::FXSingleEventThread(FXApp* a, MFXInterThreadEventClient* client)
    : FXObject(), myClient(client) {
    myApp = (a);
#ifndef WIN32
    FXMALLOC(&event, FXThreadEventHandle, 2);
    FXint res = pipe(event);
    FXASSERT(res == 0);
    UNUSED_PARAMETER(res); // only used for assertion
    myApp->addInput(event[PIPE_READ], INPUT_READ, this, ID_THREAD_EVENT);
#else
    event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    FXASSERT(event != NULL);
    myApp->addInput(event, INPUT_READ, this, ID_THREAD_EVENT);
#endif
}


FXSingleEventThread::~FXSingleEventThread() {
#ifndef WIN32
    myApp->removeInput(event[PIPE_READ], INPUT_READ);
    ::close(event[PIPE_READ]);
    ::close(event[PIPE_WRITE]);
    FXFREE(&event);
#else
    myApp->removeInput(event, INPUT_READ);
    ::CloseHandle(event);
#endif
}


void
FXSingleEventThread::signal() {
#ifndef WIN32
    FXuint seltype = SEL_THREAD;
    FXint res = ::write(event[PIPE_WRITE], &seltype, sizeof(seltype));
    UNUSED_PARAMETER(res); // to make the compiler happy
#else
    ::SetEvent(event);
#endif
}


void
FXSingleEventThread::signal(FXuint seltype) {
    UNUSED_PARAMETER(seltype);
#ifndef WIN32
    FXint res = ::write(event[PIPE_WRITE], &seltype, sizeof(seltype));
    UNUSED_PARAMETER(res); // to make the compiler happy
#else
    ::SetEvent(event);
#endif
}


long
FXSingleEventThread::onThreadSignal(FXObject*, FXSelector, void*) {
#ifndef WIN32
    FXuint seltype = SEL_THREAD;
    FXint res = ::read(event[PIPE_READ], &seltype, sizeof(seltype));
    UNUSED_PARAMETER(res); // to make the compiler happy
#else
    //FIXME need win32 support
#endif
    FXSelector sel = FXSEL(SEL_THREAD, 0);
    handle(this, sel, nullptr);
    return 0;
}


long
FXSingleEventThread::onThreadEvent(FXObject*, FXSelector, void*) {
    myClient->eventOccurred();
    /*
    FXuint seltype1 = FXSELTYPE(SEL_THREAD);
    if(myTarget && myTarget->handle(this,FXSEL(seltype1,mySelector),NULL)) {
    }
    FXuint seltype = FXSELTYPE(sel);
    return myTarget && myTarget->handle(this,FXSEL(seltype,mySelector),NULL);
    */
    return 1;
}


void
FXSingleEventThread::sleep(long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}



