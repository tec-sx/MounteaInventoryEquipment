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
#include "Settings/FVAdvancedInventoryGlobalConfig.h"
#include "FVAdvancedInventoryJsonStatics.generated.h"

class UFVAdvancedInventoryGlobalConfig;
class UFVJsonObject;

struct FVINVENTORYEQUIPMENTSYSTEM_API FFVResolvedJsonObjectDefinitionField
{
	FFVJsonObjectDefinitionField Field;
	FString SourceDefinitionKey;
	TArray<FString> IncludePath;
	int32 OccurrenceIndex = 1;
	int32 OccurrenceCount = 1;

	bool IsDuplicate() const
	{
		return OccurrenceCount > 1;
	}
};

UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryJsonStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|JSON",
		meta=(FVGetter),
		meta=(CallableWithoutWorldContext),
		meta=(DefaultToSelf="Target"),
		DisplayName="Create Json Object")
	static UFVJsonObject* CreateJsonObject(UObject* Target);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|JSON|Serialization",
		meta=(FVGetter),
		meta=(CallableWithoutWorldContext),
		meta=(DefaultToSelf="Target"),
		DisplayName="Parse Json Object From String")
	static UFVJsonObject* ParseJsonObjectFromString(UObject* Target, const FString& JsonString);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|JSON|Serialization",
		meta=(FVGetter),
		DisplayName="Json Object To String")
	static FString JsonObjectToString(UFVJsonObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|JSON|Serialization",
		meta=(FVGetter),
		DisplayName="Json Object To Pretty String")
	static FString JsonObjectToPrettyString(UFVJsonObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|JSON",
		meta=(FVValidate),
		DisplayName="Is Valid Json Object")
	static bool IsValidJsonObject(UFVJsonObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|JSON|Definitions",
		meta=(FVGetter),
		DisplayName="Get Global Json Config")
	static UFVAdvancedInventoryGlobalConfig* GetGlobalJsonConfig();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|JSON|Definitions",
		meta=(FVGetter),
		meta=(FVJsonDefinition),
		meta=(FVJsonDefinitionPin="DefinitionKey"),
		DisplayName="Find Json Object Definition")
	static bool FindJsonObjectDefinition(const FString& DefinitionKey, FFVJsonObjectDefinition& Definition);

	static bool ResolveJsonObjectDefinitionByKey(const FString& DefinitionKey, FFVJsonObjectDefinition& OutDefinition, TArray<FString>& Errors);
	static bool ResolveJsonObjectDefinitionFieldsByKey(const FString& DefinitionKey, TArray<FFVResolvedJsonObjectDefinitionField>& OutFields, TArray<FString>& Errors);
	static FString JoinJsonDefinitionIncludePath(const TArray<FString>& IncludePath);
	static FName MakeJsonDefinitionFieldPinName(const FString& FieldPinPrefix, FName FieldName, int32 OccurrenceIndex, TSet<FName>& UsedPinNames);
	static FString BuildJsonDefinitionFieldPinTooltip(const FFVResolvedJsonObjectDefinitionField& FieldData);
	static FString BuildJsonDefinitionIssueText(const FString& DefinitionKey);
	static bool IsSupportedJsonDefinitionFieldPinType(const FEdGraphPinType& PinType);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|JSON|Definitions",
		meta=(FVGetter),
		meta=(CallableWithoutWorldContext),
		meta=(DefaultToSelf="Target"),
		DisplayName="Create Json Object From Definition")
	static UFVJsonObject* CreateJsonObjectFromDefinition(UObject* Target, const FFVJsonObjectDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|JSON|Definitions",
		meta=(FVGetter),
		meta=(FVJsonDefinition),
		meta=(FVJsonDefinitionPin="DefinitionKey"),
		meta=(CallableWithoutWorldContext),
		meta=(DefaultToSelf="Target"),
		DisplayName="Create Json Object From Definition Key")
	static UFVJsonObject* CreateJsonObjectFromDefinitionKey(UObject* Target, const FString& DefinitionKey);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|JSON|Definitions",
		meta=(FVValidate),
		DisplayName="Validate Json Object Against Definition")
	static bool ValidateJsonObjectAgainstDefinition(UFVJsonObject* Target, const FFVJsonObjectDefinition& Definition, TArray<FString>& Errors);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|JSON|Definitions",
		meta=(FVValidate),
		meta=(FVJsonDefinition),
		meta=(FVJsonDefinitionPin="DefinitionKey"),
		DisplayName="Validate Json Object Against Definition Key")
	static bool ValidateJsonObjectAgainstDefinitionKey(UFVJsonObject* Target, const FString& DefinitionKey, TArray<FString>& Errors);

#pragma region K2NodeHelpers

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetIntJsonField(UFVJsonObject* Target, FName FieldName, int32 Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetInt64JsonField(UFVJsonObject* Target, FName FieldName, int64 Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetFloatJsonField(UFVJsonObject* Target, FName FieldName, float Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetStringJsonField(UFVJsonObject* Target, FName FieldName, const FString& Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetNameJsonField(UFVJsonObject* Target, FName FieldName, const FName& Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetTextJsonField(UFVJsonObject* Target, FName FieldName, const FText& Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetBoolJsonField(UFVJsonObject* Target, FName FieldName, bool Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetByteJsonField(UFVJsonObject* Target, FName FieldName, uint8 Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetObjectJsonField(UFVJsonObject* Target, FName FieldName, UObject* Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetSoftObjectJsonField(UFVJsonObject* Target, FName FieldName, const TSoftObjectPtr<UObject>& Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetClassJsonField(UFVJsonObject* Target, FName FieldName, TSubclassOf<UObject> Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool SetSoftClassJsonField(UFVJsonObject* Target, FName FieldName, const TSoftClassPtr<UObject>& Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields", CustomThunk,
		meta=(CustomStructureParam="Value"),
		meta=(DefaultToSelf="Target"))
	static bool SetStructJsonField(UFVJsonObject* Target, FName FieldName, const int32& Value, UFVJsonObject*& JsonObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetIntJsonField(UFVJsonObject* Target, FName FieldName, int32& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetInt64JsonField(UFVJsonObject* Target, FName FieldName, int64& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetFloatJsonField(UFVJsonObject* Target, FName FieldName, float& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetStringJsonField(UFVJsonObject* Target, FName FieldName, FString& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetNameJsonField(UFVJsonObject* Target, FName FieldName, FName& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetTextJsonField(UFVJsonObject* Target, FName FieldName, FText& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetBoolJsonField(UFVJsonObject* Target, FName FieldName, bool& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetByteJsonField(UFVJsonObject* Target, FName FieldName, uint8& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetObjectJsonField(UFVJsonObject* Target, FName FieldName, UObject*& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetSoftObjectJsonField(UFVJsonObject* Target, FName FieldName, TSoftObjectPtr<UObject>& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetClassJsonField(UFVJsonObject* Target, FName FieldName, TSubclassOf<UObject>& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields",
		meta=(DefaultToSelf="Target"))
	static bool GetSoftClassJsonField(UFVJsonObject* Target, FName FieldName, TSoftClassPtr<UObject>& Value);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category="FV|Inventory & Equipment|JSON|Fields", CustomThunk,
		meta=(CustomStructureParam="Value"),
		meta=(DefaultToSelf="Target"))
	static bool GetStructJsonField(UFVJsonObject* Target, FName FieldName, int32& Value);

	DECLARE_FUNCTION(execSetStructJsonField);
	DECLARE_FUNCTION(execGetStructJsonField);

#pragma endregion
};
