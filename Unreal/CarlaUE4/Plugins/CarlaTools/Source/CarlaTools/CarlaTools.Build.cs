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
        return null;
    }

    public CarlaTools(ReadOnlyTargetRules Target) :
		base(Target)
    {
        bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;
        var DirectoryInfo = new DirectoryInfo(ModuleDirectory);
        for (int i = 0; i != 6; ++i)
            DirectoryInfo = DirectoryInfo.Parent;
        var WorkspacePath = DirectoryInfo.ToString();
        Debug.Assert(Directory.Exists(WorkspacePath));

        if (CarlaInstallPath == null)
        {
            Console.WriteLine("-carla-install-path was not specified, inferring...");
            CarlaInstallPath = Path.Combine(WorkspacePath, "Install", "libcarla");
            Debug.Assert(Directory.Exists(CarlaInstallPath), "Could not infer CARLA install directory.");
        }

        if (CarlaDependenciesPath == null)
        {
            Console.WriteLine("-carla-dependencies-path was not specified, inferring...");
            CarlaDependenciesPath = Path.Combine(WorkspacePath, "Build", "Dependencies");
            Debug.Assert(Directory.Exists(CarlaDependenciesPath), "Could not infer CARLA dependencies directory.");
        }

		bEnableExceptions = bEnableExceptions || IsWindows;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
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
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"OmniverseUSD",
				"OmniverseRuntime"
			});
		}

        // DynamicallyLoadedModuleNames.AddRange(new string[] { });

        var LibraryPrefix = IsWindows ? "" : "lib";
        var LibrarySuffix = IsWindows ? ".lib" : ".a";

        Func<string, string> GetLibraryName = name =>
        {
            return LibraryPrefix + name + LibrarySuffix;
        };

        var LibCarlaInstallPath = CarlaInstallPath;
        var DependenciesInstallPath = CarlaDependenciesPath;
        // LibCarla
        var LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");
        var LibCarlaLibPath = Path.Combine(LibCarlaInstallPath, "lib");
        var LibCarlaServerPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-server"));
        var LibCarlaClientPath = Path.Combine(LibCarlaLibPath, GetLibraryName("carla-client"));
        // Boost
        var BoostInstallPath = Path.Combine(DependenciesInstallPath, "boost-install");
        var BoostLibPath = Path.Combine(BoostInstallPath, "lib");
        var BoostLibraryPatterns = new string[]
        {
            GetLibraryName("*boost_asio*"),
            GetLibraryName("*boost_python*"),
        };
        var BoostLibraries =
            from Pattern in BoostLibraryPatterns
            select FindLibrary(BoostLibPath, Pattern);
        // Chrono
        var ChronoInstallPath = Path.Combine(DependenciesInstallPath, "chrono-install");
        var ChronoLibPath = Path.Combine(ChronoInstallPath, "lib");
        var ChronoLibraryNames = new string[]
        {
            "ChronoEngine",
            "ChronoEngine_vehicle",
            "ChronoModels_vehicle",
            "ChronoModels_robot",
        };
        var ChronoLibraries =
            from Name in ChronoLibraryNames
            select FindLibrary(Name);
        // SQLite
        var SQLiteBuildPath = Path.Combine(DependenciesInstallPath, "sqlite-build");
        var SQLiteLibrary = FindLibrary(SQLiteBuildPath, GetLibraryName("sqlite*"));
        // RPCLib
        var RPCLibInstallPath = Path.Combine(DependenciesInstallPath, "rpclib-install");
        var RPCLibPath = Path.Combine(RPCLibInstallPath, "lib");
        var RPCLibraryPath = FindLibrary(RPCLibPath, "rpc");
        // Xerces-C
        var XercesCInstallPath = Path.Combine(DependenciesInstallPath, "xercesc-install");
        var XercesCLibPath = Path.Combine(XercesCInstallPath, "lib");
        var XercesCLibrary = FindLibrary(XercesCLibPath, "xercesc*");
        // Proj
        var ProjInstallPath = Path.Combine(DependenciesInstallPath, "proj-install");
        var ProjLibPath = Path.Combine(ProjInstallPath, "lib");
        var ProjLibrary = FindLibrary(ProjLibPath, "proj*");
        // SUMO (OSM2ODR)
        var SUMOInstallPath = Path.Combine(DependenciesInstallPath, "sumo-install");
        var SUMOLibPath = Path.Combine(SUMOInstallPath, "lib");
        var SUMOLibrary = FindLibrary(SUMOLibPath, "sumo*");
        // ZLib
        var ZLibInstallPath = Path.Combine(DependenciesInstallPath, "zlib-install");
        var ZLibLibPath = Path.Combine(ZLibInstallPath, "lib");
        var ZLibLibrary = FindLibrary(ZLibLibPath, "zlib*");

        PublicAdditionalLibraries.Add(LibCarlaServerPath);
        PublicAdditionalLibraries.Add(LibCarlaClientPath);
		PublicAdditionalLibraries.Add(RPCLibraryPath);
		PublicAdditionalLibraries.AddRange(BoostLibraries);
		
		PublicIncludePaths.Add(LibCarlaIncludePath);

		PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
        PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
        if (!bEnableExceptions)
        {
            PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
            PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
            PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
            PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
        }
    }
}
