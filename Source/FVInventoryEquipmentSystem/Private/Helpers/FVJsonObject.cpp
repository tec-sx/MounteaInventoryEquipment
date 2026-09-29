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

#include "Helpers/FVJsonObject.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

static UObject* ResolveJsonObjectOuter(UObject* Target)
{
	return IsValid(Target) ? Target : GetTransientPackage();
}

static TSharedPtr<FJsonValue> GetFieldValue(const TSharedPtr<FJsonObject>& JsonObject, const FName FieldName)
{
	return JsonObject.IsValid() ? JsonObject->TryGetField(FieldName.ToString()) : nullptr;
}

static UFVJsonObject* CreateJsonObjectWrapper(UObject* Target, const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject.IsValid())
		return nullptr;

	UFVJsonObject* wrapper = NewObject<UFVJsonObject>(ResolveJsonObjectOuter(Target));
	wrapper->SetSharedJsonObject(JsonObject);
	return wrapper;
}

UFVJsonObject::UFVJsonObject()
{
	InitializeJsonObject();
}

void UFVJsonObject::InitializeJsonObject()
{
	JsonObject = MakeShared<FJsonObject>();
}

bool UFVJsonObject::IsJsonObjectValid() const
{
	return JsonObject.IsValid();
}

TSharedPtr<FJsonObject> UFVJsonObject::GetSharedJsonObject() const
{
	return JsonObject;
}

void UFVJsonObject::SetSharedJsonObject(const TSharedPtr<FJsonObject>& NewJsonObject)
{
	JsonObject = NewJsonObject.IsValid() ? NewJsonObject : MakeShared<FJsonObject>();
}

bool UFVJsonObject::SetObjectFieldRaw(const FName FieldName, const TSharedPtr<FJsonObject>& Value)
{
	if (!JsonObject.IsValid() || FieldName.IsNone() || !Value.IsValid())
		return false;

	JsonObject->SetObjectField(FieldName.ToString(), Value);
	return true;
}

bool UFVJsonObject::GetObjectFieldRaw(const FName FieldName, TSharedPtr<FJsonObject>& Value) const
{
	Value.Reset();

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Object)
		return false;

	Value = fieldValue->AsObject();
	return Value.IsValid();
}

bool UFVJsonObject::SetStringField(const FName FieldName, const FString& Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	JsonObject->SetStringField(FieldName.ToString(), Value);
	return true;
}

bool UFVJsonObject::GetStringField(const FName FieldName, FString& Value) const
{
	Value.Reset();

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::String)
		return false;

	Value = fieldValue->AsString();
	return true;
}

bool UFVJsonObject::SetNameField(const FName FieldName, const FName& Value, UFVJsonObject*& OutJsonObject)
{
	return SetStringField(FieldName, Value.ToString(), OutJsonObject);
}

bool UFVJsonObject::GetNameField(const FName FieldName, FName& Value) const
{
	FString stringValue;
	if (!GetStringField(FieldName, stringValue))
	{
		Value = NAME_None;
		return false;
	}

	Value = FName(*stringValue);
	return true;
}

bool UFVJsonObject::SetTextField(const FName FieldName, const FText& Value, UFVJsonObject*& OutJsonObject)
{
	return SetStringField(FieldName, Value.ToString(), OutJsonObject);
}

bool UFVJsonObject::GetTextField(const FName FieldName, FText& Value) const
{
	FString stringValue;
	if (!GetStringField(FieldName, stringValue))
	{
		Value = FText::GetEmpty();
		return false;
	}

	Value = FText::FromString(stringValue);
	return true;
}

bool UFVJsonObject::SetIntField(const FName FieldName, const int32 Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	JsonObject->SetNumberField(FieldName.ToString(), Value);
	return true;
}

bool UFVJsonObject::GetIntField(const FName FieldName, int32& Value) const
{
	Value = 0;

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Number)
		return false;

	const double numberValue = fieldValue->AsNumber();
	if (numberValue < static_cast<double>(MIN_int32) || numberValue > static_cast<double>(MAX_int32))
		return false;

	Value = static_cast<int32>(numberValue);
	return true;
}

