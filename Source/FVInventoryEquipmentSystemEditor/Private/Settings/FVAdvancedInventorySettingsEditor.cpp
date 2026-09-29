// Copyright (C) 2025 Dominik (Pavlicek) Morse. All rights reserved.
//
// Developed for the Mountea Framework as a free tool. This solution is provided
// for use and sharing without charge. Redistribution is allowed under the following conditions:
//
// - You may use this solution in commercial products, provided the product is not 
//   this solution itself (or unless significant modifications have been made to the solution).
// - You may not resell or redistribute the original, unmodified solution.
//
// For more information, visit: https://mountea.tools


#include "Settings/FVAdvancedInventorySettingsEditor.h"

#include "Definitions/FVAdvancedInventoryEditorTypes.h"
#include "Interfaces/IPluginManager.h"

UFVAdvancedInventorySettingsEditor::UFVAdvancedInventorySettingsEditor() : bDisplayEditorButtonText(false)
{
	CategoryName = TEXT("FlickerVoid");
	SectionName = TEXT("FV Inventory System (Editor)");
	
	auto ResolvePluginPath = [](const FString& RelativePath) -> FString
	{
		const FString pluginDir =
			IPluginManager::Get().FindPlugin(TEXT("FVInventoryEquipmentSystem"))->GetBaseDir();

		const FString fullPath = FPaths::Combine(pluginDir, RelativePath);
		return FPaths::ConvertRelativePathToFull(fullPath);
	};

	SharedStylesheetPath.FilePath = ResolvePluginPath(TEXT("Resources/Help/ItemTemplateEditor.css"));
	SharedScriptPath.FilePath = ResolvePluginPath(TEXT("Resources/Help/ItemTemplateEditor.js"));
	
	FItemTemplateEditorPageConfig firstPage(
	NSLOCTEXT("FVAdvancedInventorySettingsEditor", "EditorTemplatePages_FirstPage", "Introduction"),
	ResolvePluginPath(TEXT("Resources/Help/page_0.html"))
);

	FItemTemplateEditorPageConfig secondPage(
		NSLOCTEXT("FVAdvancedInventorySettingsEditor", "EditorTemplatePages_SecondPage", "Configuration"),
		ResolvePluginPath(TEXT("Resources/Help/page_1.html"))
	);

	FItemTemplateEditorPageConfig thirdPage(
		NSLOCTEXT("FVAdvancedInventorySettingsEditor", "EditorTemplatePages_ThirdPage", "Create New Template"),
		ResolvePluginPath(TEXT("Resources/Help/page_2.html"))
	);

	FItemTemplateEditorPageConfig fourthPage(
		NSLOCTEXT("FVAdvancedInventorySettingsEditor", "EditorTemplatePages_FourthPage", "Exporting Templates"),
		ResolvePluginPath(TEXT("Resources/Help/page_3.html"))
	);

	FItemTemplateEditorPageConfig fifthPage(
		NSLOCTEXT("FVAdvancedInventorySettingsEditor", "EditorTemplatePages_FifthPage", "Importing Templates"),
		ResolvePluginPath(TEXT("Resources/Help/page_4.html"))
	);

	FItemTemplateEditorPageConfig sixthPage(
		NSLOCTEXT("FVAdvancedInventorySettingsEditor", "EditorTemplatePages_SixthPage", "Advanced Tips"),
		ResolvePluginPath(TEXT("Resources/Help/page_5.html"))
	);

	EditorTemplatePages.Add(0, firstPage);
	EditorTemplatePages.Add(1, secondPage);
	EditorTemplatePages.Add(2, thirdPage);
	EditorTemplatePages.Add(3, fourthPage);
	EditorTemplatePages.Add(4, fifthPage);
	EditorTemplatePages.Add(5, sixthPage);

}
