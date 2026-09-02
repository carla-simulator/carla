// Copyright Epic Games, Inc. All Rights Reserved.

#include "CarlaTools.h"
#include "VehicleImporter.h"

#include <util/ue-header-guard-begin.h>
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include <util/ue-header-guard-end.h>

#include "TrafficLights/Widgets/TLWTrafficLightToolWidget.h"

#define LOCTEXT_NAMESPACE "FCarlaToolsModule"

DEFINE_LOG_CATEGORY(LogCarlaTools);

void FCarlaToolsModule::StartupModule()
{
	// carla-digitaltwins traffic-light authoring tool: LevelEditor menu entry + nomad tab.
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FCarlaToolsModule::AddMenuEntry));
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner("TrafficLightToolTab", FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&) {
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(STrafficLightToolWidget)
			];
	}))
	.SetDisplayName(LOCTEXT("TrafficLightToolTab", "Traffic Light Tool"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FCarlaToolsModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("TrafficLightToolTab");
}

void FCarlaToolsModule::AddMenuEntry(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(
		LOCTEXT("OpenTrafficLightTool", "Traffic Light Tool"),
		LOCTEXT("OpenTrafficLightToolTooltip", "Opens the Traffic Light Tool."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCarlaToolsModule::OpenTrafficLightToolTab))
	);
}

void FCarlaToolsModule::OpenTrafficLightToolTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FTabId("TrafficLightToolTab"));
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
