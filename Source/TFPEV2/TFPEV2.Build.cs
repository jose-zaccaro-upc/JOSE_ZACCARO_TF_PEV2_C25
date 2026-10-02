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
			"Slate",
			"SlateCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemNull",
			"OnlineSubsystemSteam",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TFPEV2",
			"TFPEV2/Public",
			"TFPEV2/Public/Characters",
			"TFPEV2/Public/Core",
			"TFPEV2/Public/Framework",
			"TFPEV2/Public/Gameplay",
			"TFPEV2/Public/Gameplay/Components",
			"TFPEV2/Public/Interfaces",
			"TFPEV2/Public/UI",
			"TFPEV2/Public/Weapons",
			"TFPEV2/Variant_Horror",
			"TFPEV2/Variant_Horror/UI",
			"TFPEV2/Variant_Shooter",
			"TFPEV2/Variant_Shooter/AI",
			"TFPEV2/Variant_Shooter/UI",
			"TFPEV2/Variant_Shooter/Weapons"
		});

		PrivateIncludePaths.AddRange(new string[] {
			"TFPEV2/Private",
		});
	}
}
