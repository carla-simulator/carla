// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using System.Linq;
using System.Diagnostics;
using System.Collections.Generic;
using UnrealBuildTool;
using EpicGames.Core;

public class CarlaTools :
	ModuleRules
{
    [CommandLine("-verbose")]
    bool Verbose = true;

    [CommandLine("-houdini")]
    bool EnableHoudini = false;

    [CommandLine("-nv-omniverse")]
    bool EnableNVIDIAOmniverse = false;

    [CommandLine("-osm2odr")]
    bool EnableOSM2ODR = true;

    [CommandLine("-carla-install-path")]
    string CarlaInstallPath = null;

    [CommandLine("-carla-dependencies-install-path")]
    string CarlaDependenciesPath = null;

    private static void LogFlagStatus(string name, bool value)
    {
        var state = value ? "enabled" : "disabled";
        Console.WriteLine(string.Format("{0} is {1}.", name, state));
    }

    public CarlaTools(ReadOnlyTargetRules Target) :
		base(Target)
    {
        LogFlagStatus("Houdini support", EnableHoudini);
        LogFlagStatus("NVIDIA Omniverse support", EnableNVIDIAOmniverse);

        if (EnableOSM2ODR)
        {
            PublicDefinitions.Add("WITH_OSM2ODR");
            PrivateDefinitions.Add("WITH_OSM2ODR");
        }

        var DirectoryInfo = new DirectoryInfo(ModuleDirectory);
        for (int i = 0; i != 6; ++i)
            DirectoryInfo = DirectoryInfo.Parent;
        var WorkspacePath = DirectoryInfo.ToString();
        Debug.Assert(WorkspacePath != null && !Directory.Exists(WorkspacePath));

        if (CarlaInstallPath == null)
        {
            Console.WriteLine("\"-carla-install-path\" was not specified, inferring...");
            CarlaInstallPath = Path.Combine(WorkspacePath, "Install");
            if (!Directory.Exists(CarlaInstallPath))
                throw new DirectoryNotFoundException("Could not infer CARLA install directory.");
            Console.WriteLine("Using \"" + CarlaInstallPath + "\" as the CARLA install path.");
        }

        if (CarlaDependenciesPath == null)
        {
            Console.WriteLine("\"-carla-dependencies-path\" was not specified, inferring...");
            CarlaDependenciesPath = Path.Combine(WorkspacePath, "Build", "Dependencies");
            if (!Directory.Exists(CarlaDependenciesPath))
                throw new DirectoryNotFoundException("Could not infer CARLA dependencies directory.");
            Console.WriteLine("Using \"" + CarlaDependenciesPath + "\" as the CARLA depenencies install path.");
        }

        Console.WriteLine("Current module directory: " + ModuleDirectory);

        bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = bEnableExceptions || IsWindows;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "ProceduralMeshComponent",
            "MeshDescription",
            "RawMesh",
            "AssetTools",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"Blutility",
			"UMG",
			"EditorScriptingUtilities",
			"Landscape",
			"Foliage",
            "FoliageEdit",
            "MeshMergeUtilities",
			"Carla",
			"StaticMeshDescription",
			"Json",
			"JsonUtilities",
			"Networking",
			"Sockets",
			"HTTP",
			"RHI",
			"RenderCore",
			"MeshMergeUtilities",
			"StreetMapImporting",
			"StreetMapRuntime",
            "Chaos",
            "ChaosVehicles"
        });

		if (EnableHoudini)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
			{
				"HoudiniEngine",
				"HoudiniEngineEditor",
				"HoudiniEngineRuntime"
			});
		}

		if (EnableNVIDIAOmniverse)
        {
            PublicDefinitions.Add("WITH_OMNIVERSE");
            PrivateDefinitions.Add("WITH_OMNIVERSE");

            PrivateDependencyModuleNames.AddRange(new string[]
			{
				"OmniverseUSD",
				"OmniverseRuntime"
			});
        }

        if (IsWindows)
        {
            PublicDefinitions.Add("NOMINMAX");
            PublicDefinitions.Add("VC_EXTRALEAN");
            PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        }

        PrivateDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
        PrivateDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
        PrivateDefinitions.Add("ASIO_NO_EXCEPTIONS");
        PrivateDefinitions.Add("BOOST_NO_EXCEPTIONS");
        PrivateDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
        PrivateDefinitions.Add("PUGIXML_NO_EXCEPTIONS");

        var LibraryPrefix = IsWindows ? "" : "lib";
        var LibrarySuffix = IsWindows ? ".lib" : ".a";

        var LibCarlaInstallPath = Path.Combine(CarlaInstallPath, "LibCarla");
        var DependenciesInstallPath = CarlaDependenciesPath;

        Func<string, string> GetLibraryName = name =>
        {
            return LibraryPrefix + name + LibrarySuffix;
        };

        Func<string, string, string[]> FindLibraries = (name, pattern) =>
        {
            var InstallPath = Path.Combine(DependenciesInstallPath, name + "-install");
            var LibPath = Path.Combine(InstallPath, "lib");
            var Candidates = Directory.GetFiles(LibPath, GetLibraryName(pattern));
            if (Candidates.Length == 0)
                throw new FileNotFoundException(string.Format("Could not find any matching libraries for \"{0}\" using pattern \"{1}\"", name, pattern));
            Array.Sort(Candidates);
            return Candidates;
        };

        Func<string, string> GetIncludePath = name => Path.Combine(DependenciesInstallPath, name + "-install", "include");

        // LibCarla
        var LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");
        var LibCarlaLibPath = Path.Combine(LibCarlaInstallPath, "lib");
        var LibCarlaServerPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-server"));
        var LibCarlaClientPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-client"));

        // Boost

        var BoostIncludePath = Path.Combine(DependenciesInstallPath, "boost-install", "include");
        var BoostLibraries = new List<string>();

        var BoostLibraryPatterns = new string[]
        {
            "libboost_atomic*",
            "libboost_date_time*",
            "libboost_filesystem*",
            "libboost_numpy*",
            "libboost_python*",
            "libboost_system*",
        };

        foreach (var Pattern in BoostLibraryPatterns)
        {
            var Candidates = FindLibraries("boost", Pattern);
            if (Candidates.Length != 0)
                BoostLibraries.Add(Candidates[0]);
        }

        var SQLiteBuildPath = Path.Combine(DependenciesInstallPath, "sqlite-build");
        var SQLiteLibraryCandidates = Directory.GetFiles(SQLiteBuildPath, GetLibraryName("sqlite*"));
        if (SQLiteLibraryCandidates.Length == 0)
            throw new FileNotFoundException("Could not find any matching libraries for SQLite");
        var RPCLibCandidates = FindLibraries("rpclib", "rpc");
        var XercesCCandidates = FindLibraries("xercesc", "xerces-c*");
        var PROJCandidates = FindLibraries("proj", "proj");
        var ZlibCandidates = FindLibraries("zlib", "zlibstatic*");

        var AdditionalLibraries = new List<string>
        {
            SQLiteLibraryCandidates[0],
            RPCLibCandidates[0],
            XercesCCandidates[0],
            PROJCandidates[0],
            ZlibCandidates[0],
        };

        if (EnableOSM2ODR)
            AdditionalLibraries.Add(FindLibraries("sumo", "*osm2odr")[0]);

        PublicIncludePaths.Add(ModuleDirectory);
        PublicIncludePaths.Add(LibCarlaIncludePath);

        PublicIncludePaths.AddRange(new string[]
        {
            BoostIncludePath,
            GetIncludePath("boost"),
            GetIncludePath("rpclib"),
            GetIncludePath("xercesc"),
            GetIncludePath("sumo"),
            GetIncludePath("zlib"),
        });

        PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
        PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
        PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
        PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
        PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
        PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");

        if (IsWindows)
        {
            PublicDefinitions.Add("NOMINMAX");
            PublicDefinitions.Add("VC_EXTRALEAN");
            PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        }

        if (Verbose)
        {
            Console.WriteLine("Additional CARLA libraries:");
            foreach (var e in AdditionalLibraries)
                Console.WriteLine(" - " + e);
        }

        PublicAdditionalLibraries.AddRange(AdditionalLibraries);
    }
}
