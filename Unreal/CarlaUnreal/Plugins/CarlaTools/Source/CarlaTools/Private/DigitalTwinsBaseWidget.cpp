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

#include "UObject/StrongObjectPtr.h"

// TStrongObjectPtr, not a raw static pointer: a bare `static UObject*` isn't
// a GC root, so once OpenDriveToMap's own RemoveFromRoot() fires (right
// after map generation finishes, see OpenDriveToMap.cpp), this pointer could
// be left dangling by the next garbage collection pass -- same use-after-free
// pattern found and fixed in Weather.cpp (confirmed there via gdb backtrace).
static TStrongObjectPtr<UOpenDriveToMap> OpenDriveToMapObject;

UOpenDriveToMap* UDigitalTwinsBaseWidget::InitializeOpenDriveToMap(TSubclassOf<UOpenDriveToMap> BaseClass){
  if( !OpenDriveToMapObject && BaseClass != nullptr ){
    UE_LOG(LogCarlaTools, Error, TEXT("Creating New Object") );
    OpenDriveToMapObject.Reset(NewObject<UOpenDriveToMap>(this, BaseClass));
  }
  return OpenDriveToMapObject.Get();
}

UOpenDriveToMap* UDigitalTwinsBaseWidget::GetOpenDriveToMap(){
  return OpenDriveToMapObject.Get();
}

void UDigitalTwinsBaseWidget::SetOpenDriveToMap(UOpenDriveToMap* ToSet){
  OpenDriveToMapObject.Reset(ToSet);
}

void UDigitalTwinsBaseWidget::DestroyOpenDriveToMap(){
  OpenDriveToMapObject->ConditionalBeginDestroy();
  OpenDriveToMapObject.Reset();
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
