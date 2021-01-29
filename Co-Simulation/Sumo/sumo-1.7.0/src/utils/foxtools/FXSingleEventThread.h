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
/// @file    FXSingleEventThread.h
/// @author  unknown_author
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004-03-19
///
//
/****************************************************************************/

#ifndef FXSingleEventThread_h
#define FXSingleEventThread_h
#include <config.h>

#include <fx.h>
#include <FXThread.h>
#include "fxexdefs.h"

class MFXInterThreadEventClient;

class FXSingleEventThread : public FXObject, public FXThread {
    FXDECLARE(FXSingleEventThread)

private:
    FXEX::FXThreadEventHandle event;

private:
    FXSingleEventThread(const FXSingleEventThread&);
    FXSingleEventThread& operator=(const FXSingleEventThread&);

public:
    enum {
        ID_THREAD_EVENT
    };

public:
    long onThreadSignal(FXObject*, FXSelector, void*);
    long onThreadEvent(FXObject*, FXSelector, void*);

public:
    FXSingleEventThread(FXApp* a, MFXInterThreadEventClient* client);

    void signal();

    void signal(FXuint seltype);

    virtual FXint run() {
        return 0;
    }

    virtual ~FXSingleEventThread();

    static void sleep(long ms);

private:
    FXApp* myApp;
    MFXInterThreadEventClient* myClient;

protected:
    FXSingleEventThread() { }
};


#endif
