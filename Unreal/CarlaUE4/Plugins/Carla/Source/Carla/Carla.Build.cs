// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class Carla : ModuleRules
{
  bool UsingCarSim = false;
  bool UsingChrono = false;
  private bool IsWindows(ReadOnlyTargetRules Target)
  {
    return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
  }

  public Carla(ReadOnlyTargetRules Target) : base(Target)
  {
    PrivatePCHHeaderFile = "Carla.h";

    if (IsWindows(Target))
    {
      bEnableExceptions = true;
    }

    // Read config about carsim
    string CarlaPluginPath = Path.GetFullPath( ModuleDirectory );
    string ConfigDir =  Path.GetFullPath(Path.Combine(CarlaPluginPath, "../../../../Config/"));
    string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
    string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);
    foreach (string line in text)
    {
      if (line.Contains("CarSim ON"))
      {
        Console.WriteLine("Enabling carsim");
        UsingCarSim = true;
        PublicDefinitions.Add("WITH_CARSIM");
        PrivateDefinitions.Add("WITH_CARSIM");
      }
      if (line.Contains("Chrono ON"))
      {
        Console.WriteLine("Enabling chrono");
        UsingChrono = true;
        PublicDefinitions.Add("WITH_CHRONO");
        PrivateDefinitions.Add("WITH_CHRONO");
      }
    }

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
        "RenderCore",
        "RHI",
        "ProceduralMeshComponent"
        // ... add other public dependencies that you statically link with here ...
      }
      );
    if (UsingCarSim)
    {
      PublicDependencyModuleNames.AddRange(new string[] { "CarSim" });
    }

	 if (Target.Type == TargetType.Editor)
	 {
		PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
	 }

    PrivateDependencyModuleNames.AddRange(
      new string[]
      {
        "AIModule",
        "AssetRegistry",
        "CoreUObject",
        "Engine",
        "Foliage",
        "ImageWriteQueue",
        "Json",
        "JsonUtilities",
        "Landscape",
        "PhysX",
        "PhysXVehicles",
        "PhysXVehicleLib",
        "Slate",
        "SlateCore"
        // ... add private dependencies that you statically link with here ...
      }
      );
    if (UsingCarSim)
    {
      PrivateDependencyModuleNames.AddRange(new string[] { "CarSim" });
      PrivateIncludePathModuleNames.AddRange(new string[] { "CarSim" });
    }


    DynamicallyLoadedModuleNames.AddRange(
      new string[]
      {
        // ... add any modules that your module loads dynamically here ...
      }
      );

    AddCarlaServerDependency(Target);
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

  private void AddBoostLibs(string LibPath)
  {
    string [] files = Directory.GetFiles(LibPath, "*boost*.lib");
    foreach (string file in files) PublicAdditionalLibraries.Add(file);
  }

  private void AddCarlaServerDependency(ReadOnlyTargetRules Target)
  {
    string LibCarlaInstallPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../CarlaDependencies"));

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
    if (IsWindows(Target))
    {
      AddBoostLibs(Path.Combine(LibCarlaInstallPath, "lib"));
      PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("rpc")));

      if (UseDebugLibs(Target))
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server_debug")));
      }
      else
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server")));
      }
      if (UsingChrono)
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoEngine")));
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoEngine_vehicle")));
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoModels_vehicle")));
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoModels_robot")));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoEngine.dll"));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoEngine_vehicle.dll"));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoModels_vehicle.dll"));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoModels_robot.dll"));
        PublicDelayLoadDLLs.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoEngine.dll"));
        PublicDelayLoadDLLs.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoEngine_vehicle.dll"));
        PublicDelayLoadDLLs.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoModels_vehicle.dll"));
        PublicDelayLoadDLLs.Add(Path.Combine(LibCarlaInstallPath, "dll", "ChronoModels_robot.dll"));
      }
    }
    else
    {
      PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("rpc")));
      if (UseDebugLibs(Target))
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server_debug")));
      }
      else
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server")));
      }
      if (UsingChrono)
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoEngine.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoEngine_vehicle.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoModels_vehicle.so"));
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoModels_robot.so"));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoEngine.so"));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoEngine_vehicle.so"));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoModels_vehicle.so"));
        RuntimeDependencies.Add(Path.Combine(LibCarlaInstallPath, "lib/libChronoModels_robot.so"));
        bUseRTTI = true;
      }
    }

    // Include path.
    string LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");

    PublicIncludePaths.Add(LibCarlaIncludePath);
    PrivateIncludePaths.Add(LibCarlaIncludePath);

    PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
    PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
    PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
    PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
  }
}
