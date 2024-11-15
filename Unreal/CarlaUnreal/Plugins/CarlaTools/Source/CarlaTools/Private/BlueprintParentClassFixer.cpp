// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintParentClassFixer.h"

#include <util/ue-header-guard-begin.h>
#include "HAL/FileManagerGeneric.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EditorAssetLibrary.h"
#include "Engine/Blueprint.h"
#include "EditorClassUtils.h"
#include "VehicleAnimationInstance.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Animation/AnimBlueprint.h"
#include <util/ue-header-guard-end.h>

ABlueprintParentClassFixer::ABlueprintParentClassFixer()
{
  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
  SetRootComponent(SceneComponent);
}

void ABlueprintParentClassFixer::FixBlueprints()
{
  //We get the path of our development directory
  FString MyPath = FPaths::ConvertRelativePathToFull(FPaths::GameUserDeveloperDir());

  TArray<FString> Out;
  FString FullPath;
  MyPath.ParseIntoArray(Out, TEXT("/"), true);

  //Here we remove the two folders that we dont need, ("Carla/Development")
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

  //We add the rest of the route to the folder from which the assets will be scanned
  FullPath += PathToSearchClassOn;

  //Local Variable that will store our classes found.
  TArray<FString> FoundBlueprintClassesPath;

  if (FPaths::DirectoryExists(FullPath)) //We only make this proccess if the directory is even found.
  {
    FFileManagerGeneric::Get().FindFilesRecursive(FoundBlueprintClassesPath, *FullPath, TEXT("*"), true, true, true);

    //For each asset found we get his path, Since this returns a PC dependant route we have to tweak them
    for (int i = 0; i < FoundBlueprintClassesPath.Num(); i++)
    {
      //We split the path and remove the .uasset file extension
      TArray<FString> CurrentPathSplitted;
      FString CurrentPath = FoundBlueprintClassesPath[i];
      CurrentPath.ParseIntoArray(CurrentPathSplitted,TEXT("/"), true );

      FString LastPartOfPath = CurrentPathSplitted.Last();
      LastPartOfPath.RemoveFromEnd(".uasset", ESearchCase::IgnoreCase);

      FString ReversedPath = "";

      //We reverse the path to make it easier to remove the non needed part, Once we get to the content folder that is our needed path to load objects from
      for (int j= CurrentPathSplitted.Num() - 1; j >= 0; j--)
      {
        if(CurrentPathSplitted[j] == "Content") break;

        ReversedPath += CurrentPathSplitted[j] + "/";
      }

      TArray<FString> ReversedPathSplitted;
      FString FixedPath = "";
      ReversedPath.ParseIntoArray(ReversedPathSplitted, TEXT("/"), true);

      //We get the path in the right order
      for (int j = ReversedPathSplitted.Num() - 1; j >= 0; j--)
      {
        FixedPath += ReversedPathSplitted[j] + "/";
      }

      FString ClassPrefix = "";

      //Depending on which class we are looking for, we have to add the "Class reference name" before adding /Game/
      switch (ClassToFix)
      {
        case EFixClass::BLUEPRINT:
          ClassPrefix = "Blueprint'";
          FixedPath.InsertAt(0, "Blueprint'/Game/");

        break;

        case EFixClass::ANIMBLUEPRINT:
          ClassPrefix = "AnimBlueprint'";
          FixedPath.InsertAt(0, "AnimBlueprint'/Game/");
        break;
      }

      //We make sure to remove the uasset extension.
      FixedPath.RemoveFromEnd(".uasset/", ESearchCase::IgnoreCase);
      
      //We add a point and the name of the asset again since "AssetName.AssetName" is the reference format which we need for loading assets
      FixedPath += "." + LastPartOfPath + "'";

      //We load the asset from the path and get his full reference path
      UObject* Obj = UEditorAssetLibrary::LoadAsset(FixedPath);

      UBlueprint* BP = NewObject<UBlueprint>();
      UAnimBlueprint* AnimBP = NewObject<UAnimBlueprint>();

      UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
      FString ReferencePath = EditorAssetSubsystem->GetPathNameForLoadedAsset(Obj);

      ReferencePath.InsertAt(0, ClassPrefix);
      ReferencePath += "'";

      //If the names filters is not empty, make the cast and change the blueprint parent class of the blueprint or animation blueprint
      if (ContainsStringCondition.Num() > 0 && !PathToSearchClassOn.IsEmpty())
      {
        for (FString CurrentString : ContainsStringCondition)
        {
          if (ReferencePath.Contains(CurrentString))
          {
            switch (ClassToFix)
            {
              case EFixClass::BLUEPRINT:

                if (IsValid(BP))
                {
                  BP = Cast<UBlueprint>(Obj);
                  BP->ParentClass = NewParentClass;

                  //Here I just Compile the blueprint by code
                  FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::SkipSave);

                  //And here the blueprint will be reload because it changed its parent class
                  FBlueprintEditorUtils::RefreshAllNodes(BP);
                  FBlueprintEditorUtils::MarkBlueprintAsModified(BP);
                }
                else
                {
                  UE_LOG(LogTemp, Warning, TEXT("Blueprint was not valid"));
                }

              break;

              case EFixClass::ANIMBLUEPRINT:

                if (IsValid(AnimBP))
                {
                  AnimBP = Cast<UAnimBlueprint>(Obj);
                  AnimBP->ParentClass = NewParentClass;

                  //Here I just Compile the blueprint by code
                  FKismetEditorUtilities::CompileBlueprint(AnimBP, EBlueprintCompileOptions::SkipSave);

                  //And here the blueprint will be reload because it changed its parent class
                  FBlueprintEditorUtils::RefreshAllNodes(AnimBP);
                  FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);
                }
                else
                {
                  UE_LOG(LogTemp, Warning, TEXT("AnimBlueprint was not valid"));
                }
              break;
            }

            break;
          }
        }
      }
    }
  }
  else
  {
    UE_LOG(LogTemp, Warning, TEXT("Base path was not valid"));
  }
}
