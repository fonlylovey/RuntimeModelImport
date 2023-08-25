// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ModelDBTarget : TargetRules
{
	public ModelDBTarget( TargetInfo Target) : base(Target)
	{
        Type = TargetType.Game;
        LinkType = TargetLinkType.Monolithic;
        DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		ExtraModuleNames.Add("ModelDB");
	}
}
