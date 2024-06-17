// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;


public class Carla : CarlaRules
{
  bool UsingCarSim = false;
  bool UsingChrono = false;
  bool UsingPytorch = false;
  public Carla(ReadOnlyTargetRules Target) : base(Target, "../../CarlaDependencies")
  {
    PrivatePCHHeaderFile = "Carla.h";
    bEnableExceptions = true;
    CppStandard = CppStandardVersion.Cpp20;

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
      if (line.Contains("Pytorch ON"))
      {
        Console.WriteLine("Enabling pytorch");
        UsingPytorch = true;
        PublicDefinitions.Add("WITH_PYTORCH");
        PrivateDefinitions.Add("WITH_PYTORCH");
      }

      if (line.Contains("Ros2 ON"))
      {
        Console.WriteLine("Enabling ros2");
        UsingRos2 = true;
      }

      if (line.Contains("Debug ON"))
      {
        Console.WriteLine("Enabling debug build");
        Debug = true;
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
        "Renderer",
        "ProceduralMeshComponent",
        "MeshDescription"
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
        "HTTP",
        "StaticMeshDescription",
        "ImageWriteQueue",
        "Json",
        "JsonUtilities",
        "Landscape",
        "PhysX",
        "PhysXVehicles",
        "PhysXVehicleLib",
        "Slate",
        "SlateCore",
        "PhysicsCore"
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

    AddCarlaServerDependency();
  }

  private void AddCarlaServerDependency()
  {

    // Link dependencies.
    AddStaticLibrary( "rpc");
    if (UseDebugLibs())
    {
      AddStaticLibrary("carla_server_debug");
    }
    else
    {
      AddStaticLibrary("carla_server");
    }
    if (IsWindows())
    {
       PublicAdditionalLibraries.Add("shlwapi.lib");

      AddBoostLibs();
      if (UsingChrono)
      {
        AddStaticLibrary("ChronoEngine");
        AddStaticLibrary("ChronoEngine_vehicle");
        AddStaticLibrary("ChronoModels_vehicle");
        AddStaticLibrary("ChronoModels_robot");
        AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoEngine.dll");
        AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoEngine_vehicle.dll");
        AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoModels_vehicle.dll");
        AddDllDependency(Path.Combine(LibCarlaInstallPath, "dll"), "ChronoModels_robot.dll");
        bUseRTTI = true;
      }
    }
    else
    {
      if (UsingChrono)
      {
        AddStaticLibrary( "ChronoEngine");
        AddStaticLibrary( "ChronoEngine_vehicle");
        AddStaticLibrary( "ChronoModels_vehicle");
        AddStaticLibrary( "ChronoModels_robot");

        bUseRTTI = true;
      }

      if (UsingPytorch)
      {
        AddStaticLibrary( "carla_pytorch");

        AddStaticLibrary("onnx_proto");
        AddStaticLibrary("fbgemm");
        AddStaticLibrary("gloo");
        AddStaticLibrary("XNNPACK");
        AddStaticLibrary("protobuf-lite");
        AddStaticLibrary("protobuf");
        AddStaticLibrary("asmjit");
        AddStaticLibrary("cpuinfo_internals");
        AddStaticLibrary("clog");
        // AddStaticLibrary("breakpad_common");
        AddStaticLibrary("benchmark");
        AddStaticLibrary("tensorpipe");
        AddStaticLibrary("pytorch_qnnpack");
        AddStaticLibrary("tensorpipe_cuda");
        AddStaticLibrary("nnpack_reference_layers");
        AddStaticLibrary("gmock");
        AddStaticLibrary("dnnl");
        AddStaticLibrary("pthreadpool");
        AddStaticLibrary("cpuinfo");
        AddStaticLibrary("qnnpack");
        // AddStaticLibrary("breakpad"));
        AddStaticLibrary("kineto");
        AddStaticLibrary("protoc");
        AddStaticLibrary("gtest");
        AddStaticLibrary("gmock_main");
        AddStaticLibrary("gtest_main");
        AddStaticLibrary("benchmark_main");
        AddStaticLibrary("fmt");
        AddStaticLibrary("tensorpipe_uv");
        AddStaticLibrary("foxi_loader");
        AddStaticLibrary("gloo_cuda");
        AddStaticLibrary("nnpack");
        AddStaticLibrary("caffe2_protos");
        AddStaticLibrary("onnx");
        AddDynamicLibrary("torch");
        AddDynamicLibrary("nnapi_backend");
        AddDynamicLibrary("backend_with_compiler");
        AddDynamicLibrary("caffe2_nvrtc");
        AddDynamicLibrary("torch_cuda_cpp");
        AddDynamicLibrary("c10_cuda");
        AddDynamicLibrary("torchbind_test");
        AddDynamicLibrary("jitbackend_test");
        AddDynamicLibrary("c10");
        AddDynamicLibrary("torch_cuda");
        AddDynamicLibrary("torch_global_deps");
        AddDynamicLibrary("torch_cpu");
        AddDynamicLibrary("shm");
        AddDynamicLibrary("torch_cuda_cu");
        AddDynamicLibrary("torchscatter");
        AddDynamicLibrary("torchcluster");
        AddDynamicLibrary("cuda", "/usr/local/cuda/lib64/stubs");
        AddDynamicLibrary("nvrtc", "/usr/local/cuda/lib64");
        AddDynamicLibrary("nvToolsExt", "/usr/local/cuda/lib64");
        AddDynamicLibrary("cudart", "/usr/local/cuda/lib64");
        AddDynamicLibrary("gomp", "/usr/lib/llvm-10/lib");
        RuntimeDependencies.Add(Path.Combine("libcudart-a7b20f20.so.11.0"));
        RuntimeDependencies.Add(Path.Combine("libgomp-a34b3233.so.1"));
        RuntimeDependencies.Add(Path.Combine("libnvrtc-builtins-4730a239.so.11.3"));
        RuntimeDependencies.Add(Path.Combine("libnvrtc-1ea278b5.so.11.2"));
        RuntimeDependencies.Add(Path.Combine("libnvToolsExt-24de1d56.so.1"));
        PublicAdditionalLibraries.Add("stdc++");
        PublicAdditionalLibraries.Add("/usr/lib/x86_64-linux-gnu/libpython3.9.so");
      }
    }

    addOsmToODR();

    addROS2();


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
