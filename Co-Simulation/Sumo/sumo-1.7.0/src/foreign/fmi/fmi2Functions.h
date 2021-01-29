#ifndef fmi2Functions_h
#define fmi2Functions_h

/* This header file must be utilized when compiling a FMU.
   It defines all functions of the
         FMI 2.0.1 Model Exchange and Co-Simulation Interface.

   In order to have unique function names even if several FMUs
   are compiled together (e.g. for embedded systems), every "real" function name
   is constructed by prepending the function name by "FMI2_FUNCTION_PREFIX".
   Therefore, the typical usage is:

      #define FMI2_FUNCTION_PREFIX MyModel_
      #include "fmi2Functions.h"

   As a result, a function that is defined as "fmi2GetDerivatives" in this header file,
   is actually getting the name "MyModel_fmi2GetDerivatives".

   This only holds if the FMU is shipped in C source code, or is compiled in a
   static link library. For FMUs compiled in a DLL/sharedObject, the "actual" function
   names are used and "FMI2_FUNCTION_PREFIX" must not be defined.

   Revisions:
   - Sep. 29, 2019: License changed to 2-clause BSD License (without extensions)
   - Apr.  9, 2014: All prefixes "fmi" renamed to "fmi2" (decision from April 8)
   - Mar. 26, 2014: FMI_Export set to empty value if FMI_Export and FMI_FUNCTION_PREFIX
                    are not defined (#173)
   - Oct. 11, 2013: Functions of ModelExchange and CoSimulation merged:
                      fmiInstantiateModel , fmiInstantiateSlave  -> fmiInstantiate
                      fmiFreeModelInstance, fmiFreeSlaveInstance -> fmiFreeInstance
                      fmiEnterModelInitializationMode, fmiEnterSlaveInitializationMode -> fmiEnterInitializationMode
                      fmiExitModelInitializationMode , fmiExitSlaveInitializationMode  -> fmiExitInitializationMode
                      fmiTerminateModel, fmiTerminateSlave  -> fmiTerminate
                      fmiResetSlave -> fmiReset (now also for ModelExchange and not only for CoSimulation)
                    Functions renamed:
                      fmiUpdateDiscreteStates -> fmiNewDiscreteStates
   - June 13, 2013: Functions removed:
                       fmiInitializeModel
                       fmiEventUpdate
                       fmiCompletedEventIteration
                       fmiInitializeSlave
                    Functions added:
                       fmiEnterModelInitializationMode
                       fmiExitModelInitializationMode
                       fmiEnterEventMode
                       fmiUpdateDiscreteStates
                       fmiEnterContinuousTimeMode
                       fmiEnterSlaveInitializationMode;
                       fmiExitSlaveInitializationMode;
   - Feb. 17, 2013: Portability improvements:
                       o DllExport changed to FMI_Export
                       o FUNCTION_PREFIX changed to FMI_FUNCTION_PREFIX
                       o Allow undefined FMI_FUNCTION_PREFIX (meaning no prefix is used)
                    Changed function name "fmiTerminate" to "fmiTerminateModel" (due to #113)
                    Changed function name "fmiGetNominalContinuousState" to
                                          "fmiGetNominalsOfContinuousStates"
                    Removed fmiGetStateValueReferences.
   - Nov. 14, 2011: Adapted to FMI 2.0:
                       o Split into two files (fmiFunctions.h, fmiTypes.h) in order
                         that code that dynamically loads an FMU can directly
                         utilize the header files).
                       o Added C++ encapsulation of C-part, in order that the header
                         file can be directly utilized in C++ code.
                       o fmiCallbackFunctions is passed as pointer to fmiInstantiateXXX
                       o stepFinished within fmiCallbackFunctions has as first
                         argument "fmiComponentEnvironment" and not "fmiComponent".
                       o New functions to get and set the complete FMU state
                         and to compute partial derivatives.
   - Nov.  4, 2010: Adapted to specification text:
                       o fmiGetModelTypesPlatform renamed to fmiGetTypesPlatform
                       o fmiInstantiateSlave: Argument GUID     replaced by fmuGUID
                                              Argument mimetype replaced by mimeType
                       o tabs replaced by spaces
   - Oct. 16, 2010: Functions for FMI for Co-simulation added
   - Jan. 20, 2010: stateValueReferencesChanged added to struct fmiEventInfo (ticket #27)
                    (by M. Otter, DLR)
                    Added WIN32 pragma to define the struct layout (ticket #34)
                    (by J. Mauss, QTronic)
   - Jan.  4, 2010: Removed argument intermediateResults from fmiInitialize
                    Renamed macro fmiGetModelFunctionsVersion to fmiGetVersion
                    Renamed macro fmiModelFunctionsVersion to fmiVersion
                    Replaced fmiModel by fmiComponent in decl of fmiInstantiateModel
                    (by J. Mauss, QTronic)
   - Dec. 17, 2009: Changed extension "me" to "fmi" (by Martin Otter, DLR).
   - Dez. 14, 2009: Added eventInfo to meInitialize and added
                    meGetNominalContinuousStates (by Martin Otter, DLR)
   - Sept. 9, 2009: Added DllExport (according to Peter Nilsson's suggestion)
                    (by A. Junghanns, QTronic)
   - Sept. 9, 2009: Changes according to FMI-meeting on July 21:
                    meInquireModelTypesVersion     -> meGetModelTypesPlatform
                    meInquireModelFunctionsVersion -> meGetModelFunctionsVersion
                    meSetStates                    -> meSetContinuousStates
                    meGetStates                    -> meGetContinuousStates
                    removal of meInitializeModelClass
                    removal of meGetTime
                    change of arguments of meInstantiateModel
                    change of arguments of meCompletedIntegratorStep
                    (by Martin Otter, DLR):
   - July 19, 2009: Added "me" as prefix to file names (by Martin Otter, DLR).
   - March 2, 2009: Changed function definitions according to the last design
                    meeting with additional improvements (by Martin Otter, DLR).
   - Dec. 3 , 2008: First version by Martin Otter (DLR) and Hans Olsson (Dynasim).


   Copyright (C) 2008-2011 MODELISAR consortium,
                 2012-2019 Modelica Association Project "FMI"
                 All rights reserved.

   This file is licensed by the copyright holders under the 2-Clause BSD License
   (https://opensource.org/licenses/BSD-2-Clause):

   ----------------------------------------------------------------------------
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "fmi2TypesPlatform.h"
#include "fmi2FunctionTypes.h"
#include <stdlib.h>


/*
  Export FMI2 API functions on Windows and under GCC.
  If custom linking is desired then the FMI2_Export must be
  defined before including this file. For instance,
  it may be set to __declspec(dllimport).
*/
#if !defined(FMI2_Export)
  #if !defined(FMI2_FUNCTION_PREFIX)
    #if defined _WIN32 || defined __CYGWIN__
     /* Note: both gcc & MSVC on Windows support this syntax. */
        #define FMI2_Export __declspec(dllexport)
    #else
      #if __GNUC__ >= 4
        #define FMI2_Export __attribute__ ((visibility ("default")))
      #else
        #define FMI2_Export
      #endif
    #endif
  #else
    #define FMI2_Export
  #endif
