// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gulag : ModuleRules
{
	public Gulag(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", 
			"EnhancedInput", "OnlineSubsystem", "OnlineSubsystemUtils", "GameLiftServerSDK",
         "HTTP", "Json", "JsonUtilities" });
	}
}
