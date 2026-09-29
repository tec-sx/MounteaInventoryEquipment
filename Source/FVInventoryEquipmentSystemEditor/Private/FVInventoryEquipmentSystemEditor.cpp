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

#include "FVInventoryEquipmentSystemEditor.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"

#include "AssetActions/FVAdvancedEquipmentComponent_AssetAction.h"
#include "AssetActions/FVAdvancedEquipmentSettingsConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryCallbackInventoryItemAction_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryComponent_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryGlobalConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryInteractiveWidgetConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryInteractiveWidgetEnvConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryItemTemplate_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryCraftingUIConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventorySelectableInventoryItemAction_AssetAction.h"
#include "AssetActions/FVAdvancedInventorySettingsConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryThemeConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryUIComponent_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryUIConfig_AssetAction.h"

#include "Commands/FMAISCommands.h"

#include "Customizations/FVJsonObjectDefinitionCustomization.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "Interfaces/IMainFrameModule.h"
#include "Interfaces/IPluginManager.h"
#include "PropertyEditorModule.h"

#include "Editor/SFVInventoryTemplateEditor.h"
#include "Popups/MAISSetupDefaultsPopup.h"
#include "Setup/FVInventorySetupUtilities.h"

#include "Styling/FVAdvancedInventoryEditorStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"

#include "Statics/FVInventoryEquipmentSystemEditorStatics.h"
#include "Visualiser/FVEquipmentComponentVisualizer.h"

#include "Widgets/Notifications/SNotificationList.h"
#include "Components/FVAttachmentContainerComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "AssetActions/FVAdvancedInventoryCraftingConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryGlobalUIConfig_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryLoadoutComponent_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryLoadoutConfigs_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryLoadoutItem_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryModalsDataTable_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryRecipeIngredientsList_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryRecipeIngredient_AssetAction.h"
#include "AssetActions/FVAdvancedInventoryRecipeTemplate_AssetAction.h"
#include "Settings/FVAdvancedInventorySettings.h"

#define LOCTEXT_NAMESPACE "FFVInventoryEquipmentSystemEditor"

static const FName MenuName("LevelEditor.LevelEditorToolBar.PlayToolBar");
const FName FFVInventoryEquipmentSystemEditor::InventoryTemplateEditorTabId = TEXT("FVInventoryTemplateEditor");
TSharedPtr<SFVInventoryTemplateEditor> CurrentEditor;