#endif

/* Macros to construct the real function name
   (prepend function name by FMI2_FUNCTION_PREFIX) */
#if defined(FMI2_FUNCTION_PREFIX)
  #define fmi2Paste(a,b)     a ## b
  #define fmi2PasteB(a,b)    fmi2Paste(a,b)
  #define fmi2FullName(name) fmi2PasteB(FMI2_FUNCTION_PREFIX, name)
#else
  #define fmi2FullName(name) name
#endif

/***************************************************
Common Functions
****************************************************/
#define fmi2GetTypesPlatform         fmi2FullName(fmi2GetTypesPlatform)
#define fmi2GetVersion               fmi2FullName(fmi2GetVersion)
#define fmi2SetDebugLogging          fmi2FullName(fmi2SetDebugLogging)
#define fmi2Instantiate              fmi2FullName(fmi2Instantiate)
#define fmi2FreeInstance             fmi2FullName(fmi2FreeInstance)
#define fmi2SetupExperiment          fmi2FullName(fmi2SetupExperiment)
#define fmi2EnterInitializationMode  fmi2FullName(fmi2EnterInitializationMode)
#define fmi2ExitInitializationMode   fmi2FullName(fmi2ExitInitializationMode)
#define fmi2Terminate                fmi2FullName(fmi2Terminate)
#define fmi2Reset                    fmi2FullName(fmi2Reset)
#define fmi2GetReal                  fmi2FullName(fmi2GetReal)
#define fmi2GetInteger               fmi2FullName(fmi2GetInteger)
#define fmi2GetBoolean               fmi2FullName(fmi2GetBoolean)
#define fmi2GetString                fmi2FullName(fmi2GetString)
#define fmi2SetReal                  fmi2FullName(fmi2SetReal)
#define fmi2SetInteger               fmi2FullName(fmi2SetInteger)
#define fmi2SetBoolean               fmi2FullName(fmi2SetBoolean)
#define fmi2SetString                fmi2FullName(fmi2SetString)
#define fmi2GetFMUstate              fmi2FullName(fmi2GetFMUstate)
#define fmi2SetFMUstate              fmi2FullName(fmi2SetFMUstate)
#define fmi2FreeFMUstate             fmi2FullName(fmi2FreeFMUstate)
#define fmi2SerializedFMUstateSize   fmi2FullName(fmi2SerializedFMUstateSize)
#define fmi2SerializeFMUstate        fmi2FullName(fmi2SerializeFMUstate)
#define fmi2DeSerializeFMUstate      fmi2FullName(fmi2DeSerializeFMUstate)
#define fmi2GetDirectionalDerivative fmi2FullName(fmi2GetDirectionalDerivative)


