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

#include "Setup/FVInventorySetupUtilities.h"

#include "Definitions/FVAdvancedInventoryLoadoutConfig.h"
#include "Components/FVAdvancedInventoryLoadoutComponent.h"
#include "Components/FVCraftingParticipantComponent.h"
#include "Components/FVCraftingParticipantUIComponent.h"
#include "Components/FVEquipmentComponent.h"
#include "Components/FVInventoryComponent.h"
#include "Components/FVInventoryUIComponent.h"
#include "Components/ActorComponent.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"
#include "GameMapsSettings.h"
#include "InputMappingContext.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Settings/FVAdvancedCraftingConfig.h"
#include "Settings/FVAdvancedCraftingUIConfig.h"
#include "Settings/FVAdvancedEquipmentSettingsConfig.h"
#include "Settings/FVAdvancedInventoryGlobalConfig.h"
#include "Settings/FVAdvancedInventoryGlobalUIConfig.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Settings/FVAdvancedInventorySettingsConfig.h"
#include "Settings/FVAdvancedInventoryUIConfig.h"

namespace FVInventorySetupDefaults
{
	static constexpr const TCHAR* inputMappingPath =
		TEXT("/FVInventoryEquipmentSystem/Input/IMC_Inventory.IMC_Inventory");
	static constexpr const TCHAR* inventorySettingsConfigPath =
		TEXT("/FVInventoryEquipmentSystem/Data/Config/DefaultInventoryConfig.DefaultInventoryConfig");
	static constexpr const TCHAR* equipmentSettingsConfigPath =
		TEXT("/FVInventoryEquipmentSystem/Data/Config/DefaultEquipmentConfig.DefaultEquipmentConfig");
	static constexpr const TCHAR* inventoryUISettingsConfigPath =
		TEXT("/FVInventoryEquipmentSystem/Data/Config/DefaultInventoryUIConfig.DefaultInventoryUIConfig");
	static constexpr const TCHAR* globalUIConfigPath =
		TEXT("/FVInventoryEquipmentSystem/Data/Config/DefaultGlobalUIConfig.DefaultGlobalUIConfig");
	static constexpr const TCHAR* globalConfigPath =
		TEXT("/FVInventoryEquipmentSystem/Data/Config/DefaultFVGlobalConfig.DefaultFVGlobalConfig");
	static constexpr const TCHAR* craftingSettingsConfigPath =
		TEXT("/FVInventoryEquipmentSystem/Data/Config/DefaultFVCraftingConfig.DefaultFVCraftingConfig");
	static constexpr const TCHAR* craftingUISettingsConfigPath =
		TEXT("/FVInventoryEquipmentSystem/Data/Config/DefaultCraftingUISettingsConfig.DefaultCraftingUISettingsConfig");

