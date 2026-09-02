// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DigitalTwinsBaseWidget.h"
#include "OpenDriveToMap.h"
#include "Misc/FileHelper.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/Paths.h"
#include "CarlaTools.h"
#include "HAL/PlatformFileManager.h"

static UOpenDriveToMap* OpenDriveToMapObject = nullptr;

UOpenDriveToMap* UDigitalTwinsBaseWidget::InitializeOpenDriveToMap(TSubclassOf<UOpenDriveToMap> BaseClass){
  if( OpenDriveToMapObject == nullptr && BaseClass != nullptr ){
    UE_LOG(LogCarlaTools, Error, TEXT("Creating New Object") );
    OpenDriveToMapObject = NewObject<UOpenDriveToMap>(this, BaseClass);
  }
  return OpenDriveToMapObject;
}

UOpenDriveToMap* UDigitalTwinsBaseWidget::GetOpenDriveToMap(){
  return OpenDriveToMapObject;
}

void UDigitalTwinsBaseWidget::SetOpenDriveToMap(UOpenDriveToMap* ToSet){
  OpenDriveToMapObject = ToSet;
}

void UDigitalTwinsBaseWidget::DestroyOpenDriveToMap(){
  OpenDriveToMapObject->ConditionalBeginDestroy();
  OpenDriveToMapObject = nullptr;
}

void UDigitalTwinsBaseWidget::CreatePlugin(FString PluginName){

  const FString PluginsDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Plugins"));
  const FString PluginDir = FPaths::Combine(PluginsDir, PluginName);
  const FString ContentDir = FPaths::Combine(PluginDir, TEXT("Content"));
  const FString UPluginFile = FPaths::Combine(PluginDir, PluginName + TEXT(".uplugin"));

  // Ensure directories exist
  IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
  PlatformFile.CreateDirectoryTree(*ContentDir); // creates both plugin and content folders

  // Format the .uplugin JSON
  const FString UPluginContent = FString::Printf(
      TEXT("{\n")
      TEXT("  \"FileVersion\": 3,\n")
      TEXT("  \"Version\": 1,\n")
      TEXT("  \"VersionName\": \"1.0\",\n")
      TEXT("  \"FriendlyName\": \"%s\",\n")
      TEXT("  \"Description\": \"A content-only plugin generated in C++.\",\n")
      TEXT("  \"Category\": \"Content\",\n")
      TEXT("  \"CreatedBy\": \"AutoGenerator\",\n")
      TEXT("  \"CreatedByURL\": \"\",\n")
      TEXT("  \"DocsURL\": \"\",\n")
      TEXT("  \"CanContainContent\": true,\n")
      TEXT("  \"IsBetaVersion\": false,\n")
      TEXT("  \"IsExperimentalVersion\": false,\n")
      TEXT("  \"EnabledByDefault\": true,\n")
      TEXT("  \"Installed\": false\n")
      TEXT("}")
      , *PluginName
  );

  // Write the .uplugin file
  if (FFileHelper::SaveStringToFile(UPluginContent, *UPluginFile))
  {
      UE_LOG(LogTemp, Log, TEXT("Successfully created content-only plugin: %s"), *PluginName);
  }
  else
  {
      UE_LOG(LogTemp, Error, TEXT("Failed to create plugin file for: %s"), *PluginName);
  }

  FString PluginPath = FPaths::Combine(FPaths::ProjectPluginsDir(), PluginName);
  FString MountPoint = FString::Printf(TEXT("/%s/"), *PluginName);
  FString VirtualPath = FString::Printf(TEXT("/%s"), *PluginName);
  // Mount the content folder
  FPackageName::RegisterMountPoint(MountPoint, PluginPath / TEXT("Content"));
  TArray<FString> PathsToOpen = { VirtualPath };
  
  FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
  ContentBrowserModule.Get().SyncBrowserToFolders(PathsToOpen);
}