void FFVInventoryEquipmentSystemEditor::StartupModule()
{
	// Register Style
	{
		FFVAdvancedInventoryEditorStyle::Initialize();
	}
	
	// Register new Category
	{
		FAssetToolsModule::GetModule().Get().RegisterAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"), FText::FromString(TEXT("📦 FV Inventory & Equipment")));
	}

	// Thumbnails and Icons
	{
		AdvancedInventorySet = MakeShareable(new FSlateStyleSet("FVAdvancedInventorySet"));
		const TSharedPtr<IPlugin> pluginPtr = IPluginManager::Get().FindPlugin("FVInventoryEquipmentSystem");

		if (pluginPtr.IsValid())
		{
			{
				const FString contentDir = pluginPtr->GetBaseDir();
				AdvancedInventorySet->SetContentRoot(contentDir);

				struct FClassIconMapping
				{
					const TCHAR* ClassName;
					const TCHAR* ResourceName;
				};

				const FClassIconMapping classIconMappings[] =
				{
					{ TEXT("FVInventoryItemTemplate"), TEXT("FVInventoryItemTemplate") },
					{ TEXT("FVAdvancedInventorySettingsConfig"), TEXT("FVAdvancedInventorySettingsConfig") },
					{ TEXT("FVAdvancedEquipmentSettingsConfig"), TEXT("FVAdvancedEquipmentSettingsConfig") },
					{ TEXT("FVAdvancedInventoryThemeConfig"), TEXT("ThemeConfigClassIcon") },
					{ TEXT("FVAdvancedInventoryInteractiveWidgetConfig"), TEXT("InteractiveWidgetConfigIcon") },
					{ TEXT("FVAttachmentContainerComponent"), TEXT("AttachmentContainerIcon") },
					{ TEXT("FVEquipmentComponent"), TEXT("EquipmentComponentIcon") },
					{ TEXT("FVInventoryComponent"), TEXT("InventoryComponentIcon") },
					{ TEXT("FVInventoryUIComponent"), TEXT("InventoryUIComponentIcon") },
					{ TEXT("FVAttachableComponent"), TEXT("AttachableComponentIcon") },
					{ TEXT("FVAdvancedAttachmentSlot"), TEXT("AttachmentSlotIcon") },
					{ TEXT("FVAdvancedInventoryUIConfig"), TEXT("FVAdvancedInventoryUISettingsConfig") },
					{ TEXT("FVAdvancedInventoryPreviewEnvironmentSettings"), TEXT("PreviewEnvironmentSettingsIcon") },
					{ TEXT("FVSelectableInventoryItemAction"), TEXT("ItemActionIcon") },
					{ TEXT("FVCallbackInventoryItemAction"), TEXT("ItemActionCallbackIcon") },
					{ TEXT("FVAdvancedInventoryLoadoutConfig"), TEXT("LoadoutConfiguration") },
					{ TEXT("FVAdvancedInventoryLoadoutComponent"), TEXT("LoadoutComponent") },
					{ TEXT("FVAdvancedInventoryLoadoutItem"), TEXT("LoadoutItem") },
					{ TEXT("FVRecipeTemplate"), TEXT("RecipeBook") },
					{ TEXT("FVRecipeIngredientsList"), TEXT("RecipeIngredientsList") },
					{ TEXT("FVRecipeIngredient"), TEXT("FVRecipeIngredients") },
					{ TEXT("FVCraftingParticipantComponent"), TEXT("CraftingParticipantIcon") },
					{ TEXT("FVCraftingParticipantUIComponent"), TEXT("CraftingParticipantUIIcon") },
					{ TEXT("FVCraftingStationComponent"), TEXT("CraftingStationIcon") },
					{ TEXT("FVAdvancedCraftingConfig"), TEXT("CraftingConfigIcon") },
					{ TEXT("FVAdvancedCraftingUIConfig"), TEXT("CraftingUIConfigIcon") },
					{ TEXT("FVAdvancedInventoryGlobalConfig"), TEXT("GlobalUIConfig") },
					{ TEXT("FVAdvancedInventoryGlobalUIConfig"), TEXT("GlobalUIConfig") },
					{ TEXT("FVAdvancedInventoryModalsDataTable"), TEXT("ModalsDataTableIcon") }
				};

				const auto RegisterClassIcon = [this](const TCHAR* ClassName, const TCHAR* ResourceName)
				{
					const FString resourcePath = FString::Printf(TEXT("Resources/ClassIcons/%s"), ResourceName);
					const FString brushPath = AdvancedInventorySet->RootToContentDir(*resourcePath, TEXT(".png"));
					const FString thumbnailKey = FString::Printf(TEXT("ClassThumbnail.%s"), ClassName);
					const FString iconKey = FString::Printf(TEXT("ClassIcon.%s"), ClassName);

					AdvancedInventorySet->Set(*thumbnailKey, new FSlateImageBrush(brushPath, FVector2D(128.f, 128.f)));
					AdvancedInventorySet->Set(*iconKey, new FSlateImageBrush(brushPath, FVector2D(16.f, 16.f)));
				};

				for (const FClassIconMapping& classIconMapping : classIconMappings)
				{
					RegisterClassIcon(classIconMapping.ClassName, classIconMapping.ResourceName);
				}

				const FString jsonIconPath = AdvancedInventorySet->RootToContentDir(TEXT("Resources/WebIcons/json"), TEXT(".svg"));
				AdvancedInventorySet->Set(TEXT("ClassThumbnail.FVJsonObject"), new FSlateVectorImageBrush(jsonIconPath, FVector2D(128.f, 128.f)));
				AdvancedInventorySet->Set(TEXT("ClassIcon.FVJsonObject"), new FSlateVectorImageBrush(jsonIconPath, FVector2D(16.f, 16.f)));

				FSlateStyleRegistry::RegisterSlateStyle(*AdvancedInventorySet.Get());
			}
		}
	}

	// Asset Actions
	{
		AssetActions.Add(MakeShared<FFVAdvancedInventorySettingsConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryThemeConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryInteractiveWidgetConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedEquipmentSettingsConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryItemTemplate_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryComponent_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryUIComponent_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedEquipmentComponent_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryUIConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventorySelectableInventoryItemAction_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryLoadoutConfigs_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryLoadoutItem_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryLoadoutComponent_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryModalsDataTable_AssetAction>());
		
		AssetActions.Add(MakeShared<FFVAdvancedInventoryRecipeTemplate_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryRecipeIngredientsList_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryRecipeIngredient_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryCraftingConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryCraftingUIConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryGlobalConfig_AssetAction>());
		AssetActions.Add(MakeShared<FFVAdvancedInventoryGlobalUIConfig_AssetAction>());
		
		for (const auto& Itr : AssetActions)
		{
			FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(Itr.ToSharedRef());
		}
	}

	// Detail Customizations
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		RegisteredCustomPropertyTypeLayouts =
		{
			TEXT("FVJsonObjectDefinition")
		};

		PropertyModule.RegisterCustomPropertyTypeLayout(
			RegisteredCustomPropertyTypeLayouts[0],
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFVJsonObjectDefinitionCustomization::MakeInstance)
		);
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	// Register Menu Buttons
	{
		FMAISCommands::Register();

		PluginCommands = MakeShareable(new FUICommandList);
		PluginCommands->MapAction(
			FMAISCommands::Get().MAI_SetupDefaultsAction,
			FExecuteAction::CreateRaw(this, &FFVInventoryEquipmentSystemEditor::SetupDefaultsButtonClicked)
		);
		
		IMainFrameModule& mainFrame = FModuleManager::Get().LoadModuleChecked<IMainFrameModule>("MainFrame");
		mainFrame.GetMainFrameCommandBindings()->Append(PluginCommands.ToSharedRef());

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFVInventoryEquipmentSystemEditor::RegisterMenus));
	}

	// Register in Window tab
	{
		UToolMenu* editorMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help");
		{
			FToolMenuSection& editorMenuSection = editorMenu->FindOrAddSection("FVFramework");
			editorMenuSection.Label = FText::FromString(TEXT("FlickerVoid"));
						
			FToolMenuEntry menuEntry = editorMenuSection.AddMenuEntryWithCommandList
			(
				FMAISCommands::Get().MAI_FVSupportAction,
				PluginCommands,
				NSLOCTEXT("FVSupport", "TabTitle", "FV Support"),
				NSLOCTEXT("FVSupport", "TooltipText", "🆘 Open FlickerVoid Support channel"),
				FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Help")
			);
		}
	}

	/*
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		TArray<FOnGetDetailCustomizationInstance> CustomClassLayouts =
		{
			FOnGetDetailCustomizationInstance::CreateStatic(&FFVAttachmentContainerDetailsCustomization::MakeInstance),
		};
		RegisteredCustomClassLayouts =
		{
			UFVAttachmentContainerComponent::StaticClass()->GetFName(),
		};
		for (int32 i = 0; i < RegisteredCustomClassLayouts.Num(); i++)
		{
			PropertyModule.RegisterCustomClassLayout(RegisteredCustomClassLayouts[i], CustomClassLayouts[i]);
		}
	}
	*/
	// Register Tab Spawners
	{
		RegisterTabSpawners();
	}

	// Register Equipment Component Visualiser
	{
		if (GUnrealEd)
		{
			TSharedPtr<FComponentVisualizer> compVisualizer = MakeShareable(new FFVEquipmentComponentVisualizer);
			GUnrealEd->RegisterComponentVisualizer(UFVAttachmentContainerComponent::StaticClass()->GetFName(), compVisualizer);
			compVisualizer->OnRegister();
		}
	}
	
	// Validate WebBrowserWidget
	{
		if (!FModuleManager::Get().IsModuleLoaded("WebBrowserWidget"))
		{
			FText errorTitle = NSLOCTEXT("FVInventory", "WebBrowserMissing", "⚠ Missing Plugin ⚠");
			FText errorMessage = NSLOCTEXT("FVInventory", "WebBrowserMissingMessage", 
				"The 'Web Browser Widget' plugin is required for the Item Template Editor system.\n\n"
				"We need to enable the plugin and restart the Editor. Sorry for the inconvenience.");
		
			FMessageDialog::Open(EAppMsgType::Ok, errorMessage, errorTitle);
		
			FString projectFilePath = FPaths::ProjectDir() / FApp::GetProjectName() + TEXT(".uproject");			
			FString jsonString;
			if (FFileHelper::LoadFileToString(jsonString, *projectFilePath))
			{
				TSharedPtr<FJsonObject> jsonObject;
				TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
				
				if (FJsonSerializer::Deserialize(reader, jsonObject) && jsonObject.IsValid())
				{
					TArray<TSharedPtr<FJsonValue>> pluginsArray;
					if (jsonObject->HasField(TEXT("Plugins")))
						pluginsArray = jsonObject->GetArrayField(TEXT("Plugins"));
					
					bool bExists = false;
					for (const TSharedPtr<FJsonValue>& plugin : pluginsArray)
					{
						TSharedPtr<FJsonObject> pluginObj = plugin->AsObject();
						if (pluginObj.IsValid() && pluginObj->GetStringField(TEXT("Name")) == TEXT("WebBrowserWidget"))
						{
							pluginObj->SetBoolField(TEXT("Enabled"), true);
							bExists = true;
							break;
						}
					}
					
					if (!bExists)
					{
						TSharedPtr<FJsonObject> newPlugin = MakeShareable(new FJsonObject());
						newPlugin->SetStringField(TEXT("Name"), TEXT("WebBrowserWidget"));
						newPlugin->SetBoolField(TEXT("Enabled"), true);
						pluginsArray.Add(MakeShareable(new FJsonValueObject(newPlugin)));
					}
					
					jsonObject->SetArrayField(TEXT("Plugins"), pluginsArray);
					
					FString outputString;
					TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&outputString);
					if (FJsonSerializer::Serialize(jsonObject.ToSharedRef(), writer))
					{
						if (FFileHelper::SaveStringToFile(outputString, *projectFilePath))
							FUnrealEdMisc::Get().RestartEditor(false);
					}
				}
			}
		}
	}
}

