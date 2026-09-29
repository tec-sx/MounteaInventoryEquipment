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

#include "FVAdvancedInventoryLoadoutConfig_Factory.h"

#include "Definitions/FVAdvancedInventoryLoadoutConfig.h"
#include "Statics/FVAdvancedInventoryLoadoutEditorStatics.h"
#include "Utilities/FVAdvancedInventoryEditorUtilities.h"

UFVAdvancedInventoryLoadoutConfigs_Factory::UFVAdvancedInventoryLoadoutConfigs_Factory()
{
	bCreateNew    = true;
	bEditAfterNew = true;
	bEditorImport = true;

	Formats.Add(TEXT("mntealoadout;FV Loadout File"));
	Formats.Add(TEXT("mntealoadouts;FV Loadouts File"));

	SupportedClass = UFVAdvancedInventoryLoadoutConfig::StaticClass();
}

UObject* UFVAdvancedInventoryLoadoutConfigs_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFVAdvancedInventoryLoadoutConfig>(InParent, ParentClass, Name, Flags, Context);
}

bool UFVAdvancedInventoryLoadoutConfigs_Factory::ConfigureProperties()
{
	static const FText TitleText = FText::FromString(TEXT("Pick Parent Class for new Inventory & Equipment Loadouts Config"));

	UClass* ChosenClass = nullptr;
	const bool bPressedOk = FFVAdvancedInventoryEditorUtilities::PickChildrenOfClass(TitleText, ChosenClass, UFVAdvancedInventoryLoadoutConfig::StaticClass());

	if (bPressedOk)
		ParentClass = ChosenClass;

	return bPressedOk;
}

bool UFVAdvancedInventoryLoadoutConfigs_Factory::FactoryCanImport(const FString& Filename)
{
	const FString ext = FPaths::GetExtension(Filename);
	return ext.Equals(TEXT("mntealoadout"), ESearchCase::IgnoreCase) ||
		   ext.Equals(TEXT("mntealoadouts"), ESearchCase::IgnoreCase);
}

UObject* UFVAdvancedInventoryLoadoutConfigs_Factory::FactoryCreateFile(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString targetFolder = InParent->GetPathName();

	TArray<UFVAdvancedInventoryLoadoutConfig*> importedLoadouts;
	FString errorMessage;

	if (!UFVAdvancedInventoryLoadoutEditorStatics::ImportLoadoutsFromFilePath(
		Filename, targetFolder, importedLoadouts, errorMessage))
	{
		if (Warn)
			Warn->Logf(ELogVerbosity::Error, TEXT("%s"), *errorMessage);
		bOutOperationCanceled = true;
		return nullptr;
	}

	return importedLoadouts.Num() > 0 ? importedLoadouts[0] : nullptr;
}

UObject* UFVAdvancedInventoryLoadoutConfigs_Factory::ImportObject(
	UClass* InClass, UObject* InOuter, FName InName, EObjectFlags Flags,
	const FString& Filename, const TCHAR* Parms, bool& OutCanceled)
{
	if (!FactoryCanImport(Filename))
		return nullptr;
	return FactoryCreateFile(InClass, InOuter, InName, Flags, Filename, Parms, nullptr, OutCanceled);
}
