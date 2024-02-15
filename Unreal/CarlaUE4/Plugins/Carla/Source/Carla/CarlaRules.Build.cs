// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class CarlaRules : ModuleRules
{
  protected string LibCarlaInstallPath;
  protected bool UsingRos2 = false;

  public CarlaRules(ReadOnlyTargetRules Target, string LibCarlaInstallPathRelativeToModule) : base(Target)
  {
    LibCarlaInstallPath = Path.GetFullPath(Path.Combine(ModuleDirectory, LibCarlaInstallPathRelativeToModule));
  }

  protected bool IsWindows()
  {
    return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
  }

  protected bool UseDebugLibs()
  {
    if (IsWindows())
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

  protected void AddDllDependency(string PathToFolder, string DllName)
  {
    string Source = Path.Combine(PathToFolder, DllName);
    string Destination = Path.Combine("$(BinaryOutputDir)", DllName);
    RuntimeDependencies.Add(Destination, Source);
  }

  protected void AddBoostLibs()
  {
    string [] files = Directory.GetFiles(Path.Combine(LibCarlaInstallPath, "lib"), "*boost*.lib");
    foreach (string file in files) PublicAdditionalLibraries.Add(file);
  }

  protected void AddStaticLibrary(string LibBaseName, string LinkBasePath = "", string LibPrefix = "", string LibPostfix = "") {
    string linkBasePath = LinkBasePath;
    if ( linkBasePath == "" )
    {
      linkBasePath = Path.GetFullPath(Path.Combine(LibCarlaInstallPath, "lib"));
    }

    string libPrefix = LibPrefix;
    if ( libPrefix == "" )
    {
      if (IsWindows())
      {
        libPrefix = "";
      }
      else
      {
        libPrefix = "lib";
      }
    }

    string libPostfix = LibPostfix;
    if ( libPostfix == "" )
    {
      if (IsWindows())
      {
        libPostfix = ".lib";
      }
      else
      {
        libPostfix = ".a";
      }
    }

    string linkLibrary= Path.Combine(linkBasePath, libPrefix + LibBaseName + libPostfix);
    PublicAdditionalLibraries.Add(linkLibrary);
  }
  protected void AddDynamicLibrary(string LibBaseName, string LinkBasePath = "", string LibPrefix = "", string LibPostfix = "")
  {
    string linkBasePath = LinkBasePath;
    if ( linkBasePath == "" )
    {
      linkBasePath = Path.GetFullPath(Path.Combine(LibCarlaInstallPath, "lib"));
    }

    string libPrefix;
    if (IsWindows())
    {
      libPrefix = "";
    }
    else
    {
      libPrefix = "lib";
    }
    if ( LibPrefix != "" )
    {
      libPrefix = LibPrefix;
    }

    string libPostfix;
    if (IsWindows())
    {
      libPostfix = ".dll";
    }
    else
    {
      libPostfix = ".so";
    }
    if (LibPostfix != "")
    {
      libPostfix = LibPostfix;
    }

    string linkLibrary= Path.Combine(linkBasePath, libPrefix + LibBaseName + libPostfix);
    PublicAdditionalLibraries.Add(linkLibrary);
    RuntimeDependencies.Add(linkLibrary);
    if ( IsWindows() )
    {
      PublicDelayLoadDLLs.Add(linkLibrary);
    }
  }

  protected void addOsmToODR()
  {
      AddStaticLibrary( "sqlite3");
      AddStaticLibrary( "proj");
      AddStaticLibrary( "osm2odr");
      if ( IsWindows() )
      {
        AddStaticLibrary( "xerces-c_3");
        AddStaticLibrary( "zlibstatic");
      }
      else
      {
        AddStaticLibrary( "xerces-c");
      }
  }

  protected void addROS2()
  {
    if (UsingRos2) 
    {
      AddStaticLibrary( "foonathan_memory-0.7.3");
      if ( IsWindows() )
      {
        AddStaticLibrary( "libfastcdr-1.1");
        AddStaticLibrary( "libfastrtps-2.11");
      }
      else
      {
        AddStaticLibrary( "fastcdr");
        AddStaticLibrary( "fastrtps");
      }
      PrivateIncludePaths.Add(Path.Combine(LibCarlaInstallPath, "include", "carla", "ros2", "ros_types"));
      PublicDefinitions.Add("WITH_ROS2");
      PrivateDefinitions.Add("WITH_ROS2");
    }
  }
}