void FFVInventoryEquipmentSystemEditor::ShutdownModule()
{
	// Thumbnails and Icons
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(AdvancedInventorySet->GetStyleSetName());
	}

	// Asset Types Cleanup
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			for (const auto& Itr : AssetActions)
			{
				FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(Itr.ToSharedRef());
			}
		}
	}

	// Detail Customizations Cleanup
	{
		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			for (const FName& layoutName : RegisteredCustomPropertyTypeLayouts)
				PropertyModule.UnregisterCustomPropertyTypeLayout(layoutName);

			PropertyModule.NotifyCustomizationModuleChanged();
		}
	}

	// Style Shutdown
	{
		FFVAdvancedInventoryEditorStyle::Shutdown();
	}

	// Help Button Cleanup
	{
		UToolMenus::UnRegisterStartupCallback(this);

		UToolMenus::UnregisterOwner(this);

		FMAISCommands::Unregister();
	}

	// Tab Spawners Cleanup
	{
		UnregisterTabSpawners();
	}

	// Equipment Component Visualiser Cleanup
	{
		if (GUnrealEd)
			GUnrealEd->UnregisterComponentVisualizer(UFVAttachmentContainerComponent::StaticClass()->GetFName());
	}
}

void FFVInventoryEquipmentSystemEditor::RegisterTabSpawners()
{
	FGlobalTabmanager::Get()->RegisterTabSpawner(
		InventoryTemplateEditorTabId,
		FOnSpawnTab::CreateRaw(this, &FFVInventoryEquipmentSystemEditor::SpawnInventoryTemplateEditorTab)
	)
	.SetDisplayName(LOCTEXT("InventoryTemplateEditorTabTitle", "FV Inventory Template Editor"))
	.SetTooltipText(LOCTEXT("InventoryTemplateEditorTooltipText", "Open the FV Inventory Template Editor"))
	.SetIcon(FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.TemplatesEditor")); 
}

void FFVInventoryEquipmentSystemEditor::UnregisterTabSpawners()
{
	if (FSlateApplication::IsInitialized())
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InventoryTemplateEditorTabId);
}

