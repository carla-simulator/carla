// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SimReadyRuntime : ModuleRules
	{
		public SimReadyRuntime(ReadOnlyTargetRules Target) : base(Target)
		{
			bUseRTTI = true;    // For boost
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;    // For game module
			bEnableExceptions = true;   // For game build
			AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RHI",
                "RenderCore",
                "ImageCore",
				"TextureCompressor",
				"Projects",
				"SimReadyAddDLLDirectory",
				"HTTP",
			});

			PublicDependencyModuleNames.AddRange(new string[]
			{
				"Engine",
				"IntelTBB",
			});

			if (Target.Type == TargetType.Editor)
			{
				PublicDependencyModuleNames.AddRange(new string[]
				{
					"EditorStyle",
					"MaterialEditor",
					"MDL",
					"UnrealEd",
					"SlateCore",
					"Slate",
					"Json",
					"JsonUtilities",
					"MessageLog",
                });
			}

			// ModuleDirectory is where this file (the Build.cs file) is located
			// Going up two folders gets to the root of the plugin (We are in Source/SimReadyEditor)
			string ThirdPartyPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty");

			string ThirdPartyConfig = Target.Platform.ToString() + (Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT ? "_debug" : "_release");

			///////////////////////////////////////////////////////////
			// From UnrealUSDWrapper.Build.cs
			///////////////////////////////////////////////////////////
			string USDWrapperModuleDirectory = Path.Combine(Path.GetFullPath(Target.RelativeEnginePath), "Plugins", "Importers", "USDImporter", "Source", "UnrealUSDWrapper");
			PublicDependencyModuleNames.Add("Python3");

			PublicDefinitions.Add("BOOST_LIB_TOOLSET=\"vc141\"");

			PublicIncludePaths.AddRange(
				new string[] {
				USDWrapperModuleDirectory + "/../ThirdParty/USD/include",
				});

			var USDLibsDir = "";

			var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
			var PythonSourceTPSDir = Path.Combine(EngineDir, "Source", "ThirdParty", "Python3");
			//var PythonBinaryTPSDir = Path.Combine(EngineDir, "Binaries", "ThirdParty", "Python3");

			// Always use the official version of IntelTBB
			string IntelTBBLibs = Target.UEThirdPartySourceDirectory + "Intel/TBB/IntelTBB-2019u8/lib/";
			string IntelTBBIncludes = Target.UEThirdPartySourceDirectory + "Intel/TBB/IntelTBB-2019u8/include/";

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PublicDefinitions.Add("USD_USES_SYSTEM_MALLOC=1");

				USDLibsDir = Path.Combine(USDWrapperModuleDirectory, "../ThirdParty/USD/lib/");

				var USDLibs = new string[]
				{
					"ar",
					"arch",
					"gf",
					"js",
					"kind",
					"pcp",
					"plug",
					"sdf",
					"tf",
					"usd",
					"usdGeom",
					"usdLux",
					"usdShade",
					"usdSkel",
					"usdUtils",
					"vt",
					"work",
				};

				foreach (string UsdLib in USDLibs)
				{
					PublicAdditionalLibraries.Add(Path.Combine(USDLibsDir, UsdLib + ".lib"));
				}
				PublicAdditionalLibraries.Add(Path.Combine(IntelTBBLibs, "Win64/vc14/tbb.lib"));

				PublicIncludePaths.Add(PythonSourceTPSDir + "/Win64/include");
				PublicSystemLibraryPaths.Add(Path.Combine(EngineDir, "Source/ThirdParty/Python3/" + Target.Platform.ToString() + "/libs"));
			}
			else if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				PublicDefinitions.Add("USD_USES_SYSTEM_MALLOC=0"); // USD uses tbb malloc on Linux

				USDLibsDir = Path.Combine(USDWrapperModuleDirectory, "../../Binaries/Linux/", Target.Architecture);

				var USDLibs = new string[]
				{
						"libar.so",
						"libarch.so",
						"libboost_python37.so",
						"libgf.so",
						"libjs.so",
						"libkind.so",
						"libndr.so",
						"libpcp.so",
						"libplug.so",
						"libsdf.so",
						"libsdr.so",
						"libtf.so",
						"libtrace.so",
						"libusd.so",
						"libusdGeom.so",
						"libusdLux.so",
						"libusdShade.so",
						"libusdSkel.so",
						"libusdUtils.so",
						"libusdVol.so",
						"libvt.so",
						"libwork.so",
				};

				PublicSystemIncludePaths.Add(IntelTBBIncludes);
				PublicAdditionalLibraries.Add(Path.Combine(IntelTBBLibs, "Linux/libtbb.so"));
				RuntimeDependencies.Add("$(EngineDir)/Binaries/Linux/libtbb.so.2", Path.Combine(IntelTBBLibs, "Linux/libtbb.so.2"));
				PublicAdditionalLibraries.Add(Path.Combine(IntelTBBLibs, "Linux/libtbbmalloc.so"));

				foreach (string UsdLib in USDLibs)
				{
					PublicAdditionalLibraries.Add(Path.GetFullPath(Path.Combine(USDLibsDir, UsdLib)));
					RuntimeDependencies.Add(Path.GetFullPath(Path.Combine(USDLibsDir, UsdLib)));
				}

				PublicSystemLibraryPaths.Add(Path.Combine(EngineDir, "Source/ThirdParty/Python3/" + Target.Platform.ToString() + "/lib"));
			}
			
			PublicRuntimeLibraryPaths.Add(USDLibsDir);
			PublicSystemLibraryPaths.Add(USDLibsDir);
			///////////////////////////////////////////////////////////

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "nvtt", ThirdPartyConfig, "include"));

				string NvttLibPath = Path.Combine(ThirdPartyPath, "nvtt", ThirdPartyConfig, "lib/x64-v142");
				PublicAdditionalLibraries.Add(Path.Combine(NvttLibPath, "nvtt30106.lib"));
				PublicDelayLoadDLLs.Add("cudart64_110.dll");
				PublicDelayLoadDLLs.Add("nvtt30106.dll");
				string NvttDllPath = Path.Combine(ThirdPartyPath, "nvtt", ThirdPartyConfig);
				RuntimeDependencies.Add(Path.Combine(NvttDllPath, "cudart64_110.dll"));
				RuntimeDependencies.Add(Path.Combine(NvttDllPath, "nvtt30106.dll"));
			}
        }
	}
}