// Copyright Epic Games, Inc. All Rights Reserved.

#include "CarlaTools.h"
#include "VehicleImporter.h"

#define LOCTEXT_NAMESPACE "FCarlaToolsModule"

DEFINE_LOG_CATEGORY(LogCarlaTools);

void FCarlaToolsModule::StartupModule()
{
	// Module startup only ensures the class is available, doesn't auto-start server
}

void FCarlaToolsModule::ShutdownModule()
{
	// No automatic cleanup needed here
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCarlaToolsModule, CarlaTools)

#ifdef LIBCARLA_NO_EXCEPTIONS
#include <util/disable-ue4-macros.h>
#include <carla/Exception.h>
#include <util/enable-ue4-macros.h>

#include <exception>

namespace carla {

  void throw_exception(const std::exception &e) {
    UE_LOG(LogCarlaTools, Fatal, TEXT("Exception thrown: %s"), UTF8_TO_TCHAR(e.what()));
    // It should never reach this part.
    std::terminate();
  }

} // namespace carla 
#endif
