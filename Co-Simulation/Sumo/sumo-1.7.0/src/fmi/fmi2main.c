/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2020-2020 German Aerospace Center (DLR) and others.
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
/// @file    fmi2main.c
/// @author  Robert Hilbrich
/// @date    Tue, 03 Mar 2020
///
// Implementation of the FMI2 to sumolib bridge features
/****************************************************************************/

#define FMI_VERSION 2

#include <foreign/fmi/fmi2Functions.h>

#include "libsumocpp2c.h"
#include "fmi2main.h"


void 
logError(ModelInstance *comp, const char *message, ...) {
    if (!comp->logErrors) return;

    va_list args;
    va_start(args, message);
    logMessage(comp, fmi2Error, "logStatusError", message, args);
    va_end(args); 
}

void 
logMessage(ModelInstance *comp, int status, const char *category, const char *message, va_list args) {
    va_list args1;
    size_t len = 0;
    char *buf = "";
    
    va_copy(args1, args);
    len = vsnprintf(buf, len, message, args1);
    va_end(args1);
    
    va_copy(args1, args);
    buf = comp->allocateMemory(len + 1, sizeof(char));
    vsnprintf(buf, len + 1, message, args);
    va_end(args1);
    
    comp->logger(comp->componentEnvironment, comp->instanceName, status, category, buf);
    
    comp->freeMemory(buf);
}

// Retrieve the integer value for a single variable
fmi2Status
getInteger(ModelInstance* comp, fmi2ValueReference vr, int* value) {
 
    // Do we need the pointer to comp here?
    switch (vr) {
        case 0:
            *value = libsumo_vehicle_getIDCount();
            return fmi2OK;
        default:
            return fmi2Error;
    }
}

fmi2Status 
step(ModelInstance *comp, double tNext) {
    libsumo_step(tNext);
    return fmi2OK;
}
