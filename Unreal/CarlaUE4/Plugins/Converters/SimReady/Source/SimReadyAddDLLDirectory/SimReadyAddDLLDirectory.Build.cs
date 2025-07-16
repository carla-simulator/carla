// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SimReadyAddDLLDirectory : ModuleRules
	{
		public SimReadyAddDLLDirectory(ReadOnlyTargetRules Target) : base(Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;    // For game module

			PrivateDependencyModuleNames.Add("Core");
			PrivateDependencyModuleNames.Add("Projects");
		}
	}
}
