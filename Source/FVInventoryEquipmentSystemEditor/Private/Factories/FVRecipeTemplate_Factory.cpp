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

#include "FVRecipeTemplate_Factory.h"

#include "Definitions/FVRecipeTemplate.h"
#include "Statics/FVAdvancedInventoryRecipeEditorStatics.h"
#include "Utilities/FVAdvancedInventoryEditorUtilities.h"

UFVRecipeTemplate_Factory::UFVRecipeTemplate_Factory()
{
	bCreateNew    = true;
	bEditAfterNew = true;
	bEditorImport = true;

	Formats.Add(TEXT("mntearecipe;FV Recipe File"));
	Formats.Add(TEXT("mntearecipes;FV Recipes File"));

	SupportedClass = UFVRecipeTemplate::StaticClass();
}

UObject* UFVRecipeTemplate_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFVRecipeTemplate>(InParent, ParentClass, Name, Flags, Context);
}

bool UFVRecipeTemplate_Factory::ConfigureProperties()
{
	static const FText TitleText = FText::FromString(TEXT("Pick Parent Class for new FV Recipe Template"));

	UClass* ChosenClass = nullptr;
	const bool bPressedOk = FFVAdvancedInventoryEditorUtilities::PickChildrenOfClass(
		TitleText, ChosenClass, UFVRecipeTemplate::StaticClass());

	if (bPressedOk)
		ParentClass = ChosenClass;

	return bPressedOk;
}

bool UFVRecipeTemplate_Factory::FactoryCanImport(const FString& Filename)
{
	const FString ext = FPaths::GetExtension(Filename);
	return ext.Equals(TEXT("mntearecipe"), ESearchCase::IgnoreCase) ||
		   ext.Equals(TEXT("mntearecipes"), ESearchCase::IgnoreCase);
}

UObject* UFVRecipeTemplate_Factory::FactoryCreateFile(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString targetFolder = InParent->GetPathName();

	TArray<UFVRecipeTemplate*> importedRecipes;
	FString errorMessage;

	if (!UFVAdvancedInventoryRecipeEditorStatics::ImportRecipesFromFilePath(
		Filename, targetFolder, importedRecipes, errorMessage))
	{
		if (Warn)
			Warn->Logf(ELogVerbosity::Error, TEXT("%s"), *errorMessage);
		bOutOperationCanceled = true;
		return nullptr;
	}

	return importedRecipes.Num() > 0 ? importedRecipes[0] : nullptr;
}

UObject* UFVRecipeTemplate_Factory::ImportObject(
	UClass* InClass, UObject* InOuter, FName InName, EObjectFlags Flags,
	const FString& Filename, const TCHAR* Parms, bool& OutCanceled)
{
	if (!FactoryCanImport(Filename))
		return nullptr;
	return FactoryCreateFile(InClass, InOuter, InName, Flags, Filename, Parms, nullptr, OutCanceled);
}

