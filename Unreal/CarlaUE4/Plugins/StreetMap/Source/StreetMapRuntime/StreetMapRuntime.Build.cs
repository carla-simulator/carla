// Copyright 2017 Mike Fricker. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
  public class StreetMapRuntime : ModuleRules
  {
    public StreetMapRuntime(ReadOnlyTargetRules Target)
    : base(Target)
    {
      PrivateDependencyModuleNames.AddRange(
        new string[] {
          "Core",
          "CoreUObject",
          "Engine",
          "RHI",
          "RenderCore",
          "PropertyEditor"
        }
      );

      PrivateIncludePaths.AddRange(new string[]{"StreetMapRuntime/Private"});
    }
  }
}