void FFVInventoryEquipmentSystemEditor::OpenInventoryTemplateEditor()
{
	FGlobalTabmanager::Get()->TryInvokeTab(InventoryTemplateEditorTabId);
}

TSharedRef<SDockTab> FFVInventoryEquipmentSystemEditor::SpawnInventoryTemplateEditorTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SFVInventoryTemplateEditor> newEditor = SNew(SFVInventoryTemplateEditor)
		.OnTemplateChanged_Lambda([](UFVInventoryItemTemplate* Template)
		{
			// Handle template changes if needed
			if (Template)
			{
				// Mark package as dirty
				Template->MarkPackageDirty();
			}
		});

	CurrentTemplateEditor = newEditor;

	TSharedRef<SDockTab> newTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("InventoryTemplateEditorTabTitle", "Inventory Template Editor"))
		.ToolTipText(LOCTEXT("InventoryTemplateEditorTooltipText", "Create and edit FV Inventory Item Templates"))
		[
			newEditor
		];
	
	newTab->SetTabIcon(FAppStyle::GetBrush("MAISStyleSet.TemplatesEditor"));

	return newTab;
}

void FFVInventoryEquipmentSystemEditor::SetupDefaultsButtonClicked()
{
	const FMAISSetupDefaultsReport report = FFVInventorySetupUtilities::RunSetupDefaults();
	MAISSetupDefaultsPopup::Open(report);
}