	template <typename TObjectType>
	bool TryAssignMissing(
		TSoftObjectPtr<TObjectType>& TargetProperty,
		const TCHAR* ObjectPath,
		const TCHAR* PropertyName,
		FMAISSetupDefaultsReport& Report
	)
	{
		if (!TargetProperty.ToSoftObjectPath().IsNull())
		{
			Report.SettingsResults.Add(FMAISSetupItemResult::Make(
				EMAISSetupActorRole::Settings,
				EMAISSetupItemStatus::AlreadyPresent,
				PropertyName,
				FString::Printf(TEXT("Already assigned to %s."), *TargetProperty.ToSoftObjectPath().ToString())
			));
			return false;
		}

		const FSoftObjectPath softObjectPath(ObjectPath);
		UObject* resolvedObject = softObjectPath.ResolveObject();
		if (!resolvedObject)
			resolvedObject = softObjectPath.TryLoad();

		if (!resolvedObject)
		{
			Report.SettingsResults.Add(FMAISSetupItemResult::Make(
				EMAISSetupActorRole::Settings,
				EMAISSetupItemStatus::Failed,
				PropertyName,
				FString::Printf(TEXT("Missing default asset: %s."), ObjectPath)
			));
			LOG_WARNING(TEXT("[Setup Defaults] Missing default asset for '%s': %s"), PropertyName, ObjectPath);
			return false;
		}

		if (!Cast<TObjectType>(resolvedObject))
		{
			Report.SettingsResults.Add(FMAISSetupItemResult::Make(
				EMAISSetupActorRole::Settings,
				EMAISSetupItemStatus::Failed,
				PropertyName,
				FString::Printf(TEXT("Default asset has incompatible type: %s."), ObjectPath)
			));
			LOG_WARNING(TEXT("[Setup Defaults] Asset type mismatch for '%s': %s"), PropertyName, ObjectPath);
			return false;
		}

		TargetProperty = TSoftObjectPtr<TObjectType>(softObjectPath);
		Report.SettingsResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Settings,
			EMAISSetupItemStatus::Added,
			PropertyName,
			FString::Printf(TEXT("Assigned %s."), ObjectPath)
		));
		LOG_INFO(TEXT("[Setup Defaults] Applied default '%s' from %s"), PropertyName, ObjectPath);
		return true;
	}

	bool BlueprintHasComponent(const UBlueprintGeneratedClass* BlueprintClass, const TSubclassOf<UActorComponent> ComponentClass)
	{
		if (!BlueprintClass || !ComponentClass)
			return false;

		const UClass* currentClass = BlueprintClass;
		while (currentClass)
		{
			const UBlueprintGeneratedClass* blueprintClass = Cast<UBlueprintGeneratedClass>(currentClass);
			if (blueprintClass && blueprintClass->SimpleConstructionScript)
			{
				for (const USCS_Node* node : blueprintClass->SimpleConstructionScript->GetAllNodes())
				{
					if (node && node->ComponentClass && node->ComponentClass->IsChildOf(ComponentClass))
						return true;
				}
			}

			currentClass = currentClass->GetSuperClass();
		}

		return false;
	}

	FName MakeUniqueComponentNodeName(UBlueprint* Blueprint, const UClass* ComponentClass)
	{
		if (!Blueprint || !ComponentClass)
			return NAME_None;

		FString baseName = ComponentClass->GetName();
		baseName.RemoveFromEnd(TEXT("Component"));

		FName candidateName(*baseName);
		if (!Blueprint->SimpleConstructionScript)
			return candidateName;

		int32 suffix = 1;
		bool bNameExists = true;
		while (bNameExists)
		{
			bNameExists = false;
			for (const USCS_Node* node : Blueprint->SimpleConstructionScript->GetAllNodes())
			{
				if (node && node->GetVariableName() == candidateName)
				{
					bNameExists = true;
					candidateName = FName(*FString::Printf(TEXT("%s_%d"), *baseName, suffix++));
					break;
				}
			}
		}

		return candidateName;
	}

	void AddComponentResult(
		FMAISSetupDefaultsReport& Report,
		const EMAISSetupItemStatus Status,
		const UClass* ComponentClass,
		const FString& Message
	)
	{
		Report.PlayerResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Player,
			Status,
			ComponentClass ? ComponentClass->GetName() : TEXT("Component"),
			Message
		));
	}

	bool AddComponentToBlueprint(
		UBlueprint* Blueprint,
		const TSubclassOf<UActorComponent> ComponentClass,
		FMAISSetupDefaultsReport& Report
	)
	{
		if (!Blueprint || !ComponentClass)
		{
			AddComponentResult(Report, EMAISSetupItemStatus::Failed, ComponentClass, TEXT("Blueprint or component class is invalid."));
			return false;
		}

		if (!Blueprint->SimpleConstructionScript)
		{
			AddComponentResult(Report, EMAISSetupItemStatus::Failed, ComponentClass, TEXT("Blueprint has no Simple Construction Script."));
			return false;
		}

		const FName nodeName = MakeUniqueComponentNodeName(Blueprint, ComponentClass);
		USCS_Node* newNode = Blueprint->SimpleConstructionScript->CreateNode(ComponentClass, nodeName);
		if (!newNode)
		{
			AddComponentResult(Report, EMAISSetupItemStatus::Failed, ComponentClass, TEXT("Failed to create component node."));
			return false;
		}

		Blueprint->SimpleConstructionScript->AddNode(newNode);
		AddComponentResult(
			Report,
			EMAISSetupItemStatus::Added,
			ComponentClass,
			FString::Printf(TEXT("Added to %s."), *Blueprint->GetName())
		);
		return true;
	}
}

