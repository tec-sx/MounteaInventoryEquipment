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

#include "Settings/FVAdvancedInventoryGlobalConfig.h"

#include "Decorations/FVSelectableInventoryItemAction.h"
#include "Definitions/FVAdvancedInventoryGlobalUIDataTypes.h"

static FFVJsonObjectDefinitionField MakeJsonDefinitionField(const FName FieldName, const FName PinCategory, UObject* SubCategoryObject = nullptr)
{
	FFVJsonObjectDefinitionField field;
	field.FieldName = FieldName;
	field.FieldValueType.PinCategory = PinCategory;
	field.FieldValueType.PinSubCategoryObject = SubCategoryObject;

	return field;
}

static FFVJsonObjectDefinitionField MakeJsonIntDefinitionField(const FName FieldName)
{
	return MakeJsonDefinitionField(FieldName, TEXT("int"));
}

static FFVJsonObjectDefinitionField MakeJsonFloatDefinitionField(const FName FieldName)
{
	return MakeJsonDefinitionField(FieldName, TEXT("real"));
}

static FFVJsonObjectDefinitionField MakeJsonStringDefinitionField(const FName FieldName)
{
	return MakeJsonDefinitionField(FieldName, TEXT("string"));
}

static FFVJsonObjectDefinitionField MakeJsonBooleanDefinitionField(const FName FieldName)
{
	return MakeJsonDefinitionField(FieldName, TEXT("bool"));
}

static FFVJsonObjectDefinitionField MakeJsonStructDefinitionField(const FName FieldName, UScriptStruct* StructType)
{
	return MakeJsonDefinitionField(FieldName, TEXT("struct"), StructType);
}

static FFVJsonObjectDefinitionField MakeJsonUObjectDefinitionField(const FName FieldName, UClass* ObjectClass = nullptr)
{
	return MakeJsonDefinitionField(FieldName, TEXT("object"), ObjectClass ? ObjectClass : UObject::StaticClass());
}

static FFVJsonObjectDefinitionField MakeJsonSoftObjectPtrDefinitionField(const FName FieldName, UClass* ObjectClass = nullptr)
{
	return MakeJsonDefinitionField(FieldName, TEXT("softobject"), ObjectClass ? ObjectClass : UObject::StaticClass());
}

static FFVJsonObjectDefinitionInclude MakeJsonDefinitionInclude(const FString& DefinitionKey)
{
	FFVJsonObjectDefinitionInclude include;
	include.DefinitionKey = DefinitionKey;

	return include;
}

static FFVJsonObjectDefinition MakeJsonObjectDefinition(
	TArray<FFVJsonObjectDefinitionField> Fields,
	TArray<FFVJsonObjectDefinitionInclude> IncludedDefinitions = {})
{
	FFVJsonObjectDefinition definition;
	definition.Fields = MoveTemp(Fields);
	definition.IncludedDefinitions = MoveTemp(IncludedDefinitions);

	return definition;
}

#if WITH_EDITOR

void UFVAdvancedInventoryGlobalConfig::SetupDefaults()
{
	// ModalBaseDefinition
	JsonObjectDefinitions.Add(TEXT("ModalBaseDefinition"), MakeJsonObjectDefinition(
		{
			MakeJsonStructDefinitionField(TEXT("modalData"), TBaseStructure<FFVModalsConfig>::Get()),
			MakeJsonStringDefinitionField(TEXT("modalType"))
		}
	));
	
	// ModalItemDefinition
	JsonObjectDefinitions.Add(TEXT("ModalItemDefinition"), MakeJsonObjectDefinition(
		{
			MakeJsonStructDefinitionField(TEXT("itemGuid"), TBaseStructure<FGuid>::Get()),
		}
	));

	// ModalSliderDefinition
	JsonObjectDefinitions.Add(TEXT("ModalSliderDefinition"), MakeJsonObjectDefinition(
		{
			MakeJsonStructDefinitionField(TEXT("sliderRange"), TBaseStructure<FIntPoint>::Get())
		}
	));

	// ModalItemQuantityDefinition
	JsonObjectDefinitions.Add(TEXT("ModalItemQuantityDefinition"), MakeJsonObjectDefinition(
		{
			MakeJsonSoftObjectPtrDefinitionField(TEXT("inventoryAction"), UFVSelectableInventoryItemAction::StaticClass())
		},
		{
			MakeJsonDefinitionInclude(TEXT("ModalBaseDefinition")),
			MakeJsonDefinitionInclude(TEXT("ModalItemDefinition")),
			MakeJsonDefinitionInclude(TEXT("ModalSliderDefinition"))
		}
	));

	
	// ModalResponseBaseDefinition
	JsonObjectDefinitions.Add(TEXT("ModalResponseBaseDefinition"), MakeJsonObjectDefinition(
		{
			MakeJsonStringDefinitionField(TEXT("modalType")),
			MakeJsonBooleanDefinitionField(TEXT("confirmed"))
		}
	));

	// ModalItemResponseDefinition
	JsonObjectDefinitions.Add(TEXT("ModalItemResponseDefinition"), MakeJsonObjectDefinition(
		{
			MakeJsonStructDefinitionField(TEXT("itemGuid"), TBaseStructure<FGuid>::Get())
		},
		{
			MakeJsonDefinitionInclude(TEXT("ModalResponseBaseDefinition"))
		}
	));

	// ModalQuantityResponseDefinition
	JsonObjectDefinitions.Add(TEXT("ModalQuantityResponseDefinition"), MakeJsonObjectDefinition(
		{
			MakeJsonIntDefinitionField(TEXT("quantity"))
		},
		{
			MakeJsonDefinitionInclude(TEXT("ModalItemResponseDefinition"))
		}
	));
}

#endif
