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
/// @file    fmi2Functions.c
/// @author  Robert Hilbrich
/// @date    Tue, 03 Mar 2020
///
// Implementation of the FMI2 interface functions
/****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <foreign/fmi/fmi2Functions.h>
#include "fmi2main.h"
#include "libsumocpp2c.h"

/* **********************************************************************************************
 * * IMPLEMENTATION OF GENERIC FUNCTIONALITY
 * **********************************************************************************************/

const char* fmi2GetVersion() {
    return fmi2Version;
}

const char* fmi2GetTypesPlatform() {
    return fmi2TypesPlatform;
}

/* ***********************************************************************************************
   * CREATION AND DESTRUCTION OF AN FMU
   ***********************************************************************************************/

/* The function returns a new instance of an FMU. If a null pointer is returned, then instantiation
   failed.*/
fmi2Component 
fmi2Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID,
                  fmi2String fmuResourceLocation, const fmi2CallbackFunctions *functions,
                  fmi2Boolean visible, fmi2Boolean loggingOn)
{
  
   allocateMemoryType funcAllocateMemory = (allocateMemoryType)functions->allocateMemory;

   ModelInstance* comp = (ModelInstance *) funcAllocateMemory(1, sizeof(ModelInstance));

   if (comp) {
    	comp->componentEnvironment = functions->componentEnvironment;
		
		/* Callback functions for specific logging, malloc and free;
		   we need callback functions because we cannot know, which functions
		   the environment will provide for us */
		comp->logger = (loggerType)functions->logger;
		comp->allocateMemory = (allocateMemoryType)functions->allocateMemory;
		comp->freeMemory = (freeMemoryType)functions->freeMemory;

		comp->instanceName = (char *)comp->allocateMemory(1 + strlen(instanceName), sizeof(char));
		strcpy((char *)comp->instanceName, (char *)instanceName);

		if (fmuResourceLocation) {
		 	comp->resourceLocation = (char *)comp->allocateMemory(1 + strlen(fmuResourceLocation), sizeof(char));
		 	strcpy((char *)comp->resourceLocation, (char *)fmuResourceLocation);
		} else {
		 	comp->resourceLocation = NULL;
		}

		comp->modelData = (ModelData *)comp->allocateMemory(1, sizeof(ModelData));
        
      	comp->logEvents = loggingOn;
      	comp->logErrors = true; // always log errors
	}

	return comp;
}

/* Disposes the given instance, unloads the loaded model, and frees all the allocated memory
and other resources that have been allocated by the functions of the FMU interface. */
void 
fmi2FreeInstance(fmi2Component c) {
	ModelInstance *comp = (ModelInstance *)c;

	/* Store the pointer to the freeMemory function, because we
	   are going to free comp as well */
	freeMemoryType freeMemoryFunc = comp->freeMemory;

	/* We want to free everything that we allocated in fmi2Instantiate */
	freeMemoryFunc((void *)comp->instanceName);
	freeMemoryFunc((void *)comp->resourceLocation); 
	freeMemoryFunc((void *)comp->modelData);
	freeMemoryFunc((void *)comp);
}

/* Define what should be logged - if logging is enabled globally */ 
fmi2Status 
fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t nCategories, const fmi2String categories[]) {
    
    ModelInstance *comp = (ModelInstance *)c;

	if (loggingOn) {
        for (size_t i = 0; i < nCategories; i++) {
            if (categories[i] == NULL) {
                logError(comp, "Log category[%d] must not be NULL", i);
                return fmi2Error;
            } else if (strcmp(categories[i], "logStatusError") == 0) {
                comp->logErrors = true;
            } else if (strcmp(categories[i], "logEvents") == 0) {
                comp->logEvents = true;
            } else {
                logError(comp, "Log category[%d] must be one of logEvents or logStatusError but was %s", i, categories[i]);
                return fmi2Error;
            }
        }
    } else {
		// Logging is disabled globally, no need for a more fine grained logging
        comp->logEvents = false;
        comp->logErrors = false;
    }

    return fmi2OK;
}

fmi2Status 
fmi2SetupExperiment(fmi2Component c, fmi2Boolean toleranceDefined, fmi2Real tolerance,
					fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime) {

    // ignore arguments: toleranceDefined, tolerance
    ModelInstance *comp = (ModelInstance *)c;

	// Store the start and stop times of the experiment
    comp->startTime = startTime;
	comp->stopTime = stopTime;

    return fmi2OK;
}

// Will be called after instantiation and after initial variables have been set 
fmi2Status 
fmi2EnterInitializationMode(fmi2Component c) {
    return fmi2OK;
}

// Informs the FMU to exit Initialization Mode
fmi2Status 
fmi2ExitInitializationMode(fmi2Component c) {
	libsumo_load();
    return fmi2OK;
}

// Informs the FMU that the simulation run is terminated
// --> let libsumo know, that we want to close the simulation
fmi2Status 
fmi2Terminate(fmi2Component c) {
    ModelInstance *comp = (ModelInstance *)c;
	libsumo_close();
    return fmi2OK;
}

// Is called by the environment to reset the FMU after a simulation run
fmi2Status 
fmi2Reset(fmi2Component c) {
    ModelInstance* comp = (ModelInstance *)c;
	// Should we set some start values?
    return fmi2OK;
}

// Implementation of the getter features
fmi2Status 
fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]) {

    ModelInstance *comp = (ModelInstance *)c;
	
	// Check for null pointer errors 
    if (nvr > 0 && (!vr || !value))
        return fmi2Error;

	fmi2Status status = fmi2OK;

	// Go through the list of arrays and save all requested values
	for (int i = 0; i < nvr; i++) { 
		fmi2Status s = getInteger(comp, vr[i], &(value[i])); 
		status = s > status ? s : status; 

		if (status > fmi2Warning) 
			return status; 
	} 

	return status;
}

fmi2Status 
fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {
	return fmi2Error;
}

fmi2Status 
fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]) {
	return fmi2Error;
}

fmi2Status 
fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String value[]) {
	return fmi2Error;
}

// Implementation of the setter features

fmi2Status 
fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]) {
	return fmi2Error;
}

fmi2Status 
fmi2SetReal (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]) {
	return fmi2Error;
}

fmi2Status 
fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]) {
	return fmi2Error;
}

fmi2Status 
fmi2SetString (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[]) {
	return fmi2Error;
}

/* Further functions for interpolation */
fmi2Status 
fmi2SetRealInputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], const fmi2Real value[]) {
	return fmi2Error; /* Ignoring - SUMO cannot interpolate inputs */
}

fmi2Status 
fmi2GetRealOutputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], fmi2Real value[]) {
    for (int i = 0; i < nvr; i++) 
		value[i] = 0;	/* We cannot compute derivatives of outputs */
	return fmi2Error;
}

/* Stepping */
fmi2Status 
fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint) {
    ModelInstance *comp = (ModelInstance *)c;

    if (communicationStepSize <= 0) {
        return fmi2Error;
    }

    return step(comp, currentCommunicationPoint + communicationStepSize);
}

fmi2Status 
fmi2CancelStep(fmi2Component c) {
    return fmi2Error; /* We will never have a modelStepInProgress state */
}

/* Status functions */
fmi2Status 
fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status *value) {
	return fmi2Discard;
}

fmi2Status 
fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real *value) {
	return fmi2Discard;
}

fmi2Status 
fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer *value) {
	return fmi2Discard;
}

fmi2Status 
fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean *value) {
	return fmi2Discard;
}

fmi2Status 
fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String *value) {
	return fmi2Discard;
}








