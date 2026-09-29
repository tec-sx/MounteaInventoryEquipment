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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVAdvancedInventoryItemTemplateEditorStatics.generated.h"

struct FGameplayTagContainer;
class UFVInventoryItemTemplate;

/**
 * 
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEMEDITOR_API
	UFVAdvancedInventoryItemTemplateEditorStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
public:
	static bool ImportTemplatesFromFile(TArray<UFVInventoryItemTemplate*>& OutTemplates, FString& OutErrorMessage);
	static bool ImportTemplatesFromFilePath(const FString& FilePath, const FString& TargetFolder, TArray<UFVInventoryItemTemplate*>& OutTemplates, FString& OutErrorMessage);
	static bool ExportTemplatesToFile(const TArray<UFVInventoryItemTemplate*>& Templates, FString& OutErrorMessage);
	static bool ExportTemplatesToFilePath(const TArray<UFVInventoryItemTemplate*>& Templates, const FString& FilePath, FString& OutErrorMessage);

public:
	static TArray<UFVInventoryItemTemplate*> LoadAllExistingTemplates();
	static UFVInventoryItemTemplate* FindTemplateByGuid(const TArray<UFVInventoryItemTemplate*>& Templates, const FGuid& Guid);
	static bool UpdateExistingTemplate(UFVInventoryItemTemplate* Template, const FString& JsonString, FString& OutErrorMessage);
	static FGuid ExtractGuidFromJson(const FString& JsonString);
	static FString ExtractNameFromJson(const FString& JsonString);
	
	static bool ParseSingleTemplateJson(const FString& JsonString, UFVInventoryItemTemplate* Template, FString& OutErrorMessage);
	static bool ParseMultipleTemplatesJson(const FString& JsonString, TArray<FString>& OutItemJsons, FString& OutErrorMessage);

	// ZIP-format import (web companion export)
	static bool ParseItemJsonFromZip(const TArray<uint8>& ZipBytes, FString& OutItemJson, FString& OutErrorMessage);
	static bool ImportTemplatesFromZipBytes(const TArray<uint8>& ZipBytes, const FString& TargetFolder, TArray<UFVInventoryItemTemplate*>& OutTemplates, FString& OutErrorMessage);
    
	static UFVInventoryItemTemplate* CreateTemplateAsset(const FString& TargetFolder, const FString& AssetName, FString& OutErrorMessage);
	static bool SaveTemplateAsset(UFVInventoryItemTemplate* Template, const FString& PackagePath);
    
	static FString ShowOpenFileDialog(const FString& DialogTitle, const FString& FileTypes);
	static FString ShowSaveFileDialog(const FString& DialogTitle, const FString& DefaultFileName, const FString& FileTypes);
	static FString ShowFolderDialog(const FString& DialogTitle, const FString& DefaultPath);
	static FString ShowContentBrowserPathPicker(const FString& DialogTitle, const FString& DefaultPath);

	static bool DeserializeTemplateFromJson(const TSharedPtr<FJsonObject>& JsonObject, UFVInventoryItemTemplate* Template, FString& OutErrorMessage);
	static bool DeserializeGameplayTagContainer(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, FGameplayTagContainer& OutContainer);
	static bool DeserializeSoftClassPtrSet(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TSet<TSoftClassPtr<UObject>>& OutSet);
};
