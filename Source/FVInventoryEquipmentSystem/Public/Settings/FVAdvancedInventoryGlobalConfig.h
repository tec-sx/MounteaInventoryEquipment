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
#include "EdGraph/EdGraphPin.h"
#include "Engine/DataAsset.h"
#include "FVAdvancedInventoryGlobalConfig.generated.h"


USTRUCT(BlueprintType)
struct FVINVENTORYEQUIPMENTSYSTEM_API FFVJsonObjectDefinitionField
{
	GENERATED_BODY()

public:

	FFVJsonObjectDefinitionField()
	{
		FieldValueType.PinCategory = TEXT("string");
		FieldValueType.ContainerType = EPinContainerType::None;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON Definition",
		meta=(NoResetToDefault))
	FName FieldName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON Definition",
		meta=(NoResetToDefault))
	FEdGraphPinType FieldValueType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON Definition",
		meta=(NoResetToDefault))
	bool bRequired = true;

public:

	bool IsValid() const
	{
		return !FieldName.IsNone();
	}
};

USTRUCT(BlueprintType)
struct FVINVENTORYEQUIPMENTSYSTEM_API FFVJsonObjectDefinitionInclude
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON Definition",
		meta=(NoResetToDefault))
	FString DefinitionKey;

public:

	bool IsValid() const
	{
		return !DefinitionKey.IsEmpty();
	}
};

USTRUCT(BlueprintType)
struct FVINVENTORYEQUIPMENTSYSTEM_API FFVJsonObjectDefinition
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON Definition",
		meta=(NoResetToDefault),
		meta=(TitleProperty="{DefinitionKey}"))
	TArray<FFVJsonObjectDefinitionInclude> IncludedDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON Definition",
		meta=(NoResetToDefault),
		meta=(TitleProperty="{FieldName}"))
	TArray<FFVJsonObjectDefinitionField> Fields;

public:

	bool IsValid() const
	{
		return Fields.Num() > 0;
	}
};

UCLASS(ClassGroup=(FV), BlueprintType, Blueprintable, DisplayName="Global Config",
	meta=(ShortTooltip="Global configuration asset for FV Inventory & Equipment shared definitions."))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryGlobalConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
#if WITH_EDITOR
	
	UFUNCTION(CallInEditor, Category="Defaults")
	void SetupDefaults();
	
#endif
	

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="JSON Definitions",
		meta=(ForceInlineRow, NoResetToDefault))
	TMap<FString, FFVJsonObjectDefinition> JsonObjectDefinitions;
};
