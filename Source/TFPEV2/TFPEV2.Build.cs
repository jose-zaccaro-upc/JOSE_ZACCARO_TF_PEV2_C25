// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TFPEV2 : ModuleRules
{
	public TFPEV2(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TFPEV2",
			"TFPEV2/Variant_Horror",
			"TFPEV2/Variant_Horror/UI",
			"TFPEV2/Variant_Shooter",
			"TFPEV2/Variant_Shooter/AI",
			"TFPEV2/Variant_Shooter/UI",
			"TFPEV2/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
