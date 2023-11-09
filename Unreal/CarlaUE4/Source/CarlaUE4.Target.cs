// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;
using System;
using System.IO;

public class CarlaUE4Target : TargetRules
{
	public CarlaUE4Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("CarlaUE4");

    string ConfigDir = Path.GetDirectoryName(ProjectFile.ToString()) + "/Config/";
    string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
    string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);

    bool UnityOn = true;

    foreach (string line in text) {
      if (line.Contains("Unity OFF"))
      {
        UnityOn = false;
      }
    }

    if (!UnityOn) {
      Console.WriteLine("Disabling unity");
      bUseUnityBuild = false;
      bForceUnityBuild = false;
      bUseAdaptiveUnityBuild = false;
    }
	}
}
