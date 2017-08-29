// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Carla : ModuleRules
{
  public Carla(ReadOnlyTargetRules Target) : base(Target)
  {
    PublicIncludePaths.AddRange(
      new string[] {
        // ... add public include paths required here ...
      }
      );

    PrivateIncludePaths.AddRange(
      new string[] {
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
        "AIModule",
        "CoreUObject",
        "Engine",
        "PhysXVehicles",
        "Slate",
        "SlateCore"
        // ... add private dependencies that you statically link with here ...
      }
      );

    DynamicallyLoadedModuleNames.AddRange(
      new string[]
      {
        // ... add any modules that your module loads dynamically here ...
      }
      );

    AddCarlaServerDependency(Target);
  }

  private bool IsWindows(ReadOnlyTargetRules Target)
  {
    return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
  }

  private bool UseDebugLibs(ReadOnlyTargetRules Target)
  {
    if (IsWindows(Target))
    {
      // In Windows, Unreal uses the Release C++ Runtime (CRT) even in debug
      // mode, so unless we recompile the engine we cannot link the debug
      // libraries.
      return false;
    }
    else
    {
      return false;
    }
  }

  delegate string ADelegate(string s);

  private void AddCarlaServerDependency(ReadOnlyTargetRules Target)
  {
    string CarlaServerInstallPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Util/Install"));

    string CarlaServerLib;
    if (UseDebugLibs(Target))
    {
      CarlaServerLib = "carlaserverd";
    }
    else
    {
      CarlaServerLib = "carlaserver";
    }

    ADelegate GetLibName = (string BaseName) => {
      if (IsWindows(Target))
      {
        return BaseName + ".lib";
      }
      else
      {
        return "lib" + BaseName + ".a";
      }
    };

    // Link dependencies.
    PublicAdditionalLibraries.Add(Path.Combine(CarlaServerInstallPath, "lib", GetLibName("c++abi")));
    PublicAdditionalLibraries.Add(Path.Combine(CarlaServerInstallPath, "lib", GetLibName("boost_system")));
    PublicAdditionalLibraries.Add(Path.Combine(CarlaServerInstallPath, "lib", GetLibName("protobuf")));
    PublicAdditionalLibraries.Add(Path.Combine(CarlaServerInstallPath, "lib", GetLibName(CarlaServerLib)));

    // Include path.
    string CarlaServerIncludePath = Path.Combine(CarlaServerInstallPath, "include");
    PublicIncludePaths.Add(CarlaServerIncludePath);
    PrivateIncludePaths.Add(CarlaServerIncludePath);
  }
}
