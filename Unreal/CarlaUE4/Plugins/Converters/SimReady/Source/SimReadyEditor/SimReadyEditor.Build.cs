// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SimReadyEditor : ModuleRules
	{
		public SimReadyEditor(ReadOnlyTargetRules Target) : base(Target)
		{
            bUseRTTI = true;
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;    // For game module

			string MessageLogModuleDirectory = Path.Combine(Path.GetFullPath(Target.RelativeEnginePath), "Source", "Developer", "MessageLog", "Private");

			PrivateIncludePaths.AddRange(
				new string[] {
				MessageLogModuleDirectory,
				}
			);

			PrivateDependencyModuleNames.AddRange(new string[]{
				"ApplicationCore",
				"Core",
				"CoreUObject",
				"Engine",
				"MainFrame",
				"InputCore",
				"SlateCore",
				"Slate",
				"LevelEditor",
				"ContentBrowser",
				"ContentBrowserData",
				"UnrealEd",
				"Kismet",
				"Projects",
				//"EditorScriptingUtilities",
				"EditorStyle",
				"MovieSceneCapture",
				"MovieScene",
				"LevelSequence",
				"SourceControl",
                "DesktopPlatform",
				"PropertyEditor",
				"MessageLog",
				"MaterialEditor",
			});

			PublicDependencyModuleNames.AddRange(new string[]{
				"Engine",
				"SimReadyRuntime",
				"SimReadyUSD",
			});
		}
	}
}
