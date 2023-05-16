// Copyright 2017 Mike Fricker. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
  public class StreetMapImporting : ModuleRules
  {
    public StreetMapImporting(ReadOnlyTargetRules Target)
    : base(Target)
    {
      PrivateDependencyModuleNames.AddRange(
        new string[] {
          "Core",
          "CoreUObject",
          "Engine",
          "UnrealEd",
          "XmlParser",
          "AssetTools",
          "Projects",
          "Slate",
          "EditorStyle",
          "SlateCore",
          "PropertyEditor",
          "RenderCore",
          "RHI",
          "RawMesh",
          "AssetTools",
          "AssetRegistry",
          "StreetMapRuntime"
        }
      );

      PrivateIncludePaths.AddRange(new string[]{"StreetMapImporting/Private"});
    }
  }
}
