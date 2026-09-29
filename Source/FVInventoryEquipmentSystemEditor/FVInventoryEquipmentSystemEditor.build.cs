using System.IO;
using UnrealBuildTool;

public class FVInventoryEquipmentSystemEditor : ModuleRules
{
	public FVInventoryEquipmentSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// ZIP library (zip.h / zip.c / miniz.h)
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "zip"));
		PublicDefinitions.Add("ZIP_STATIC");

		var mzPlatform = "0";
		if (Target.Platform == UnrealTargetPlatform.Win64)  mzPlatform = "1";
		else if (Target.Platform == UnrealTargetPlatform.Mac)   mzPlatform = "2";
		else if (Target.Platform == UnrealTargetPlatform.Linux) mzPlatform = "3";
		PublicDefinitions.Add("MZ_PLATFORM=" + mzPlatform);

		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] 
			{
				"FVInventoryEquipmentSystemEditor/Private"
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core", 
				"CoreUObject", 
				"Engine", 
				"UnrealEd",
				"AssetTools", 
				
				"FVInventoryEquipmentSystem",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				"FVInventoryEquipmentSystem",

				"UnrealEd",
				"DeveloperSettings",
				"Projects",

				"Slate",
				"SlateCore",
				"EditorStyle",

				"BlueprintGraph",
				"ToolMenus",
				"AssetTools",
				"GraphEditor",
				"PropertyEditor",
				"Kismet",
				"KismetWidgets",
				"ApplicationCore",
				"MainFrame",

				"InputCore",
				"EnhancedInput",
				"GameplayTags",
				"GameplayTagsEditor",
				"UMGEditor",
				
				"Blutility",
				
				"DesktopPlatform", 
				
				"EditorSubsystem", 
				
				"Json",
				"EngineSettings",
				"WebBrowser"
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[] 
			{
			}
		);
	}
}