bool UFVJsonObject::SetFloatField(const FName FieldName, const float Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	JsonObject->SetNumberField(FieldName.ToString(), Value);
	return true;
}

bool UFVJsonObject::GetFloatField(const FName FieldName, float& Value) const
{
	Value = 0.f;

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Number)
		return false;

	Value = static_cast<float>(fieldValue->AsNumber());
	return true;
}

bool UFVJsonObject::SetBoolField(const FName FieldName, const bool Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	JsonObject->SetBoolField(FieldName.ToString(), Value);
	return true;
}

bool UFVJsonObject::GetBoolField(const FName FieldName, bool& Value) const
{
	Value = false;

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Boolean)
		return false;

	Value = fieldValue->AsBool();
	return true;
}

bool UFVJsonObject::SetObjectField(const FName FieldName, UFVJsonObject* Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;
	return IsValid(Value) ? SetObjectFieldRaw(FieldName, Value->GetSharedJsonObject()) : false;
}

bool UFVJsonObject::GetObjectField(const FName FieldName, UFVJsonObject*& Value) const
{
	Value = nullptr;

	TSharedPtr<FJsonObject> objectValue;
	if (!GetObjectFieldRaw(FieldName, objectValue))
		return false;

	Value = CreateJsonObjectWrapper(const_cast<UFVJsonObject*>(this), objectValue);
	return IsValid(Value);
}

bool UFVJsonObject::SetNullField(const FName FieldName, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	JsonObject->SetField(FieldName.ToString(), MakeShared<FJsonValueNull>());
	return true;
}

bool UFVJsonObject::HasField(const FName FieldName) const
{
	return GetFieldValue(JsonObject, FieldName).IsValid();
}

bool UFVJsonObject::RemoveField(const FName FieldName, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone() || !HasField(FieldName))
		return false;

	JsonObject->RemoveField(FieldName.ToString());
	return true;
}

void UFVJsonObject::Clear()
{
	InitializeJsonObject();
}

TArray<FString> UFVJsonObject::GetFieldNames() const
{
	TArray<FString> returnValue;

	if (!JsonObject.IsValid())
		return returnValue;

	returnValue.Reserve(JsonObject->Values.Num());

	for (const auto& keyValuePair : JsonObject->Values)
		returnValue.Add(FString(keyValuePair.Key));

	return returnValue;
}

EJson UFVJsonObject::GetFieldType(const FName FieldName) const
{
	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	return fieldValue.IsValid() ? fieldValue->Type : EJson::None;
}

bool UFVJsonObject::SetStringArrayField(const FName FieldName, const TArray<FString>& Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	TArray<TSharedPtr<FJsonValue>> arrayValues;
	for (const FString& item : Value)
		arrayValues.Add(MakeShared<FJsonValueString>(item));

	JsonObject->SetArrayField(FieldName.ToString(), arrayValues);
	return true;
}

bool UFVJsonObject::GetStringArrayField(const FName FieldName, TArray<FString>& Value) const
{
	Value.Reset();

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Array)
		return false;

	for (const TSharedPtr<FJsonValue>& item : fieldValue->AsArray())
	{
		if (!item.IsValid() || item->Type != EJson::String)
			return false;

		Value.Add(item->AsString());
	}

	return true;
}

bool UFVJsonObject::SetNameArrayField(const FName FieldName, const TArray<FName>& Value, UFVJsonObject*& OutJsonObject)
{
	TArray<FString> stringValues;
	for (const FName& item : Value)
		stringValues.Add(item.ToString());

	return SetStringArrayField(FieldName, stringValues, OutJsonObject);
}

bool UFVJsonObject::GetNameArrayField(const FName FieldName, TArray<FName>& Value) const
{
	Value.Reset();

	TArray<FString> stringValues;
	if (!GetStringArrayField(FieldName, stringValues))
		return false;

	for (const FString& item : stringValues)
		Value.Add(FName(*item));

	return true;
}

