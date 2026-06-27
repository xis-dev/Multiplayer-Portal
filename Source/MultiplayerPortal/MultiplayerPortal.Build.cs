// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MultiplayerPortal : ModuleRules
{
	public MultiplayerPortal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
		PublicIncludePaths.AddRange(new string[] {
			"MultiplayerPortal",
			"MultiplayerPortal/Variant_Horror",
			"MultiplayerPortal/Variant_Horror/UI",
			"MultiplayerPortal/Variant_Shooter",
			"MultiplayerPortal/Variant_Shooter/AI",
			"MultiplayerPortal/Variant_Shooter/UI",
			"MultiplayerPortal/Variant_Shooter/Weapons",
			"MultiplayerPortal/Variant_Portal",
			"MultiplayerPortal/Variant_Portal/Portal",
			"MultiplayerPortal/Variant_Portal/Weapons",
			"MultiplayerPortal/Variant_Portal/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
