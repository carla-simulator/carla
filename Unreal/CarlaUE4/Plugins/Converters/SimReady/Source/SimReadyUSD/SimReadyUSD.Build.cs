// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SimReadyUSD : ModuleRules
	{		
        public SimReadyUSD(ReadOnlyTargetRules Target) : base(Target)
		{
			bUseRTTI = true;    // For USD
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;    // For game module
			bEnableExceptions = true;   // For game build

            PrivateIncludePathModuleNames.AddRange(
                new string[] {
                "Foliage",
                }
            );

            PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"Engine",
					"CoreUObject",
					"SimReadyRuntime",
					"MeshDescription",
					"RenderCore",
					"DerivedDataCache",
					"SlateCore",
                    "MeshUtilitiesCommon",
                    "MeshUtilities",
                    "Landscape",
					"StaticMeshDescription",
					"MaterialBaking",
                    "LevelSequence",
                    "MovieScene",
                    "MovieSceneTracks",
					"AnimGraphRuntime",
					"ActorSequence",
					"Projects",
					"SimReadyCarlaWrapper",
				}
            );

			AddEngineThirdPartyPrivateStaticDependencies(Target, "MikkTSpace");

			if (Target.Type == TargetType.Editor)
			{
				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"RawMesh",
						"MeshDescriptionOperations",
						"UnrealEd",
						"CinematicCamera",
                        "LevelEditor",
                        "LevelSequenceEditor",
                        "Sequencer",
						"ContentBrowser",
						"BlueprintGraph",
					}
				);
			}
		}
	}
}