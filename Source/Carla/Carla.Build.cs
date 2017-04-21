// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Carla : ModuleRules
{
  public Carla(TargetInfo Target)
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
        "CoreUObject",
        "Engine",
        "Slate",
        "SlateCore",
        "PhysXVehicles"
        // ... add private dependencies that you statically link with here ...
      }
      );

    DynamicallyLoadedModuleNames.AddRange(
      new string[]
      {
        // ... add any modules that your module loads dynamically here ...
      }
      );

    AddBoostDependency(Target);
    AddProtobufDependency(Target);
    AddLibPNGDependency(Target);
    AddCarlaServerDependency(Target);

    if (Target.Platform == UnrealTargetPlatform.Linux)
    {
      // Fails to link the std libraries.
      PublicAdditionalLibraries.Add("stdc++");
    }
  }

  private bool IsWindows(TargetInfo Target)
  {
    return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
  }

  private bool UseDebugLibs(TargetInfo Target)
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
      // On Linux on the other hand, we need to use the version of the libraries
      // without optimizations due to the crash in the std::string destructor.
      return true;
    }
  }

  private void AddBoostDependency(TargetInfo Target)
  {
    if (IsWindows(Target))
    {
      string BoostRoot = System.Environment.GetEnvironmentVariable("BOOST_ROOT");
      if (string.IsNullOrEmpty(BoostRoot) || !System.IO.Directory.Exists(BoostRoot))
      {
        throw new System.Exception("BOOST_ROOT is not defined, or points to a non-existant directory, please set this environment variable.");
      }
      PrivateIncludePaths.Add(BoostRoot);

      string BoostLib = Path.Combine(BoostRoot, "stage", "lib");
      if (!System.IO.Directory.Exists(BoostLib))
      {
        throw new System.Exception("Please build boost and make sure the libraries are at " + BoostLib + ". ");
      }
      bool found = System.IO.Directory.GetFiles(BoostLib, "libboost_system-*.lib").Length > 0;
      if (!found)
      {
        throw new System.Exception("Not finding libboost_system-*.lib in " + BoostLib + ".");
      }
      PublicLibraryPaths.Add(Path.Combine(BoostLib));
    }
    else
    {
      PublicAdditionalLibraries.Add("boost_system");
    }
  }

  private void AddProtobufDependency(TargetInfo Target)
  {
    if (IsWindows(Target))
    {
      string ProtobufRoot = System.Environment.GetEnvironmentVariable("PROTOBUF_ROOT");
      string ProtobufLib;
      if (UseDebugLibs(Target))
      {
        ProtobufRoot = Path.Combine(ProtobufRoot, "Debug");
        ProtobufLib = "libprotobufd.lib";
      }
      else
      {
        ProtobufRoot = Path.Combine(ProtobufRoot, "Release");
        ProtobufLib = "libprotobuf.lib";
      }
      if (string.IsNullOrEmpty(ProtobufRoot) || !System.IO.Directory.Exists(ProtobufRoot))
      {
        throw new System.Exception("PROTOBUF_ROOT is not defined, or points to a non-existant directory, please set this environment variable.");
      }
      PrivateIncludePaths.Add(Path.Combine(ProtobufRoot, "include"));
      PublicAdditionalLibraries.Add(Path.Combine(ProtobufRoot, "lib", ProtobufLib));
    }
    else
    {
      PublicAdditionalLibraries.Add("protobuf");
    }
  }

  private void AddLibPNGDependency(TargetInfo Target)
  {
    if (Target.Platform == UnrealTargetPlatform.Linux)
    {
      PublicAdditionalLibraries.Add("png");
    }
  }

  private void AddCarlaServerDependency(TargetInfo Target)
  {
    string CarlaServerIncludePath = Path.Combine(ModuleDirectory, "..", "CarlaServer/include");
    string CarlaServerLibPath = Path.Combine(ModuleDirectory, "..", "CarlaServer/lib");

    string CarlaServerLibBaseName;
    if (UseDebugLibs(Target))
    {
      CarlaServerLibBaseName = "carlaserverd";
    }
    else
    {
      CarlaServerLibBaseName = "carlaserver";
    }

    string CarlaServerLib;
    if (IsWindows(Target))
    {
      CarlaServerLib = Path.Combine(CarlaServerLibPath, CarlaServerLibBaseName + ".lib");
    }
    else
    {
      CarlaServerLib = Path.Combine(CarlaServerLibPath, "lib" + CarlaServerLibBaseName + ".a");
    }

    PublicIncludePaths.Add(CarlaServerIncludePath);
    PrivateIncludePaths.Add(CarlaServerIncludePath);
    PublicAdditionalLibraries.Add(CarlaServerLib);
  }
}
