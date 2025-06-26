using UnrealBuildTool;
using System.IO;

public class RosUtils : ModuleRules
{
    public RosUtils(ReadOnlyTargetRules Target) : base(Target)
    {
        void AddDynamicLibrary(string library)
        {
            PublicAdditionalLibraries.Add(library);
            RuntimeDependencies.Add(library);
            PublicDelayLoadDLLs.Add(library);
            System.Console.WriteLine("Dynamic Library Added: " + library);
        }

        bEnableExceptions = true;
        PrivatePCHHeaderFile = "RosUtils.h";
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseRTTI = true;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Carla",
            "Chaos",
            "ChaosVehicles",
            "Core",
            "CoreUObject",
            "Engine",
            "Foliage",
            "HTTP",
            "ImageWriteQueue",
            "InputCore",
            "Json",
            "JsonUtilities",
            "Landscape",
            "MeshDescription",
            "PhysicsCore",
            "ProceduralMeshComponent",
            "Slate",
            "SlateCore",
            "StaticMeshDescription",
            "RHI",
            "Renderer"
            });

        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        PrivateDependencyModuleNames.AddRange(new string[] {  });

        string PluginSourcePath = Path.GetFullPath(ModuleDirectory);
        string PluginBinariesBuildPath = Path.Combine(PluginSourcePath, "..", "..", "..", "..", "..", "..", "..");
        PublicIncludePaths.Add(Path.Combine(PluginBinariesBuildPath, "Build/Ros2Native/install/include"));

        string CarlaPluginBinariesLinuxPath = Path.Combine(PluginSourcePath, "..", "..", "..", "..", "Carla", "Binaries", "Linux");
        AddDynamicLibrary(Path.Combine(CarlaPluginBinariesLinuxPath, "libcarla-ros2-native.so"));
        RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfoonathan_memory-0.7.3.so"));
        RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastcdr.so"));
        RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastcdr.so.1"));
        RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastcdr.so.1.1.0"));
        RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastrtps.so"));
        RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastrtps.so.2.11"));
        RuntimeDependencies.Add(Path.Combine(CarlaPluginBinariesLinuxPath, "libfastrtps.so.2.11.2"));


    }
}