/***************************************************
Functions for FMI2 for Model Exchange
****************************************************/
#define fmi2EnterEventMode                fmi2FullName(fmi2EnterEventMode)
#define fmi2NewDiscreteStates             fmi2FullName(fmi2NewDiscreteStates)
#define fmi2EnterContinuousTimeMode       fmi2FullName(fmi2EnterContinuousTimeMode)
#define fmi2CompletedIntegratorStep       fmi2FullName(fmi2CompletedIntegratorStep)
#define fmi2SetTime                       fmi2FullName(fmi2SetTime)
#define fmi2SetContinuousStates           fmi2FullName(fmi2SetContinuousStates)
#define fmi2GetDerivatives                fmi2FullName(fmi2GetDerivatives)
#define fmi2GetEventIndicators            fmi2FullName(fmi2GetEventIndicators)
#define fmi2GetContinuousStates           fmi2FullName(fmi2GetContinuousStates)
#define fmi2GetNominalsOfContinuousStates fmi2FullName(fmi2GetNominalsOfContinuousStates)


/***************************************************
Functions for FMI2 for Co-Simulation
****************************************************/
#define fmi2SetRealInputDerivatives      fmi2FullName(fmi2SetRealInputDerivatives)
#define fmi2GetRealOutputDerivatives     fmi2FullName(fmi2GetRealOutputDerivatives)
#define fmi2DoStep                       fmi2FullName(fmi2DoStep)
#define fmi2CancelStep                   fmi2FullName(fmi2CancelStep)
#define fmi2GetStatus                    fmi2FullName(fmi2GetStatus)
#define fmi2GetRealStatus                fmi2FullName(fmi2GetRealStatus)
#define fmi2GetIntegerStatus             fmi2FullName(fmi2GetIntegerStatus)
#define fmi2GetBooleanStatus             fmi2FullName(fmi2GetBooleanStatus)
#define fmi2GetStringStatus              fmi2FullName(fmi2GetStringStatus)

/* Version number */
#define fmi2Version "2.0"


/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers of header files */
   FMI2_Export fmi2GetTypesPlatformTYPE fmi2GetTypesPlatform;
   FMI2_Export fmi2GetVersionTYPE       fmi2GetVersion;
   FMI2_Export fmi2SetDebugLoggingTYPE  fmi2SetDebugLogging;

/* Creation and destruction of FMU instances */
   FMI2_Export fmi2InstantiateTYPE  fmi2Instantiate;
   FMI2_Export fmi2FreeInstanceTYPE fmi2FreeInstance;