void FFVInventoryEquipmentSystemEditor::RegisterMenus()
{
	if (!UToolMenus::Get()->IsMenuRegistered(FVAdvancedInventoryToolbar::FVSharedMenuName))
		UToolMenus::Get()->RegisterMenu(FVAdvancedInventoryToolbar::FVSharedMenuName, NAME_None, EMultiBoxType::Menu, false);

	UToolMenu* toolbarMenu = UToolMenus::Get()->ExtendMenu(MenuName);
	FToolMenuSection& sharedSection = toolbarMenu->FindOrAddSection(FVAdvancedInventoryToolbar::FVSharedSectionName);
	sharedSection.Label = LOCTEXT("FVSharedSection_Label", "FlickerVoid");

	if (sharedSection.FindEntry(FVAdvancedInventoryToolbar::FVSharedButtonName) == nullptr)
	{
		FToolMenuEntry comboButton = FToolMenuEntry::InitComboButton(
			FVAdvancedInventoryToolbar::FVSharedButtonName,
			FUIAction(),
			FOnGetContent::CreateLambda([SharedMenuName = FVAdvancedInventoryToolbar::FVSharedMenuName, CmdList = PluginCommands]() -> TSharedRef<SWidget>
			{
				return UToolMenus::Get()->GenerateWidget(SharedMenuName, FToolMenuContext(CmdList));
			}),
			LOCTEXT("FVMainMenu_Label", "FlickerVoid"),
			LOCTEXT("FVMainMenu_Tooltip", "📂 Open FlickerVoid menu.\n\n❔ Provides access to all FlickerVoid tools, documentation, and support."),
			FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.FVLogo"),
			false,
			FVAdvancedInventoryToolbar::FVSharedButtonName
		);
		comboButton.StyleNameOverride = "CalloutToolbar";
		sharedSection.AddEntry(comboButton);
	}

	FToolMenuOwnerScoped OwnerScoped(this);
	{
		// Register in Help menu
		if (UToolMenu* toolMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help"))
		{
			if (toolMenu->ContainsSection("FVFramework") == false)
			{
				FToolMenuSection& fvSection = toolMenu->FindOrAddSection("FVFramework");
				fvSection.InsertPosition.Position = EToolMenuInsertType::First;
				fvSection.Label = FText::FromString(TEXT("FlickerVoid"));
				FToolMenuEntry supportEntry = fvSection.AddMenuEntryWithCommandList(
					FMAISCommands::Get().MAI_FVSupportAction,
					PluginCommands,
					LOCTEXT("FVSystemEditor_SupportButton_Label", "FV Support"),
					LOCTEXT("FVSystemEditor_SupportButton_ToolTip", "🆘 Open FlickerVoid Support channel"),
					FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Help")
				);
				supportEntry.Name = FName("FVFrameworkSupport");
			}
		}

		// Extend shared submenu with a nested Inventory submenu entry
		UToolMenu* sharedMenu = UToolMenus::Get()->ExtendMenu(FVAdvancedInventoryToolbar::FVSharedMenuName);
		{
			FToolMenuSection& invSection = sharedMenu->FindOrAddSection("FVInventory");
			invSection.Label = LOCTEXT("InvSection_Label", "FV Advanced Inventory & Equipment");
			invSection.AddEntry(FToolMenuEntry::InitSubMenu(
				"FVInventorySubMenu",
				LOCTEXT("InvSubMenu_Label", "FV Advanced Inventory"),
				LOCTEXT("InvSubMenu_Tooltip", "📦 FV Advanced Inventory & Equipment tools"),
				FNewToolMenuDelegate::CreateLambda([this](UToolMenu* subMenu)
				{
					{
						FToolMenuSection& toolsSection = subMenu->FindOrAddSection("FVInventory_Tools");
						toolsSection.Label = LOCTEXT("InvTools_Label", "FV Inventory Tools");
						toolsSection.AddMenuEntryWithCommandList(
							FMAISCommands::Get().MAI_SetupDefaultsAction,
							PluginCommands,
							LOCTEXT("FVSystemEditor_SetupDefaultsButton_Label", "Setup Defaults"),
							LOCTEXT("FVSystemEditor_SetupDefaultsButton_ToolTip", "Setup missing FV Advanced Inventory defaults and required player components."),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.AutoSetup")
						);
						toolsSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_TemplateEditor",
							LOCTEXT("FVSystemEditor_TemplateEditorButton_Label", "FV Inventory Template Editor"),
							LOCTEXT("FVSystemEditor_TemplateEditorButton_ToolTip", "🎨 Open FV Inventory Template Editor\n\n❔ Create and edit FV Inventory Item Templates with a comprehensive visual editor. Design items with properties, assets, materials, and custom attributes.\n\n💡 Features include:\n- Visual template creation\n- Asset management\n- Property validation\n- Items import/export"),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.TemplatesEditor"),
							FToolMenuExecuteAction::CreateLambda([this](const FToolMenuContext&) { OpenInventoryTemplateEditor(); })
						));
					}

					{
						FToolMenuSection& contentSection = subMenu->FindOrAddSection("FVInventory_Content");
						contentSection.Label = LOCTEXT("InvContent_Label", "FV Plugin Content");
						contentSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_ExampleLevel",
							LOCTEXT("FVSystemEditor_OpenExampleLevel_Label", "Open Example Level"),
							LOCTEXT("FVSystemEditor_OpenExampleLevel_ToolTip", "🌄 Opens an example level demonstrating FV Advanced Inventory & Equipment System"),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Level"),
							FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
							{
								const FString mapPath = TEXT("/FVInventoryEquipmentSystem/ExampleLevel/M_InventoryWorld");
								if (FPackageName::DoesPackageExist(mapPath))
									FEditorFileUtils::LoadMap(mapPath, false, true);
								else
								{
									FNotificationInfo notifInfo(LOCTEXT("FVSystemEditor_OpenExampleLevel_Notification", "Unable to open example level!"));
									notifInfo.bUseSuccessFailIcons = true;
									notifInfo.SubText = LOCTEXT("FVSystemEditor_OpenExampleLevel_Notification_SubText",
										"The example level is missing. Please, verify that the FV Advanced Inventory & Equipment plugin is installed correctly!");
									TSharedPtr<SNotificationItem> notificationItem = FSlateNotificationManager::Get().AddNotification(notifInfo);
									if (notificationItem.IsValid())
										notificationItem->SetCompletionState(SNotificationItem::CS_Fail);
								}
							})
						));
						contentSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_PluginFolder",
							LOCTEXT("FVSystemEditor_OpenPluginFolder_Label", "Open Plugin Folder"),
							LOCTEXT("FVSystemEditor_OpenPluginFolder_ToolTip", "📂 Open the FV Advanced Inventory & Equipment plugin's folder"),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Folder"),
							FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
							{
								const FContentBrowserModule& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
								TArray<FString> folderPaths;
								folderPaths.Add(TEXT("/FVInventoryEquipmentSystem"));
								contentBrowserModule.Get().SetSelectedPaths(folderPaths, true);
							})
						));
					}

					{
						FToolMenuSection& configSection = subMenu->FindOrAddSection("FVInventory_Config");
						configSection.Label = LOCTEXT("InvConfig_Label", "FV Advanced Inventory Configs");
						
						// Global UI Config
						configSection.AddEntry(FToolMenuEntry::InitSubMenu(
							"FVInventory_Config_GlobalConfigSubMenu",
							LOCTEXT("FVSystemEditor_GlobalConfigSubMenu_Label", "Shared Config"),
							LOCTEXT("FVSystemEditor_GlobalConfigSubMenu_ToolTip", "📄 Open FV Inventory & Equipment shared configuration menus."),
							FNewToolMenuDelegate::CreateLambda([](UToolMenu* globalConfigSubMenu)
							{
								FToolMenuSection& inventoryConfigSection = globalConfigSubMenu->FindOrAddSection("FVInventory_Config_GlobalConfig");
								inventoryConfigSection.Label = LOCTEXT("InvConfigGlobalConfig_Label", "Shared Configs");
								inventoryConfigSection.AddEntry(FToolMenuEntry::InitMenuEntry(
									"FVInventory_GlobalConfig",
									LOCTEXT("FVSystemEditor_GlobalInventoryConfigButton_Label", "Global Config"),
									LOCTEXT("FVSystemEditor_GlobalInventoryConfigButton_ToolTip", "📄 Open FV Inventory & Equipment Global Configuration\n\n❔ Define shared data definitions, including named JSON object definitions used for JSON generation and validation."),
									FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config"),
									FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
									{
										FText errorMessage;
										const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
										UFVInventoryEquipmentSystemEditorStatics::OpenConfig(
											settings ? settings->GlobalConfig.ToSoftObjectPath() : FSoftObjectPath(),
											errorMessage,
											LOCTEXT("FVSystemEditor_GlobalInventoryConfigButton_Error", "Unable to locate the FV Inventory Global Config asset.\nPlease, open Inventory & Equipment Settings and select proper Config!"));
									})
								));
								inventoryConfigSection.AddEntry(FToolMenuEntry::InitMenuEntry(
									"FVInventory_Config",
									LOCTEXT("FVSystemEditor_GlobalConfigButton_Label", "Global UI Config"),
									LOCTEXT("FVSystemEditor_GlobalConfigButton_ToolTip", "📄 Open FV Inventory & Equipment Global UI Configuration\n\n❔ Define shared UI elements, like Theme, Fonts, Modals etc."),
									FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config"),
									FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
									{
										FText errorMessage;
										const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
										UFVInventoryEquipmentSystemEditorStatics::OpenConfig(
											settings ? settings->GlobalUIConfig.ToSoftObjectPath() : FSoftObjectPath(),
											errorMessage,
											LOCTEXT("FVSystemEditor_GlobalConfigButton_Error", "Unable to locate the FV Inventory Config asset.\nPlease, open Inventory & Equipment Settings and select proper Config!"));
									})
								));
							}),
							false,
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config")
						));
						
						// Inventory Config
						configSection.AddEntry(FToolMenuEntry::InitSubMenu(
							"FVInventory_Config_InventorySubMenu",
							LOCTEXT("FVSystemEditor_InventoryConfigSubMenu_Label", "Inventory"),
							LOCTEXT("FVSystemEditor_InventoryConfigSubMenu_ToolTip", "📄 Open FV Inventory configuration menus."),
							FNewToolMenuDelegate::CreateLambda([](UToolMenu* inventoryConfigSubMenu)
							{
								FToolMenuSection& inventoryConfigSection = inventoryConfigSubMenu->FindOrAddSection("FVInventory_Config_Inventory");
								inventoryConfigSection.Label = LOCTEXT("InvConfigInventory_Label", "Inventory");
								inventoryConfigSection.AddEntry(FToolMenuEntry::InitMenuEntry(
									"FVInventory_Config",
									LOCTEXT("FVSystemEditor_ConfigButton_Label", "FV Advanced Inventory Config"),
									LOCTEXT("FVSystemEditor_ConfigButton_ToolTip", "📄 Open FV Inventory Configuration\n\n❔ Define inventory types, rarities, categories, limits and notification behaviors used by the core system."),
									FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config"),
									FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
									{
										FText errorMessage;
										const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
										UFVInventoryEquipmentSystemEditorStatics::OpenConfig(
											settings ? settings->AdvancedInventorySettingsConfig.ToSoftObjectPath() : FSoftObjectPath(),
											errorMessage,
											LOCTEXT("FVSystemEditor_ConfigButton_Error", "Unable to locate the FV Inventory Config asset.\nPlease, open Inventory & Equipment Settings and select proper Config!"));
									})
								));
								inventoryConfigSection.AddEntry(FToolMenuEntry::InitMenuEntry(
									"FVInventory_UIConfig",
									LOCTEXT("FVSystemEditor_UIConfigButton_Label", "FV Advanced Inventory UI Config"),
									LOCTEXT("FVSystemEditor_UIConfigButton_ToolTip", "📄 Open FV Inventory UI Configuration\n\n❔ Define inventory UI classes, styles and themes."),
									FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config"),
									FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
									{
										FText errorMessage;
										const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
										UFVInventoryEquipmentSystemEditorStatics::OpenConfig(
											settings ? settings->AdvancedInventoryUISettingsConfig.ToSoftObjectPath() : FSoftObjectPath(),
											errorMessage,
											LOCTEXT("FVSystemEditor_UIConfigButton_Error", "Unable to locate the FV Inventory UI Config asset.\nPlease, open Inventory & Equipment Settings and select proper Config!"));
									})
								));
							}),
							false,
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config")
						));
						
						// Equipment Config
						configSection.AddEntry(FToolMenuEntry::InitSubMenu(
							"FVInventory_Config_EquipmentSubMenu",
							LOCTEXT("FVSystemEditor_EquipmentConfigSubMenu_Label", "Equipment"),
							LOCTEXT("FVSystemEditor_EquipmentConfigSubMenu_ToolTip", "📄 Open FV Equipment configuration menus."),
							FNewToolMenuDelegate::CreateLambda([](UToolMenu* equipmentConfigSubMenu)
							{
								FToolMenuSection& equipmentConfigSection = equipmentConfigSubMenu->FindOrAddSection("FVInventory_Config_Equipment");
								equipmentConfigSection.Label = LOCTEXT("InvConfigEquipment_Label", "Equipment");
								equipmentConfigSection.AddEntry(FToolMenuEntry::InitMenuEntry(
									"FVInventory_EquipmentConfig",
									LOCTEXT("FVSystemEditor_EquipmentConfigButton_Label", "FV Advanced Equipment Config"),
									LOCTEXT("FVSystemEditor_EquipmentConfigButton_ToolTip", "📄 Open FV Equipment Configuration\n\n❔ Define Equipment types, slots, rules and other configuration."),
									FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config"),
									FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
									{
										FText errorMessage;
										const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
										UFVInventoryEquipmentSystemEditorStatics::OpenConfig(
											settings ? settings->AdvancedEquipmentSettingsConfig.ToSoftObjectPath() : FSoftObjectPath(),
											errorMessage,
											LOCTEXT("FVSystemEditor_EquipmentConfigButton_Error", "Unable to locate the FV Equipment Config asset.\nPlease, open Inventory & Equipment Settings and select proper Config!"));
									})
								));
							}),
							false,
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config")
						));
						
						// Crafting Config
						configSection.AddEntry(FToolMenuEntry::InitSubMenu(
							"FVInventory_Config_CraftingSubMenu",
							LOCTEXT("FVSystemEditor_CraftingConfigSubMenu_Label", "Crafting"),
							LOCTEXT("FVSystemEditor_CraftingConfigSubMenu_ToolTip", "📄 Open FV Crafting configuration menus."),
							FNewToolMenuDelegate::CreateLambda([](UToolMenu* equipmentConfigSubMenu)
							{
								FToolMenuSection& equipmentConfigSection = equipmentConfigSubMenu->FindOrAddSection("FVInventory_Config_Crafting");
								equipmentConfigSection.Label = LOCTEXT("InvConfigCrafting_Label", "Crafting");
								equipmentConfigSection.AddEntry(FToolMenuEntry::InitMenuEntry(
									"FVInventory_CraftingConfig",
									LOCTEXT("FVSystemEditor_CraftingConfigButton_Label", "FV Advanced Crafting Config"),
									LOCTEXT("FVSystemEditor_CraftingConfigButton_ToolTip", "Open FV Crafting Configuration"),
									FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config"),
									FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
									{
										FText errorMessage;
										const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
										UFVInventoryEquipmentSystemEditorStatics::OpenConfig(
											settings ? settings->AdvancedCraftingSettingsConfig.ToSoftObjectPath() : FSoftObjectPath(),
											errorMessage,
											LOCTEXT("FVSystemEditor_CraftingConfigButton_Error", "Unable to locate the FV Crafting Config asset.\nPlease, open Inventory & Equipment Settings and select proper Config!"));
									})
								));
								equipmentConfigSection.AddEntry(FToolMenuEntry::InitMenuEntry(
									"FVInventory_CraftingUIConfig",
									LOCTEXT("FVSystemEditor_CraftingUIConfigButton_Label", "FV Advanced Crafting UI Config"),
									LOCTEXT("FVSystemEditor_CraftingUIConfigButton_ToolTip", "Open FV Crafting UI Configuration"),
									FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config"),
									FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
									{
										FText errorMessage;
										const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
										UFVInventoryEquipmentSystemEditorStatics::OpenConfig(
											settings ? settings->AdvancedCraftingUISettingsConfig.ToSoftObjectPath() : FSoftObjectPath(),
											errorMessage,
											LOCTEXT("FVSystemEditor_CraftingUIConfigButton_Error", "Unable to locate the FV Crafting UI Config asset.\nPlease, open Inventory & Equipment Settings and select proper Config!"));
									})
								));
							}),
							false,
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Config")
						));
					}

					{
						FToolMenuSection& settingsSection = subMenu->FindOrAddSection("FVInventory_Settings");
						settingsSection.Label = LOCTEXT("InvSettings_Label", "FV Advanced Inventory Settings");
						settingsSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_Settings",
							LOCTEXT("FVSystemEditor_SettingsButton_Label", "FV Advanced Inventory Settings"),
							LOCTEXT("FVSystemEditor_SettingsButton_ToolTip", "⚙ Open FV Advanced Inventory Settings\n\n❔ Configure core Advanced Inventory system settings including default behaviors, performance options, and Advanced Inventory flow parameters. Customize your Advanced Inventory system's foundation here."),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Settings"),
							FToolMenuExecuteAction::CreateLambda([this](const FToolMenuContext&)
							{
								UFVInventoryEquipmentSystemEditorStatics::OpenSettings("Project",  TEXT("FlickerVoid"), TEXT("FV Inventory System"));
							})
						));
						settingsSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_EditorSettings",
							LOCTEXT("FVSystemEditor_EditorSettingsButton_Label", "FV Advanced Inventory Editor Settings"),
							LOCTEXT("FVSystemEditor_EditorSettingsButton_ToolTip", "⚙ Open FV Advanced Inventory Editor Settings\n\n❔ Customize your Advanced Inventory editor experience with settings for:TBD\n\nAll settings are saved in DefaultFVSettings.ini"),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Settings"),
							FToolMenuExecuteAction::CreateLambda([this](const FToolMenuContext&)
							{
								UFVInventoryEquipmentSystemEditorStatics::OpenSettings("Project",  TEXT("FlickerVoid"), TEXT("FV Inventory System (Editor)"));
							})
						));
					}

					{
						FToolMenuSection& linksSection = subMenu->FindOrAddSection("FVInventory_Links");
						linksSection.Label = LOCTEXT("InvLinks_Label", "FV Inventory Links");
						linksSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_Support",
							LOCTEXT("FVSystemEditor_SupportButton_Label", "FV Support"),
							LOCTEXT("FVSystemEditor_SupportButton_ToolTip", "🆘 Open FlickerVoid Support Channel\n\n❔ Get direct assistance from our support team and community. Find solutions to common issues, share your experiences, and get help with implementation challenges. Join our active community of developers!"),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Help"),
							FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
							{
								FPlatformProcess::LaunchURL(TEXT(""), nullptr, nullptr);
							})
						));
						linksSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_Wiki",
							LOCTEXT("FVSystemEditor_WikiButton_Label", "FV Advanced Inventory Wiki"),
							LOCTEXT("FVSystemEditor_WikiButton_ToolTip", "📖 Open FV Advanced Inventory Documentation\n\n❔ Access comprehensive guides, tutorials, and API references. Find detailed examples, best practices, and advanced features to master the FV Advanced Inventory System. Your one-stop resource for all documentation needs."),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Wiki"),
							FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
							{
								FPlatformProcess::LaunchURL(TEXT(""), nullptr, nullptr);
							})
						));
						linksSection.AddEntry(FToolMenuEntry::InitMenuEntry(
							"FVInventory_Youtube",
							LOCTEXT("FVSystemEditor_YoutubeButton_Label", "FV Advanced Inventory Youtube"),
							LOCTEXT("FVSystemEditor_YoutubeButton_ToolTip", "👁️ Watch FV Advanced Inventory Youtube Videos\n\n❔ Visual learning resources featuring step-by-step tutorials, implementation guides, and practical examples. Perfect for both beginners and advanced users looking to expand their knowledge through video content."),
							FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Youtube"),
							FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
							{
								FPlatformProcess::LaunchURL(TEXT(""), nullptr, nullptr);
							})
						));
					}

				}),
				false,
				FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.InventorySystemIcon")
			));
		}

		// Shared Tools section are always last; all plugins contribute; FindEntry guards prevent duplicates
		{
			FToolMenuSection& toolsSection = sharedMenu->FindOrAddSection("FVFramework_Tools");
			toolsSection.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::Last);
			toolsSection.Label = LOCTEXT("Tools_Label", "FV Tools");

			if (toolsSection.FindEntry("FVInventory_InventoryManager") == nullptr)
				toolsSection.AddEntry(FToolMenuEntry::InitMenuEntry(
					"FVInventory_InventoryManager",
					LOCTEXT("FVSystemEditor_InventoryManagerButton_Label", "FV Inventory Manager"),
					LOCTEXT("FVSystemEditor_InventoryManagerButton_ToolTip", "💼 Open FV Inventory Manager Tool\n\n❔ A browser-based Inventory Template Editor for Unreal Engine projects — manage item templates online from any device. Simple, fast, and accessible.\n\n💡 Features include:\n- Create and edit item templates\n- Manage item properties and categories\n- Export templates for use in Unreal Engine projects"),
					FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.InventoryManager"),
					FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
					{
						FPlatformProcess::LaunchURL(TEXT(""), nullptr, nullptr);
					})
				));

			if (toolsSection.FindEntry("FVFramework_Dialoguer") == nullptr)
				toolsSection.AddEntry(FToolMenuEntry::InitMenuEntry(
					"FVFramework_Dialoguer",
					LOCTEXT("FVSystemEditor_DialoguerButton_Label", "FV Dialoguer"),
					LOCTEXT("FVSystemEditor_DialoguerButton_ToolTip", "⛰ Open FV Dialoguer Standalone Tool\n\n❔ A powerful standalone dialogue crafting tool designed for narrative designers and writers. Create, edit, and manage complex dialogue trees with an intuitive interface. Seamlessly import your `.mnteadlg` files directly into the FV Dialogue System.\n\n💡 Perfect for teams wanting to separate dialogue content creation from engine implementation."),
					FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Dialoguer"),
					FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
					{
						FPlatformProcess::LaunchURL(TEXT(""), nullptr, nullptr);
					})
				));

			if (toolsSection.FindEntry("FVFramework_Launcher") == nullptr)
				toolsSection.AddEntry(FToolMenuEntry::InitMenuEntry(
					"FVFramework_Launcher",
					LOCTEXT("FVSystemEditor_LauncherButton_Label", "FV Project Launcher"),
					LOCTEXT("FVSystemEditor_LauncherButton_ToolTip", "🚀 Open FV Project Launcher\n\n❔ A versatile standalone tool for streamlined project testing and deployment. Launch your projects with customized configurations, test different build settings, and validate implementations in various environments.\n\n💡 Features include:\n- Multiple configuration profiles\n- Quick-launch presets\n- Custom command-line parameters\n- Integrated testing tools"),
					FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Launcher"),
					FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
					{
						FPlatformProcess::LaunchURL(TEXT(""), nullptr, nullptr);
					})
				));

			if (toolsSection.FindEntry("FVFramework_Builder") == nullptr)
				toolsSection.AddEntry(FToolMenuEntry::InitMenuEntry(
					"FVFramework_Builder",
					LOCTEXT("FVSystemEditor_BuilderButton_Label", "FV Unreal Builder"),
					LOCTEXT("FVSystemEditor_BuilderButton_ToolTip", "🏗 Open FV Unreal Builder\n\n❔ A desktop application for automating Unreal Engine plugin and project builds across multiple engine versions and platforms."),
					FSlateIcon(FFVAdvancedInventoryEditorStyle::GetAppStyleSetName(), "MAISStyleSet.Builder"),
					FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&)
					{
						FPlatformProcess::LaunchURL(TEXT(""), nullptr, nullptr);
					})
				));
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVInventoryEquipmentSystemEditor, FVInventoryEquipmentSystemEditor)