FMAISSetupItemResult FMAISSetupItemResult::Make(
	const EMAISSetupActorRole Role,
	const EMAISSetupItemStatus Status,
	const FString& ItemName,
	const FString& Message
)
{
	FMAISSetupItemResult result;
	result.Role = Role;
	result.Status = Status;
	result.ItemName = ItemName;
	result.Message = Message;
	return result;
}

bool FMAISSetupDefaultsReport::HasFailures() const
{
	const auto isFailure = [](const FMAISSetupItemResult& Result)
	{
		return Result.Status == EMAISSetupItemStatus::Failed;
	};

	return SettingsResults.ContainsByPredicate(isFailure) || PlayerResults.ContainsByPredicate(isFailure);
}

FMAISSetupDefaultsReport FFVInventorySetupUtilities::RunSetupDefaults()
{
	FMAISSetupDefaultsReport report;
	ApplySettingsDefaults(report);
	SetupPlayerComponents(report);
	return report;
}

FMAISSetupDefaultsReport FFVInventorySetupUtilities::ApplySettingsDefaults()
{
	FMAISSetupDefaultsReport report;
	ApplySettingsDefaults(report);
	return report;
}

void FFVInventorySetupUtilities::ApplySettingsDefaults(FMAISSetupDefaultsReport& Report)
{
	using namespace FVInventorySetupDefaults;

	UFVAdvancedInventorySettings* settings = GetMutableDefault<UFVAdvancedInventorySettings>();
	if (!settings)
	{
		Report.SettingsResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Settings,
			EMAISSetupItemStatus::Failed,
			TEXT("FV Advanced Inventory Settings"),
			TEXT("Failed to access inventory settings.")
		));
		LOG_ERROR(TEXT("[Setup Defaults] Failed to access inventory settings."));
		return;
	}

	bool bAnyChange = false;
	bAnyChange |= TryAssignMissing(settings->AdvancedInventoryEquipmentInputMapping, inputMappingPath, TEXT("AdvancedInventoryEquipmentInputMapping"), Report);
	bAnyChange |= TryAssignMissing(settings->AdvancedInventorySettingsConfig, inventorySettingsConfigPath, TEXT("AdvancedInventorySettingsConfig"), Report);
	bAnyChange |= TryAssignMissing(settings->AdvancedEquipmentSettingsConfig, equipmentSettingsConfigPath, TEXT("AdvancedEquipmentSettingsConfig"), Report);
	bAnyChange |= TryAssignMissing(settings->AdvancedInventoryUISettingsConfig, inventoryUISettingsConfigPath, TEXT("AdvancedInventoryUISettingsConfig"), Report);
	bAnyChange |= TryAssignMissing(settings->GlobalUIConfig, globalUIConfigPath, TEXT("GlobalUIConfig"), Report);
	bAnyChange |= TryAssignMissing(settings->GlobalConfig, globalConfigPath, TEXT("GlobalConfig"), Report);
	bAnyChange |= TryAssignMissing(settings->AdvancedCraftingSettingsConfig, craftingSettingsConfigPath, TEXT("AdvancedCraftingSettingsConfig"), Report);
	bAnyChange |= TryAssignMissing(settings->AdvancedCraftingUISettingsConfig, craftingUISettingsConfigPath, TEXT("AdvancedCraftingUISettingsConfig"), Report);

	if (bAnyChange)
	{
		settings->SaveConfig();
		Report.bSettingsSaved = true;
		LOG_INFO(TEXT("[Setup Defaults] Saved updated inventory defaults to config."));
	}
}