bool UFVJsonObject::SetIntArrayField(const FName FieldName, const TArray<int32>& Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	TArray<TSharedPtr<FJsonValue>> arrayValues;
	for (const int32 item : Value)
		arrayValues.Add(MakeShared<FJsonValueNumber>(item));

	JsonObject->SetArrayField(FieldName.ToString(), arrayValues);
	return true;
}

bool UFVJsonObject::GetIntArrayField(const FName FieldName, TArray<int32>& Value) const
{
	Value.Reset();

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Array)
		return false;

	for (const TSharedPtr<FJsonValue>& item : fieldValue->AsArray())
	{
		if (!item.IsValid() || item->Type != EJson::Number)
			return false;

		const double numberValue = item->AsNumber();
		if (numberValue < static_cast<double>(MIN_int32) || numberValue > static_cast<double>(MAX_int32))
			return false;

		Value.Add(static_cast<int32>(numberValue));
	}

	return true;
}

bool UFVJsonObject::SetFloatArrayField(const FName FieldName, const TArray<float>& Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	TArray<TSharedPtr<FJsonValue>> arrayValues;
	for (const float item : Value)
		arrayValues.Add(MakeShared<FJsonValueNumber>(item));

	JsonObject->SetArrayField(FieldName.ToString(), arrayValues);
	return true;
}

bool UFVJsonObject::GetFloatArrayField(const FName FieldName, TArray<float>& Value) const
{
	Value.Reset();

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Array)
		return false;

	for (const TSharedPtr<FJsonValue>& item : fieldValue->AsArray())
	{
		if (!item.IsValid() || item->Type != EJson::Number)
			return false;

		Value.Add(static_cast<float>(item->AsNumber()));
	}

	return true;
}

bool UFVJsonObject::SetBoolArrayField(const FName FieldName, const TArray<bool>& Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	TArray<TSharedPtr<FJsonValue>> arrayValues;
	for (const bool item : Value)
		arrayValues.Add(MakeShared<FJsonValueBoolean>(item));

	JsonObject->SetArrayField(FieldName.ToString(), arrayValues);
	return true;
}

bool UFVJsonObject::GetBoolArrayField(const FName FieldName, TArray<bool>& Value) const
{
	Value.Reset();

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Array)
		return false;

	for (const TSharedPtr<FJsonValue>& item : fieldValue->AsArray())
	{
		if (!item.IsValid() || item->Type != EJson::Boolean)
			return false;

		Value.Add(item->AsBool());
	}

	return true;
}

bool UFVJsonObject::SetObjectArrayField(const FName FieldName, const TArray<UFVJsonObject*>& Value, UFVJsonObject*& OutJsonObject)
{
	OutJsonObject = this;

	if (!JsonObject.IsValid() || FieldName.IsNone())
		return false;

	TArray<TSharedPtr<FJsonValue>> arrayValues;
	for (UFVJsonObject* item : Value)
	{
		if (!IsValid(item) || !item->GetSharedJsonObject().IsValid())
			return false;

		arrayValues.Add(MakeShared<FJsonValueObject>(item->GetSharedJsonObject()));
	}

	JsonObject->SetArrayField(FieldName.ToString(), arrayValues);
	return true;
}

bool UFVJsonObject::GetObjectArrayField(const FName FieldName, TArray<UFVJsonObject*>& Value) const
{
	Value.Reset();

	const TSharedPtr<FJsonValue> fieldValue = GetFieldValue(JsonObject, FieldName);
	if (!fieldValue.IsValid() || fieldValue->Type != EJson::Array)
		return false;

	for (const TSharedPtr<FJsonValue>& item : fieldValue->AsArray())
	{
		if (!item.IsValid() || item->Type != EJson::Object)
			return false;

		UFVJsonObject* wrapper = CreateJsonObjectWrapper(const_cast<UFVJsonObject*>(this), item->AsObject());
		if (!IsValid(wrapper))
			return false;

		Value.Add(wrapper);
	}

	return true;
}
