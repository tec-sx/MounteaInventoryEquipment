using UnrealBuildTool;
 
public class FVInventoryEquipmentSystemDeveloper : ModuleRules
{
	public FVInventoryEquipmentSystemDeveloper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"InputCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Projects",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayTags",
				
				"FVInventoryEquipmentSystem"
			}
		);

		if (Target.bBuildEditor == true) 
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"EditorStyle",
					"BlueprintGraph",
					"ApplicationCore",
					"UnrealEd",
					"GraphEditor",
					"KismetCompiler",
					"PropertyEditor",
					"ToolMenus",
					"AssetTools",
					"ContentBrowser"
				}
			);
		}
	}
}