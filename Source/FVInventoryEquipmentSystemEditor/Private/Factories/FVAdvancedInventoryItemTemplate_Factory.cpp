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


#include "FVAdvancedInventoryItemTemplate_Factory.h"

#include "Definitions/FVInventoryItemTemplate.h"
#include "Statics/FVAdvancedInventoryItemTemplateEditorStatics.h"
#include "Subsystems/FVInventoryTemplateEditorSubsystem.h"
#include "Utilities/FVAdvancedInventoryEditorUtilities.h"


UFVAdvancedInventoryItemTemplate_Factory::UFVAdvancedInventoryItemTemplate_Factory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	Formats.Add("mnteaitem;FV Item Template File");
	Formats.Add("mnteaitems;FV Item Templates File");
	SupportedClass = UFVInventoryItemTemplate::StaticClass();
}

UObject* UFVAdvancedInventoryItemTemplate_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, const FName Name,
	const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFVInventoryItemTemplate>(InParent, ParentClass, Name, Flags, Context);
}

bool UFVAdvancedInventoryItemTemplate_Factory::ConfigureProperties()
{
	static const FText TitleText = FText::FromString(TEXT("Pick Parent Class for new FV Inventory Item Template"));
	
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = FFVAdvancedInventoryEditorUtilities::PickChildrenOfClass
	(
		TitleText, ChosenClass, UFVInventoryItemTemplate::StaticClass()
	);

	if ( bPressedOk )
		ParentClass = ChosenClass;

	return bPressedOk;
}

bool UFVAdvancedInventoryItemTemplate_Factory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension.Equals(TEXT("mnteaitem"), ESearchCase::IgnoreCase) || 
		   Extension.Equals(TEXT("mnteaitems"), ESearchCase::IgnoreCase);
}

bool UFVAdvancedInventoryItemTemplate_Factory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (Obj == nullptr) return false;
	UFVInventoryItemTemplate* itemTemplate = Cast<UFVInventoryItemTemplate>(Obj);
	if (itemTemplate == nullptr) return false;
	
	return true;
}

EReimportResult::Type UFVAdvancedInventoryItemTemplate_Factory::Reimport(UObject* Obj)
{
	bool bReimportSuccess = false;
	return bReimportSuccess ? EReimportResult::Succeeded : EReimportResult::Failed;
}

void UFVAdvancedInventoryItemTemplate_Factory::PostImportCleanUp()
{
	FReimportHandler::PostImportCleanUp();
	
	ReimportPaths.Empty();
}

UObject* UFVAdvancedInventoryItemTemplate_Factory::FactoryCreateFile(UClass* InClass, UObject* InParent,
	const FName InName, const EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn,
	bool& bOutOperationCanceled)
{
	const FString targetFolder = InParent->GetPathName();
    
	TArray<UFVInventoryItemTemplate*> importedTemplates;
	FString errorMessage;
    
	if (!UFVAdvancedInventoryItemTemplateEditorStatics::ImportTemplatesFromFilePath(Filename, targetFolder, importedTemplates, errorMessage))
	{
		if (Warn)
			Warn->Logf(ELogVerbosity::Error, TEXT("%s"), *errorMessage);
		bOutOperationCanceled = true;
		return nullptr;
	}

	if (importedTemplates.Num() == 0)
	{
		bOutOperationCanceled = true;
		return nullptr;
	}

	if (UFVInventoryTemplateEditorSubsystem* editorTemplateSubsystem = GEditor->GetEditorSubsystem<UFVInventoryTemplateEditorSubsystem>())
		editorTemplateSubsystem->NotifyTemplatesChanged();

	return importedTemplates[0];
}

UObject* UFVAdvancedInventoryItemTemplate_Factory::ImportObject(UClass* InClass, UObject* InOuter, const FName InName,
	const EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, bool& OutCanceled)
{
	if (!FactoryCanImport(Filename))
		return nullptr;
	return FactoryCreateFile(InClass, InOuter, InName, Flags, Filename, Parms, nullptr, OutCanceled);
}
