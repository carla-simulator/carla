// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "OpenDriveToMap.h"
#include "Components/Button.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"

void UOpenDriveToMap::NativeConstruct()
{
  Super::NativeConstruct();
  if( IsValid(ChooseFileButon) ){
    ChooseFileButon->OnClicked.AddDynamic( this, &UOpenDriveToMap::OpenFileDialog );
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("YES Bind on Button"));
  }else{
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("NOT Bind on Button"));

  }
}

void UOpenDriveToMap::NativeDestruct()
{
  Super::NativeDestruct();
  if( IsValid(ChooseFileButon) ){
    ChooseFileButon->OnClicked.RemoveDynamic( this, &UOpenDriveToMap::OpenFileDialog );
  }

}


void UOpenDriveToMap::OpenFileDialog()
{
  TArray<FString> OutFileNames;
  void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
  IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
  if (DesktopPlatform)
  {
    DesktopPlatform->OpenFileDialog(ParentWindowPtr, "Select xodr file", FPaths::ProjectDir(), FString(""), ".xodr", 0, OutFileNames);
  }
  for(FString& CurrentString : OutFileNames)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("FileObtained %s"), *CurrentString );
  }

}
