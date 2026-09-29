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
#include "FVAttachableComponent.h"
#include "Interfaces/Equipment/FVAdvancedEquipmentItemInterface.h"
#include "FVEquipmentItemComponent.generated.h"


/**
 * UFVEquipmentItemComponent represents items that can be equipped by actors or components.
 * Equipment components inherits all behaviour from Attachable Component, and extends the behaviour
 * by providing Equipment State logic.
 *
 * @see Equipment Components
 * @see IFVAdvancedEquipmentItemInterface
 * @see UFVAttachableComponent
 */
UCLASS(ClassGroup=(FV), Blueprintable,
	AutoExpandCategories=("FV","Attachable","FV|Attachable","FV|Equipment","Equipment"),
	HideCategories=("Cooking","Collision"),
	meta=(BlueprintSpawnableComponent, DisplayName="FV Equipment Item"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVEquipmentItemComponent : public UFVAttachableComponent, public IFVAdvancedEquipmentItemInterface
{
	GENERATED_BODY()
	
public:
	
	UFVEquipmentItemComponent();
	
public:
	
	virtual EEquipmentItemState GetEquipmentItemState_Implementation() const override
	{ return EquipmentItemState; };
	virtual bool SetEquipmentItemState_Implementation(const EEquipmentItemState NewEquipmentItemState) override;
	virtual bool DoesAutoActive_Implementation() const override
	{ return bAutoActivates; };
	virtual bool SetAutoActive_Implementation(const bool bValue) override;
	virtual bool DoesRequireActivationEvent_Implementation() const override;
	virtual bool SetRequiresActivationEvent_Implementation(const bool bValue) override;
	virtual UAnimMontage* GetActivationAnimation_Implementation() const override;
	virtual bool SetActivationAnimation_Implementation(UAnimMontage* NewActivateAnimation) override;
	virtual bool ShouldReplicateActivationAnimation_Implementation() const override
	{ return bReplicateActivationAnimation; };
	virtual bool DoesUseDeactivationAnimation_Implementation() const override
	{ return bUseDeactivationAnimation; };
	virtual bool SetUseDeactivationAnimation_Implementation(const bool bValue) override;
	virtual UAnimMontage* GetDeactivationAnimation_Implementation() const override;
	virtual bool SetDeactivationAnimation_Implementation(UAnimMontage* NewDeactivateAnimation) override;
	virtual FOnEquipmentItemStateChanged& GetOnEquipmentItemStateChangedHandle() override
	{ return OnEquipmentItemStateChanged; };
	virtual FName GetEquipmentItemPreferredSlot_Implementation() const override
	{ return PreferredSlot; };
	virtual FGameplayTag GetEquipmentPreferredSlotTag_Implementation() const override
	{ return PreferredSlotTag; };
	virtual FGuid GetEquippedItemId_Implementation() const override
	{ return EquippedItemId; };
	virtual void SetEquippedItemId_Implementation(const FGuid& NewEquippedItemId) override;
	
protected:
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(EditCondition="!bAutoActivates && bRequiresActivationEvent"),
		meta=(DisplayPriority=0))
	TSoftObjectPtr<UAnimMontage> ActivationAnimation;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(EditCondition="!bAutoActivates && bRequiresActivationEvent && bUseDeactivationAnimation"),
		meta=(DisplayPriority=1))
	TSoftObjectPtr<UAnimMontage> DeactivationAnimation;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(GetOptions="GetAvailableSlots"),
		meta=(DisplayPriority=2))
	FName PreferredSlot = NAME_None;
	
	// Defined the Equipped Item Id from Inventory
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		Replicated, 
		meta=(NoResetToDefault),
		meta=(DisplayPriority=4))
	FGuid EquippedItemId;
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(Categories="FV_Inventory.AttachmentSlots,Slot,Attachment"),
		meta=(DisplayPriority=3))
	FGameplayTag PreferredSlotTag;
	
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		ReplicatedUsing="OnRep_EquipmentItemState",
		meta=(NoResetToDefault),
		meta=(DisplayPriority=1))
	EEquipmentItemState EquipmentItemState;

	// If Item does activate when equipped (eg: Rings), like those Items which require no animation to activate.
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(DisplayPriority=5))
	uint8 bAutoActivates : 1;

	/**
	 * Defines whether this Item requires an Event to be activated.
	 * 
	 * Example: 
	 * The equipped weapon (sword) is sheathed. To activate the Equipment Item should play some animation.
	 * That means this weapon will set this value to YES.
	 * 
	 * Editable only if Auto Activates is FALSE.
	 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(EditCondition="!bAutoActivates"),
		meta=(DisplayPriority=6))
	uint8 bRequiresActivationEvent : 1;

	/**
	 * If true, activation animation will be multicast to other clients.
	 * Reserved for future use (v2).
	 *
	 * Editable only if Auto Activates is FALSE and Requires Activation Event is TRUE.
	 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(EditCondition="!bAutoActivates && bRequiresActivationEvent"),
		meta=(DisplayPriority=7))
	uint8 bReplicateActivationAnimation : 1;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "FV|Equipment",
		meta=(NoResetToDefault),
		meta=(EditCondition="!bAutoActivates && bRequiresActivationEvent"),
		meta=(DisplayPriority=8))
	uint8 bUseDeactivationAnimation : 1;

	/**
	 * Even Triggered when Equipment Item changes.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="FV|Equipment",
		meta=(IsBindableEvent=true),
		meta=(NoResetToDefault))
	FOnEquipmentItemStateChanged OnEquipmentItemStateChanged;
	
public:
	
	UFUNCTION()
	static TArray<FName> GetAvailableSlots();
	
protected:
	
	UFUNCTION()
	void OnRep_EquipmentItemState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
