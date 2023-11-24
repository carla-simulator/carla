// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;
using System;
using System.IO;
using EpicGames.Core;

public class CarlaUE4EditorTarget :
    TargetRules
{
    [CommandLine("-unity-build")]
    bool EnableUnityBuild = false;

    public CarlaUE4EditorTarget(TargetInfo Target) :
        base(Target)
    {
        Type = TargetType.Editor;

        ExtraModuleNames.Add("CarlaUE4");

        Console.WriteLine("Unity build is disabled.");
        bUseUnityBuild = EnableUnityBuild;
        bForceUnityBuild = EnableUnityBuild;
        bUseAdaptiveUnityBuild = EnableUnityBuild;
    }
}