void FFVInventorySetupUtilities::SetupPlayerComponents(FMAISSetupDefaultsReport& Report)
{
	using namespace FVInventorySetupDefaults;

	UWorld* editorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!editorWorld)
	{
		Report.PlayerResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Player,
			EMAISSetupItemStatus::Failed,
			TEXT("Editor World"),
			TEXT("Unable to resolve the editor world.")
		));
		return;
	}

	TSubclassOf<AGameModeBase> gameModeClass;
	if (const AWorldSettings* worldSettings = editorWorld->GetWorldSettings())
		gameModeClass = worldSettings->DefaultGameMode;

	if (!gameModeClass)
	{
		Report.bIsDefaultGameMode = true;
		const FString defaultGameModePath = UGameMapsSettings::GetGlobalDefaultGameMode();
		if (!defaultGameModePath.IsEmpty())
			gameModeClass = LoadClass<AGameModeBase>(nullptr, *defaultGameModePath);
	}

	if (!gameModeClass)
	{
		Report.PlayerResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Player,
			EMAISSetupItemStatus::Failed,
			TEXT("Game Mode"),
			TEXT("No world or project default GameMode is configured.")
		));
		return;
	}

	Report.GameModeName = gameModeClass->GetName();

	const AGameModeBase* gameModeDefaultObject = Cast<AGameModeBase>(gameModeClass->GetDefaultObject());
	if (!gameModeDefaultObject)
	{
		Report.PlayerResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Player,
			EMAISSetupItemStatus::Failed,
			TEXT("Game Mode"),
			FString::Printf(TEXT("Unable to read default object for %s."), *gameModeClass->GetName())
		));
		return;
	}

	TSubclassOf<APawn> playerClass = gameModeDefaultObject->DefaultPawnClass;
	if (!playerClass)
	{
		Report.PlayerResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Player,
			EMAISSetupItemStatus::Failed,
			TEXT("Default Pawn"),
			FString::Printf(TEXT("%s has no DefaultPawnClass configured."), *gameModeClass->GetName())
		));
		return;
	}

	Report.PlayerClassName = playerClass->GetName();

	UBlueprintGeneratedClass* blueprintClass = Cast<UBlueprintGeneratedClass>(playerClass.Get());
	if (!blueprintClass)
	{
		Report.PlayerResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Player,
			EMAISSetupItemStatus::NativeClass,
			Report.PlayerClassName,
			TEXT("Default pawn is a native C++ class. Add the required components manually or use a Blueprint pawn.")
		));
		return;
	}

	UBlueprint* blueprint = Cast<UBlueprint>(blueprintClass->ClassGeneratedBy);
	if (!blueprint)
	{
		Report.PlayerResults.Add(FMAISSetupItemResult::Make(
			EMAISSetupActorRole::Player,
			EMAISSetupItemStatus::Failed,
			Report.PlayerClassName,
			TEXT("Default pawn class is generated, but its Blueprint asset could not be resolved.")
		));
		return;
	}

	const TArray<TSubclassOf<UActorComponent>> requiredComponents =
	{
		UFVInventoryComponent::StaticClass(),
		UFVInventoryUIComponent::StaticClass(),
		UFVEquipmentComponent::StaticClass(),
		UFVCraftingParticipantComponent::StaticClass(),
		UFVCraftingParticipantUIComponent::StaticClass(),
		UFVAdvancedInventoryLoadoutComponent::StaticClass()
	};

	bool bBlueprintModified = false;
	for (const TSubclassOf<UActorComponent>& componentClass : requiredComponents)
	{
		if (BlueprintHasComponent(blueprintClass, componentClass))
		{
			AddComponentResult(
				Report,
				EMAISSetupItemStatus::AlreadyPresent,
				componentClass,
				FString::Printf(TEXT("%s already has this component."), *Report.PlayerClassName)
			);
			continue;
		}

		bBlueprintModified |= AddComponentToBlueprint(blueprint, componentClass, Report);
	}

	if (bBlueprintModified)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);
		FKismetEditorUtilities::CompileBlueprint(blueprint);
	}
}
