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

    [CommandLine("-carla-install-path")]
    string CarlaInstallPath = null;

    [CommandLine("-carla-dependencies-install-path")]
    string CarlaDependenciesPath = null;

    private static void LogFlagStatus(string name, bool value)
    {
        var state = value ? "enabled" : "disabled";
        Console.WriteLine(string.Format("{0} is {1}.", name, state));
    }

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
        LogFlagStatus("Houdini support", EnableHoudini);
        LogFlagStatus("NVIDIA Omniverse support", EnableNVIDIAOmniverse);

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

        bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = bEnableExceptions || IsWindows;
        // PrivatePCHHeaderFile = "Carla.h";

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
            PublicDefinitions.Add("WITH_OMNIVERSE");
            PrivateDefinitions.Add("WITH_OMNIVERSE");

            PrivateDependencyModuleNames.AddRange(new string[]
			{
				"OmniverseUSD",
				"OmniverseRuntime"
			});
		}
        PrivateDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
        PrivateDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
        PrivateDefinitions.Add("ASIO_NO_EXCEPTIONS");
        PrivateDefinitions.Add("BOOST_NO_EXCEPTIONS");
        PrivateDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
        PrivateDefinitions.Add("PUGIXML_NO_EXCEPTIONS");

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

        Func<string, string> GetIncludePath = name => Path.Combine(DependenciesInstallPath, name + "-install", "include");

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
        PrivateIncludePaths.Add(LibCarlaIncludePath);
        PrivateIncludePaths.AddRange(new string[]
        {
            GetIncludePath("boost"),
            GetIncludePath("rpclib"),
            GetIncludePath("xercesc"),
            GetIncludePath("sumo"),
            GetIncludePath("zlib"),
        });
        PrivateIncludePaths.Add(LibCarlaIncludePath);
    }
}
