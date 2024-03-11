// Copyright 2023 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

using UnrealBuildTool;

public class DTHttpServer : ModuleRules
{
	public DTHttpServer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
				{
					"Core",
				}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
				{
					"CoreUObject",
					"Engine",
					"Slate",
					"SlateCore", 
					"HTTPServer",
				}
			);
	}
}