/* Enter and exit initialization mode, terminate and reset */
   FMI2_Export fmi2SetupExperimentTYPE         fmi2SetupExperiment;
   FMI2_Export fmi2EnterInitializationModeTYPE fmi2EnterInitializationMode;
   FMI2_Export fmi2ExitInitializationModeTYPE  fmi2ExitInitializationMode;
   FMI2_Export fmi2TerminateTYPE               fmi2Terminate;
   FMI2_Export fmi2ResetTYPE                   fmi2Reset;

/* Getting and setting variables values */
   FMI2_Export fmi2GetRealTYPE    fmi2GetReal;
   FMI2_Export fmi2GetIntegerTYPE fmi2GetInteger;
   FMI2_Export fmi2GetBooleanTYPE fmi2GetBoolean;
   FMI2_Export fmi2GetStringTYPE  fmi2GetString;

   FMI2_Export fmi2SetRealTYPE    fmi2SetReal;
   FMI2_Export fmi2SetIntegerTYPE fmi2SetInteger;
   FMI2_Export fmi2SetBooleanTYPE fmi2SetBoolean;
   FMI2_Export fmi2SetStringTYPE  fmi2SetString;

/* Getting and setting the internal FMU state */
   FMI2_Export fmi2GetFMUstateTYPE            fmi2GetFMUstate;
   FMI2_Export fmi2SetFMUstateTYPE            fmi2SetFMUstate;
   FMI2_Export fmi2FreeFMUstateTYPE           fmi2FreeFMUstate;
   FMI2_Export fmi2SerializedFMUstateSizeTYPE fmi2SerializedFMUstateSize;
   FMI2_Export fmi2SerializeFMUstateTYPE      fmi2SerializeFMUstate;
   FMI2_Export fmi2DeSerializeFMUstateTYPE    fmi2DeSerializeFMUstate;

/* Getting partial derivatives */
   FMI2_Export fmi2GetDirectionalDerivativeTYPE fmi2GetDirectionalDerivative;


/***************************************************
Functions for FMI2 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
   FMI2_Export fmi2EnterEventModeTYPE               fmi2EnterEventMode;
   FMI2_Export fmi2NewDiscreteStatesTYPE            fmi2NewDiscreteStates;
   FMI2_Export fmi2EnterContinuousTimeModeTYPE      fmi2EnterContinuousTimeMode;
   FMI2_Export fmi2CompletedIntegratorStepTYPE      fmi2CompletedIntegratorStep;

/* Providing independent variables and re-initialization of caching */
   FMI2_Export fmi2SetTimeTYPE             fmi2SetTime;
   FMI2_Export fmi2SetContinuousStatesTYPE fmi2SetContinuousStates;

/* Evaluation of the model equations */
   FMI2_Export fmi2GetDerivativesTYPE                fmi2GetDerivatives;
   FMI2_Export fmi2GetEventIndicatorsTYPE            fmi2GetEventIndicators;
   FMI2_Export fmi2GetContinuousStatesTYPE           fmi2GetContinuousStates;
   FMI2_Export fmi2GetNominalsOfContinuousStatesTYPE fmi2GetNominalsOfContinuousStates;


/***************************************************
Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
   FMI2_Export fmi2SetRealInputDerivativesTYPE  fmi2SetRealInputDerivatives;
   FMI2_Export fmi2GetRealOutputDerivativesTYPE fmi2GetRealOutputDerivatives;

   FMI2_Export fmi2DoStepTYPE     fmi2DoStep;
   FMI2_Export fmi2CancelStepTYPE fmi2CancelStep;

/* Inquire slave status */
   FMI2_Export fmi2GetStatusTYPE        fmi2GetStatus;
   FMI2_Export fmi2GetRealStatusTYPE    fmi2GetRealStatus;
   FMI2_Export fmi2GetIntegerStatusTYPE fmi2GetIntegerStatus;
   FMI2_Export fmi2GetBooleanStatusTYPE fmi2GetBooleanStatus;
   FMI2_Export fmi2GetStringStatusTYPE  fmi2GetStringStatus;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* fmi2Functions_h */
