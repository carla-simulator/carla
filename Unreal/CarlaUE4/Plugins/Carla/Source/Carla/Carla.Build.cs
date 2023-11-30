// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using System.Linq;
using UnrealBuildTool;
using EpicGames.Core;
using System.Diagnostics;

public class Carla :
	ModuleRules
{
	[CommandLine("-carsim")]
	bool EnableCarSim = false;

    [CommandLine("-chrono")]
    bool EnableChrono = false;

    [CommandLine("-pytorch")]
    bool EnablePytorch = false;

    [CommandLine("-ros2")]
    bool EnableRos2 = false;

    [CommandLine("-carla-install-path")]
    string CarlaInstallPath = null;

    [CommandLine("-carla-dependencies-path")]
    string CarlaDependenciesPath = null;



    public Carla(ReadOnlyTargetRules Target) :
		base(Target)
    {
        bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

        PrivatePCHHeaderFile = "Carla.h";
		bEnableExceptions = bEnableExceptions || IsWindows;

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

		Action<string, bool> LogBuildFlagStatus = (name, enabled) =>
		{
			Console.WriteLine(
				string.Format(
					"{0} is {1}.",
					name,
					enabled ? "enabled" : "disabled"));
		};

        LogBuildFlagStatus("CarSim support", EnableCarSim);
        LogBuildFlagStatus("Chrono support", EnableChrono);
        LogBuildFlagStatus("PyTorch support", EnablePytorch);
        LogBuildFlagStatus("ROS2 support", EnableRos2);

        if (EnableCarSim)
		{
            PublicDefinitions.Add("WITH_CARSIM");
			PrivateDefinitions.Add("WITH_CARSIM");
		}

		if (EnableChrono)
		{
			PublicDefinitions.Add("WITH_CHRONO");
			PrivateDefinitions.Add("WITH_CHRONO");
		}

		if (EnablePytorch)
		{
			PublicDefinitions.Add("WITH_PYTORCH");
			PrivateDefinitions.Add("WITH_PYTORCH");
		}

		if (EnableRos2)
        {
            PublicDefinitions.Add("WITH_ROS2");
			PrivateDefinitions.Add("WITH_ROS2");
		}

        // PublicIncludePaths.AddRange(new string[] { });
        // PrivateIncludePaths.AddRange(new string[] { });

        PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"RenderCore",
			"RHI",
			"Renderer",
			"ProceduralMeshComponent",
			"MeshDescription"
		});

		if (EnableCarSim)
		{
			PublicDependencyModuleNames.Add("CarSim");
		}

		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.Add("UnrealEd");
		}

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AIModule",
			"AssetRegistry",
			"CoreUObject",
			"Engine",
			"Foliage",
			"HTTP",
			"StaticMeshDescription",
			"ImageWriteQueue",
			"Json",
			"JsonUtilities",
			"Landscape",
			"Chaos",
			"ChaosVehicles",
			"Slate",
			"SlateCore",
			"PhysicsCore"
		});

		if (EnableCarSim)
		{
			PrivateDependencyModuleNames.Add("CarSim");
			PrivateIncludePathModuleNames.Add("CarSim");
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

        if (EnableChrono)
        {
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
				select FindLibraries(ChronoLibPath, GetLibraryName(Name))[0];
			PublicAdditionalLibraries.AddRange(ChronoLibraries);
        }

        PrivateIncludePaths.Add(LibCarlaIncludePath);
        PrivateIncludePaths.Add(LibCarlaIncludePath);
        PrivateIncludePaths.AddRange(new string[]
        {
            GetIncludePath("boost"),
            GetIncludePath("rpclib"),
            GetIncludePath("xercesc"),
            GetIncludePath("sumo"),
            GetIncludePath("zlib"),
        });

        PrivateDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
        PrivateDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
        PrivateDefinitions.Add("ASIO_NO_EXCEPTIONS");
        PrivateDefinitions.Add("BOOST_NO_EXCEPTIONS");
        PrivateDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
        PrivateDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
    }

