// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;
using EpicGames.Core;
using System.Linq;
using System.Diagnostics;

public class CarlaTools :
	ModuleRules
{
    [CommandLine("-houdini")]
    bool EnableHoudini = false;

    [CommandLine("-nv-omniverse")]
    bool EnableNVIDIAOmniverse = false;

    [CommandLine("-unity")]
    bool EnableUnityBuild = false;

    [CommandLine("-carla-install-path")]
    string CarlaInstallPath = null;

    [CommandLine("-carla-dependencies-install-path")]
    string CarlaDependenciesPath = null;



    public static string FindLibrary(string SearchPath, params string[] Patterns)
    {
        foreach (var Pattern in Patterns)
        {
            var Candidates = Directory.GetFiles(SearchPath, Pattern);
            if (Candidates.Length == 0)
                continue;
            Array.Sort(Candidates);
            return Candidates[0];
        }

        string message = "Warning: Could not find any matches in \"";
        message += SearchPath;
        message += "\" for";
        foreach (var Pattern in Patterns)
        {
            message += " \"";
            message += Pattern;
            message += "\",";
        }
        if (Patterns.Length != 0)
            message.Remove(message.Length - 1, 1);
        message += '.';
        Console.WriteLine(message);

        return "";
    }

    public CarlaTools(ReadOnlyTargetRules Target) :
		base(Target)
    {
        bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;
        var DirectoryInfo = new DirectoryInfo(ModuleDirectory);
        for (int i = 0; i != 6; ++i)
            DirectoryInfo = DirectoryInfo.Parent;
        var WorkspacePath = DirectoryInfo.ToString();
        Debug.Assert(WorkspacePath != null && !Directory.Exists(WorkspacePath));

        if (CarlaInstallPath == null)
        {
            Console.WriteLine("\"-carla-install-path\" was not specified, inferring...");
            CarlaInstallPath = Path.Combine(WorkspacePath, "Install", "libcarla");
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

		bEnableExceptions = bEnableExceptions || IsWindows;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // PrivatePCHHeaderFile = "Carla.h";

        Action<string, bool> LogBuildFlagStatus = (name, enabled) =>
        {
            Console.WriteLine(
                string.Format(
                    "{0} is {1}.",
                    name,
                    enabled ? "enabled" : "disabled"));
        };

        LogBuildFlagStatus("Houdini support", EnableHoudini);
        LogBuildFlagStatus("NVIDIA Omniverse support", EnableNVIDIAOmniverse);
        LogBuildFlagStatus("Unity build", EnableUnityBuild);

        bUseUnity = EnableUnityBuild;

        // PublicIncludePaths.AddRange(new string[] { });
        // PrivateIncludePaths.AddRange(new string[] { });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "ProceduralMeshComponent",
            "MeshDescription",
            "RawMesh",
            "AssetTools"
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
			"ChaosVehicles",
			"Json",
			"JsonUtilities",
			"Networking",
			"Sockets",
			"HTTP",
			"RHI",
			"RenderCore",
			"MeshMergeUtilities",
			"StreetMapImporting",
			"StreetMapRuntime"
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
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"OmniverseUSD",
				"OmniverseRuntime"
			});
		}

        // DynamicallyLoadedModuleNames.AddRange(new string[] { });

        var LibraryPrefix = IsWindows ? "" : "lib";
        var LibrarySuffix = IsWindows ? ".lib" : ".a";

        var LibCarlaInstallPath = CarlaInstallPath;
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
            Array.Sort(Candidates);
            return Candidates;
        };

        // LibCarla
        var LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");
        var LibCarlaLibPath = Path.Combine(LibCarlaInstallPath, "lib");
        var LibCarlaServerPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-server"));
        var LibCarlaClientPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-client"));
        // Boost
        var BoostLibraryPatterns = new string[]
        {
            GetLibraryName("*boost_asio*"),
            GetLibraryName("*boost_python*"),
        };
        var BoostLibraries =
            from Pattern in BoostLibraryPatterns
            from Candidate in FindLibraries("boost", Pattern)
            select Candidate;

        var SQLiteBuildPath = Path.Combine(DependenciesInstallPath, "sqlite-build");
        var SQLiteLibrary = Directory.GetFiles(SQLiteBuildPath, GetLibraryName("sqlite*"))[0];

        PublicAdditionalLibraries.AddRange(BoostLibraries);
        PublicAdditionalLibraries.AddRange(new string[]
        {
            SQLiteLibrary,
            FindLibraries("rpclib", "rpc")[0],
            FindLibraries("xercesc", "xerces-c*")[0],
            FindLibraries("proj", "proj")[0],
            FindLibraries("sumo", "*osm2odr")[0],
            FindLibraries("zlib", "zlib*")[0],
        });

        PublicIncludePaths.Add(LibCarlaIncludePath);

        PrivateIncludePaths.Add(LibCarlaIncludePath);

        PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
        PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");

        PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
        PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
        PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
        PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");

        if (EnableHoudini)
            PublicDefinitions.Add("CARLA_HOUDINI_ENABLED");
    }
}
