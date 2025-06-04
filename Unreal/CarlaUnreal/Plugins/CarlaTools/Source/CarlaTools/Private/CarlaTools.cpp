// Copyright Epic Games, Inc. All Rights Reserved.

#include "CarlaTools.h"

#define LOCTEXT_NAMESPACE "FCarlaToolsModule"

DEFINE_LOG_CATEGORY(LogCarlaTools);

void FCarlaToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCarlaToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
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
