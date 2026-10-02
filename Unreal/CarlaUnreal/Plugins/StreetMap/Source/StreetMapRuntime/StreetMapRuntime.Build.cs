// Copyright 2017 Mike Fricker. All Rights Reserved.
using System.IO;

namespace UnrealBuildTool.Rules
{
  public class StreetMapRuntime : ModuleRules
  {
    public StreetMapRuntime(ReadOnlyTargetRules Target)
    : base(Target)
    {
      PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
      //PublicDependencyModuleNames.AddRange(new string[] { "Carla" });
      PrivateDependencyModuleNames.AddRange(
        new string[] {
          "Core",
          "CoreUObject",
          "Engine",
          "RHI",
          "RenderCore",
          "PropertyEditor",
          "ProceduralMeshComponent",
          "GeometryScriptingCore",
          "GeometryFramework",
          "MeshDescription",
          "StaticMeshDescription",
          "Carla"
        }
      );

      if (Target.Version.MajorVersion < 5)
        PrivateDependencyModuleNames.Add("GeometricObjects");
      else
        PrivateDependencyModuleNames.Add("GeometryCore");

      PrivateIncludePaths.AddRange(new string[] { "StreetMapRuntime/Private" });

    }
  }
}
