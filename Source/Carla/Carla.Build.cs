// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Carla : ModuleRules
{
  public Carla(TargetInfo Target)
  {
    string CarlaServerIncludePath = "CarlaServer/include";
    string CarlaServerLibPath = "CarlaServer/lib";

    PublicIncludePaths.AddRange(
      new string[] {
        "Carla/Public",
        CarlaServerIncludePath
        // ... add public include paths required here ...
      }
      );


    PrivateIncludePaths.AddRange(
      new string[] {
        "Carla/Private",
        CarlaServerIncludePath
        // ... add other private include paths required here ...
      }
      );


    PublicDependencyModuleNames.AddRange(
      new string[]
      {
        "Core",
        // ... add other public dependencies that you statically link with here ...
      }
      );


    PrivateDependencyModuleNames.AddRange(
      new string[]
      {
        "CoreUObject",
        "Engine",
        "Slate",
        "SlateCore",
        "PhysXVehicles",
        // ... add private dependencies that you statically link with here ...
      }
      );


    DynamicallyLoadedModuleNames.AddRange(
      new string[]
      {
        // ... add any modules that your module loads dynamically here ...
      }
      );

    PublicAdditionalLibraries.AddRange(
      new string[]
      {
        Path.Combine(ModuleDirectory, "..", CarlaServerLibPath, "libcarla_server.a")
        // ... add any modules that your module loads dynamically here ...
      }
      );
  }
}
