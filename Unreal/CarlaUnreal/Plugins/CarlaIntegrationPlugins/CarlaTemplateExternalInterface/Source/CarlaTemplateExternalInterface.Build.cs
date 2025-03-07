using UnrealBuildTool;
using System.IO;

public class CarlaTemplateExternalInterface : ModuleRules
{
    public CarlaTemplateExternalInterface(ReadOnlyTargetRules Target) : base(Target)
    {
        bEnableExceptions = true;
        PrivatePCHHeaderFile = "CarlaTemplateExternalInterface.h";

        PublicDependencyModuleNames.AddRange(new string[] {
            "Carla",
            "Chaos",
            "ChaosVehicles",
            "Core",
            "CoreUObject",
            "Engine",
            "Foliage"
        });

        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        PrivateDependencyModuleNames.AddRange(new string[] {  });
    }
}
