// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "WheelParentClassFixer.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EditorAssetLibrary.h"
#include "Engine/Blueprint.h"
#include "EditorClassUtils.h"

AWheelParentClassFixer::AWheelParentClassFixer()
{
  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
  SetRootComponent(SceneComponent);
}

void AWheelParentClassFixer::FixWheelClassesParentActor() 
{
  FString MyPath = FPaths::ConvertRelativePathToFull(FPaths::GameUserDeveloperDir());

  TArray<FString> Out;
  FString FullPath;
  MyPath.ParseIntoArray(Out, TEXT("/"), true);

  for (int i = 0; i < Out.Num() - 2; i++)
  {
    if (i == 0)
    {
      FullPath += Out[i];
    }
    else
    {
      FullPath += "/" + Out[i];
    }
  }

  FullPath += "/Carla/Blueprints/";

  TArray<FString> FoundBlueprintClassesPath;

  if (FPaths::DirectoryExists(FullPath))
  {
    FFileManagerGeneric::Get().FindFilesRecursive(FoundBlueprintClassesPath, *FullPath, TEXT("*"), true, true, true);
    for (int i = 0; i < FoundBlueprintClassesPath.Num(); i++)
    {
      //UE_LOG(LogTemp, Warning, TEXT("Found Folder: %s"), *FoundBlueprintClassesPath[i]);
      TArray<FString> CurrentPathSplitted;
      FString CurrentPath = FoundBlueprintClassesPath[i];
      CurrentPath.ParseIntoArray(CurrentPathSplitted,TEXT("/"), true );

      FString LastPartOfPath = CurrentPathSplitted.Last();
      LastPartOfPath.RemoveFromEnd(".uasset", ESearchCase::IgnoreCase);

      FString ReversedPath = "";

      for (int j= CurrentPathSplitted.Num() - 1; j >= 0; j--)
      {
        if(CurrentPathSplitted[j] == "Content") break;

        ReversedPath += CurrentPathSplitted[j] + "/";
      }

      TArray<FString> ReversedPathSplitted;
      FString FixedPath = "";
      ReversedPath.ParseIntoArray(ReversedPathSplitted, TEXT("/"), true);

      for (int j = ReversedPathSplitted.Num() - 1; j >= 0; j--)
      {
        FixedPath += ReversedPathSplitted[j] + "/";
      }

      FixedPath.InsertAt(0, "Blueprint'/Game/");

      FixedPath.RemoveFromEnd(".uasset/", ESearchCase::IgnoreCase);
      
      FixedPath += "." + LastPartOfPath + "'";

      if (LastPartOfPath.StartsWith("BP") && ((LastPartOfPath.Contains("FW") || LastPartOfPath.Contains("RW")) || LastPartOfPath.Contains("Wheel")))
      {
        UObject* Obj = UEditorAssetLibrary::LoadAsset(FixedPath);
        UBlueprint* BP = Cast<UBlueprint>(Obj);

        if(IsValid(BP))
        {
          if(BP->ParentClass == nullptr)
          {
            BP->ParentClass = NewParentClass;

            //Here I just Compile the blueprint by code
            FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::SkipSave);

            //And here the blueprint will be reload because it changed its parent class
            FBlueprintEditorUtils::RefreshAllNodes(BP);
            FBlueprintEditorUtils::MarkBlueprintAsModified(BP);
          }
        }
      }
    }
  }
  else
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Path don�t exist!");
  }
}
