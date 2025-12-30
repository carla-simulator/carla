// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class MDL : ModuleRules
    {
        public MDL(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateDependencyModuleNames.AddRange(
                new string[] {
					"Engine",
                    "Projects",
					"Core",
					"CoreUObject",
					"Engine",
					"RHI",
					"RenderCore",
					"InputCore",
					"UnrealEd",
					"MaterialEditor",
				}
            );


            //string MDLSDKPath = Environment.GetEnvironmentVariable("MDL_SDK_PATH");
            string ThirdPartyPath = Path.Combine(ModuleDirectory, "..", "ThirdParty");
            string MDLSDKPath = Path.Combine(ThirdPartyPath, "mdl_sdk");

            if (String.IsNullOrEmpty(MDLSDKPath))
            {
                PublicDefinitions.Add("WITH_MDL_SDK=0");
                return;
            }
            else
            {
                PublicDefinitions.Add("WITH_MDL_SDK=1");
            }
            //PublicDefinitions.Add("ADD_CLIP_MASK");
            PublicDefinitions.Add("USE_WORLD_ALIGNED_TEXTURES");
            PublicDefinitions.Add("USE_WAT_AS_SCALAR");

            // This is used to redirect plugin to look in the project content folder, disable for now
			//PublicDefinitions.Add("WITH_CARLA");

            PublicIncludePaths.Add(Path.Combine(MDLSDKPath, "include"));

            string PlatformExtension;
            string MDLLibPath;

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PlatformExtension = ".dll";
                MDLLibPath = Path.Combine(MDLSDKPath, "bin");
            }
            else if (Target.Platform == UnrealTargetPlatform.Linux)
            {
                PlatformExtension = ".so";
                MDLLibPath = Path.Combine(MDLSDKPath, "lib");
        }
            else
            {
                return;
            }
            
            RuntimeDependencies.Add(Path.Combine(MDLLibPath, "libmdl_sdk" + PlatformExtension), StagedFileType.NonUFS);
            RuntimeDependencies.Add(Path.Combine(MDLLibPath, "mdl_distiller" + PlatformExtension), StagedFileType.NonUFS);
        }
    }
}
