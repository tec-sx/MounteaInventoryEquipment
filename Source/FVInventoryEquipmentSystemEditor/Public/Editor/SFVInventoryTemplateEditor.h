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

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

class UFVInventoryItemTemplate;
class SFVInventoryTemplateSearchFilter;

DECLARE_DELEGATE_OneParam(FOnTemplateChanged, UFVInventoryItemTemplate*);

#define LOCTEXT_NAMESPACE "SFVInventoryTemplateEditor"

struct FTemplateDisplayInfo
{
	FText DisplayText;
	FText TooltipText;
	FText FullText;
	FString AssetPath;
	bool bIsTransient = false;
	bool bIsDirty = false;

	FTemplateDisplayInfo()
		: DisplayText(LOCTEXT("InvalidTemplate", "Invalid Template"))
		, TooltipText(LOCTEXT("InvalidTemplateTooltip", "Invalid Template"))
		, FullText(DisplayText)
		, AssetPath(TEXT("Unknown Path"))
	{}
};

enum class ETemplateTreeItemType : uint8
{
	Category,
	Template
};

struct FTemplateTreeItem
{
	ETemplateTreeItemType Type;
	FString CategoryName;
	TWeakObjectPtr<UFVInventoryItemTemplate> Template;
	TArray<TSharedPtr<FTemplateTreeItem>> Children;
	
	static TSharedPtr<FTemplateTreeItem> MakeCategory(const FString& InCategoryName)
	{
		TSharedPtr<FTemplateTreeItem> item = MakeShared<FTemplateTreeItem>();
		item->Type = ETemplateTreeItemType::Category;
		item->CategoryName = InCategoryName;
		return item;
	}
	
	static TSharedPtr<FTemplateTreeItem> MakeTemplate(TWeakObjectPtr<UFVInventoryItemTemplate> InTemplate)
	{
		TSharedPtr<FTemplateTreeItem> item = MakeShared<FTemplateTreeItem>();
		item->Type = ETemplateTreeItemType::Template;
		item->Template = InTemplate;
		return item;
	}
};

class SFVInventoryTemplateEditor : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SFVInventoryTemplateEditor) {}
	SLATE_EVENT(FOnTemplateChanged, OnTemplateChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);	
	virtual ~SFVInventoryTemplateEditor() override;

protected:
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

private:
	TSharedRef<SWidget> CreatePropertyMatrix();
	TSharedRef<SWidget> CreateToolbar();
	TSharedRef<ITableRow> GenerateTemplateTreeRow(TSharedPtr<FTemplateTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	
	void GetChildrenForTree(TSharedPtr<FTemplateTreeItem> Item, TArray<TSharedPtr<FTemplateTreeItem>>& OutChildren);
	void RebuildTreeStructure();

	static TArray<UObject*> LoadAllTemplatesForMatrix();
	void RefreshTemplateList();
	void OnTreeSelectionChanged(TSharedPtr<FTemplateTreeItem> SelectedItem, ESelectInfo::Type SelectInfo);
	
	void SaveExpansionState();
	void RestoreExpansionState();
	
	FReply OnCreateNewTemplate();
	FReply SaveTemplate();
	FReply SaveAllDirtyTemplates();
	FReply CloseTemplate();
	FReply SaveNewTemplate();
	FReply SaveExistingTemplate();
	
	FReply DeleteTemplate(TWeakObjectPtr<UFVInventoryItemTemplate> Template);
	FReply DuplicateTemplate(TWeakObjectPtr<UFVInventoryItemTemplate> Template);
	
	FReply ShowHelpModal();
	FReply OnCloseHelp();

	void ImportTemplate();
	void ExportTemplate();
	
	void CreateTransientTemplate();
	void CleanupTransientTemplate();
	bool ValidateTemplateData(FString& ErrorMessage) const;
	static void ShowTemplateEditorNotification(const FString& Message, const bool bSuccess = true);
	
	void TrackDirtyAsset(UFVInventoryItemTemplate* Template);
	void UntrackDirtyAsset(UFVInventoryItemTemplate* Template);
	bool HasUnsavedChanges() const;
	bool CheckForUnsavedChanges();
	
	static FString ShowSaveAssetDialog();

	static FTemplateDisplayInfo GenerateTemplateDisplayInfo(
		TWeakObjectPtr<UFVInventoryItemTemplate> Template,
		const TSet<TWeakObjectPtr<UFVInventoryItemTemplate>>& AllDirtyTemplates);

	TSharedPtr<FTemplateTreeItem> FindTreeItemForTemplate(const TWeakObjectPtr<UFVInventoryItemTemplate>& Template) const;

private:
	FOnTemplateChanged OnTemplateChanged;
	FDelegateHandle AssetRemovedHandle;
	FDelegateHandle AssetRenamedHandle;
	FDelegateHandle AssetAddedHandle;
	FDelegateHandle AssetUpdatedHandle;
	
	TSharedPtr<SOverlay> RootOverlay;
	TSharedPtr<class SFVItemTemplatesEditorHelp> HelpWidget;
	TSharedPtr<SWidget> HelpModalContent;
	
	TSharedPtr<IDetailsView> PropertyDetailsView;
	TSharedPtr<STreeView<TSharedPtr<FTemplateTreeItem>>> TemplateTreeView;
	TSharedPtr<SFVInventoryTemplateSearchFilter> SearchFilterWidget;
	
	TArray<TSharedPtr<FTemplateTreeItem>> TreeRootItems;
	TArray<TSharedPtr<FTemplateTreeItem>> FilteredTreeRootItems;
	TSet<FString> ExpandedCategories;
	
	TArray<TWeakObjectPtr<UFVInventoryItemTemplate>> AvailableTemplates;
	TSet<TWeakObjectPtr<UFVInventoryItemTemplate>> DirtyTemplates;
	
	TWeakObjectPtr<UFVInventoryItemTemplate> CurrentTemplate;
	TWeakObjectPtr<UFVInventoryItemTemplate> OriginalTemplate;
	bool bIsEditingExisting = false;
	bool bIsShowingTransient = false;
	
	TSet<FName> PendingRemovedObjectPaths;
	TWeakPtr<FActiveTimerHandle> PendingRefreshTimer;
	bool bPendingRefresh = false;
	
	TSet<UFVInventoryItemTemplate*> PendingChanges;
	FTimerHandle ProcessChangesTimer;
	float ChangeDebounceDelay = 0.5f;
	
	void ApplySearchFilter();
	
	void OnSearchTextChanged(const FText& InSearchText);
	void OnFiltersChanged();
	bool PassesFilters(TWeakObjectPtr<UFVInventoryItemTemplate> Template) const;

	static bool IsInventoryTemplateAsset(const FAssetData& AssetData);
	void OnAssetRegistryRemoved(const FAssetData& AssetData);
	void OnAssetRegistryRenamed(const FAssetData& AssetData, const FString& String);
	void OnAssetRegistryAdded(const FAssetData& AssetData);
	void OnAssetRegistryUpdated(const FAssetData& AssetData);
	EActiveTimerReturnType HandleDeferredRefresh(double, float);
	void BindAssetRegistry();
	void UnbindAssetRegistry();
	
	void BindItemChanged();
	void UnbindItemChanged();
	void OnItemChanged(UFVInventoryItemTemplate* Template);
	void ProcessPendingChanges();
};

#undef LOCTEXT_NAMESPACE