#if false
	private void AddDynamicLibrary(string library)
	{
		PublicAdditionalLibraries.Add(library);
		RuntimeDependencies.Add(library);
		PublicDelayLoadDLLs.Add(library);
	}

	private void AddDllDependency(string PathToFolder, string DllName)
	{
		string Source = Path.Combine(PathToFolder, DllName);
		string Destination = Path.Combine("$(BinaryOutputDir)", DllName);
		RuntimeDependencies.Add(Destination, Source);
	}

	private void AddCarlaServerDependency(ReadOnlyTargetRules Target)
	{
		string LibCarlaInstallPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../CarlaDependencies"));

		// Link dependencies.
		if (IsWindows)
		{
            foreach (string file in Directory.GetFiles(Path.Combine(LibCarlaInstallPath, "lib"), "*boost*.lib"))
                PublicAdditionalLibraries.Add(file);

            PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("rpc")));

			if (UseDebugLibs(Target))
			{
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server_debug")));
			}
			else
			{
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server")));
			}
			if (EnableChrono)
			{
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoEngine")));
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoEngine_vehicle")));
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoModels_vehicle")));
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("ChronoModels_robot")));
				AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoEngine.dll");
				AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoEngine_vehicle.dll");
				AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoModels_vehicle.dll");
				AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoModels_robot.dll");
				bUseRTTI = true;
			}

			//OsmToODR
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "sqlite3.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "xerces-c_3.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "proj.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "osm2odr.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "zlibstatic.lib"));
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
			if (EnableChrono)
			{
				AddDynamicLibrary(Path.Combine(LibCarlaInstallPath, "lib", "libChronoEngine.so"));
				AddDynamicLibrary(Path.Combine(LibCarlaInstallPath, "lib", "libChronoEngine_vehicle.so"));
				AddDynamicLibrary(Path.Combine(LibCarlaInstallPath, "lib", "libChronoModels_vehicle.so"));
				AddDynamicLibrary(Path.Combine(LibCarlaInstallPath, "lib", "libChronoModels_robot.so"));

				bUseRTTI = true;
			}

			if (EnablePytorch)
			{
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_pytorch")));

				string LibTorchPath = LibCarlaInstallPath;
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libonnx_proto.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libfbgemm.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libgloo.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libXNNPACK.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libprotobuf-lite.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libprotobuf.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libasmjit.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libcpuinfo_internals.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libclog.a"));
				// PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libbreakpad_common.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libbenchmark.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libtensorpipe.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libpytorch_qnnpack.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libtensorpipe_cuda.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libnnpack_reference_layers.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libgmock.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libdnnl.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libpthreadpool.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libcpuinfo.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libqnnpack.a"));
				// PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libbreakpad.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libkineto.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libprotoc.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libgtest.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libgmock_main.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libgtest_main.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libbenchmark_main.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libfmt.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libtensorpipe_uv.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libfoxi_loader.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libgloo_cuda.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libnnpack.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libcaffe2_protos.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibTorchPath, "lib", "libonnx.a"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorch.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libnnapi_backend.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libbackend_with_compiler.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libcaffe2_nvrtc.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorch_cuda_cpp.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libc10_cuda.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorchbind_test.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libjitbackend_test.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libc10.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorch_cuda.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorch_global_deps.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorch_cpu.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libshm.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorch_cuda_cu.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorchscatter.so"));
				AddDynamicLibrary(Path.Combine(LibTorchPath, "lib", "libtorchcluster.so"));
				// AddDynamicLibrary("/usr/local/cuda/lib64/stubs/libcuda.so");
				// AddDynamicLibrary("/usr/local/cuda/lib64/libnvrtc.so");
				// AddDynamicLibrary("/usr/local/cuda/lib64/libnvToolsExt.so");
				// AddDynamicLibrary("/usr/local/cuda/lib64/libcudart.so");
				// AddDynamicLibrary("/usr/lib/llvm-10/lib/libgomp.so");
				PublicAdditionalLibraries.Add("/usr/local/cuda/lib64/stubs/libcuda.so");
				PublicAdditionalLibraries.Add("/usr/local/cuda/lib64/libnvrtc.so");
				PublicAdditionalLibraries.Add("/usr/local/cuda/lib64/libnvToolsExt.so");
				PublicAdditionalLibraries.Add("/usr/local/cuda/lib64/libcudart.so");
				PublicAdditionalLibraries.Add("/usr/lib/llvm-10/lib/libgomp.so");
				RuntimeDependencies.Add(Path.Combine(LibTorchPath, "lib", "libcudart-a7b20f20.so.11.0"));
				RuntimeDependencies.Add(Path.Combine(LibTorchPath, "lib", "libgomp-a34b3233.so.1"));
				RuntimeDependencies.Add(Path.Combine(LibTorchPath, "lib", "libnvrtc-builtins-4730a239.so.11.3"));
				RuntimeDependencies.Add(Path.Combine(LibTorchPath, "lib", "libnvrtc-1ea278b5.so.11.2"));
				RuntimeDependencies.Add(Path.Combine(LibTorchPath, "lib", "libnvToolsExt-24de1d56.so.1"));
				PublicAdditionalLibraries.Add("stdc++");
				PublicAdditionalLibraries.Add("/usr/lib/x86_64-linux-gnu/libpython3.9.so");
			}

			if (EnableRos2)
			{
				PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_fastdds")));

				string LibFastDDSPath = LibCarlaInstallPath;
				AddDynamicLibrary(Path.Combine(LibFastDDSPath, "lib", "libfoonathan_memory-0.7.3.so"));
				AddDynamicLibrary(Path.Combine(LibFastDDSPath, "lib", "libfastcdr.so"));
				AddDynamicLibrary(Path.Combine(LibFastDDSPath, "lib", "libfastrtps.so"));
				PublicAdditionalLibraries.Add("stdc++");
			}


			//OsmToODR
			PublicAdditionalLibraries.Add("/usr/lib/x86_64-linux-gnu/libc.so");
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "libsqlite3.so"));
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "libxerces-c.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "libproj.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", "libosm2odr.a"));

		}
		bEnableExceptions = true;

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
#endif
}
