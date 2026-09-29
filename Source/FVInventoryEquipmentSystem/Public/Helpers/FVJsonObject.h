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
#include "Serialization/JsonTypes.h"
#include "UObject/Object.h"
#include "FVJsonObject.generated.h"

class FJsonObject;

UCLASS(ClassGroup=(FV), BlueprintType,
		meta=(DisplayName="FV Json Object"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVJsonObject : public UObject
{
	GENERATED_BODY()

public:

	UFVJsonObject();

	void InitializeJsonObject();
	bool IsJsonObjectValid() const;
	TSharedPtr<FJsonObject> GetSharedJsonObject() const;
	void SetSharedJsonObject(const TSharedPtr<FJsonObject>& NewJsonObject);

	bool SetObjectFieldRaw(FName FieldName, const TSharedPtr<FJsonObject>& Value);
	bool GetObjectFieldRaw(FName FieldName, TSharedPtr<FJsonObject>& Value) const;

	bool SetStringField(FName FieldName, const FString& Value, UFVJsonObject*& JsonObject);
	bool GetStringField(FName FieldName, FString& Value) const;

	bool SetNameField(FName FieldName, const FName& Value, UFVJsonObject*& JsonObject);
	bool GetNameField(FName FieldName, FName& Value) const;

	bool SetTextField(FName FieldName, const FText& Value, UFVJsonObject*& JsonObject);
	bool GetTextField(FName FieldName, FText& Value) const;

	bool SetIntField(FName FieldName, int32 Value, UFVJsonObject*& JsonObject);
	bool GetIntField(FName FieldName, int32& Value) const;

	bool SetFloatField(FName FieldName, float Value, UFVJsonObject*& JsonObject);
	bool GetFloatField(FName FieldName, float& Value) const;

	bool SetBoolField(FName FieldName, bool Value, UFVJsonObject*& JsonObject);
	bool GetBoolField(FName FieldName, bool& Value) const;

	bool SetObjectField(FName FieldName, UFVJsonObject* Value, UFVJsonObject*& JsonObject);
	bool GetObjectField(FName FieldName, UFVJsonObject*& Value) const;

	bool SetNullField(FName FieldName, UFVJsonObject*& JsonObject);
	bool HasField(FName FieldName) const;
	bool RemoveField(FName FieldName, UFVJsonObject*& JsonObject);
	void Clear();

	TArray<FString> GetFieldNames() const;
	EJson GetFieldType(FName FieldName) const;

	bool SetStringArrayField(FName FieldName, const TArray<FString>& Value, UFVJsonObject*& JsonObject);
	bool GetStringArrayField(FName FieldName, TArray<FString>& Value) const;

	bool SetNameArrayField(FName FieldName, const TArray<FName>& Value, UFVJsonObject*& JsonObject);
	bool GetNameArrayField(FName FieldName, TArray<FName>& Value) const;

	bool SetIntArrayField(FName FieldName, const TArray<int32>& Value, UFVJsonObject*& JsonObject);
	bool GetIntArrayField(FName FieldName, TArray<int32>& Value) const;

	bool SetFloatArrayField(FName FieldName, const TArray<float>& Value, UFVJsonObject*& JsonObject);
	bool GetFloatArrayField(FName FieldName, TArray<float>& Value) const;

	bool SetBoolArrayField(FName FieldName, const TArray<bool>& Value, UFVJsonObject*& JsonObject);
	bool GetBoolArrayField(FName FieldName, TArray<bool>& Value) const;

	bool SetObjectArrayField(FName FieldName, const TArray<UFVJsonObject*>& Value, UFVJsonObject*& JsonObject);
	bool GetObjectArrayField(FName FieldName, TArray<UFVJsonObject*>& Value) const;

private:

	TSharedPtr<FJsonObject> JsonObject;
};
