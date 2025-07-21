// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SimReadyCarlaWrapper : ModuleRules
	{
        public SimReadyCarlaWrapper(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"Engine",
					"CoreUObject",
					"Carla"
				}
			);			

			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;    // For game module
			bEnableExceptions = true;   // For game build

			PrivateIncludePaths.Add(Path.GetFullPath(Path.Combine(PluginDirectory, "../../Carla/CarlaDependencies/include")));
			PrivateIncludePaths.Add(Path.GetFullPath(Path.Combine(PluginDirectory, "../../../../../LibCarla/source/compiler")));
			PrivateIncludePaths.Add(Path.GetFullPath(Path.Combine(PluginDirectory, "../../Carla/Source/Carla")));
			PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
			PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
			PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
			PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
			PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
			PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
			bool IsWindows = (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
			string LibName = IsWindows ? "carla_server.lib" : "libcarla_server.a";
        	PublicAdditionalLibraries.Add(Path.GetFullPath(Path.Combine(PluginDirectory, "../../Carla/CarlaDependencies/lib", LibName)));
		}
	}
}