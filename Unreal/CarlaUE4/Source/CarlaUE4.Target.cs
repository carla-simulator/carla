// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System;
using EpicGames.Core;

public class CarlaUE4Target : TargetRules
{
    [CommandLine("-unity-build")]
    bool EnableUnityBuild = false;

    public CarlaUE4Target(TargetInfo Target) :
        base(Target)
    {
        Type = TargetType.Game;
        
        ExtraModuleNames.Add("CarlaUE4");

        Console.WriteLine("Unity build is disabled.");
        bUseUnityBuild = EnableUnityBuild;
        bForceUnityBuild = EnableUnityBuild;
        bUseAdaptiveUnityBuild = EnableUnityBuild;
    }
}
