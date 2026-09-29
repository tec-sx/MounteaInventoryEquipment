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
#include "Interfaces/Attachments/FVAdvancedAttachmentAttachableInterface.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentContainerInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVAttachmentsStatics.generated.h"

enum class EAttachmentSlotState : uint8;
enum class EAttachmentState : uint8;
struct FGameplayTag;
struct FGameplayTagContainer;
class UFVAttachableComponent;
class UFVAdvancedAttachmentSlot;
class IFVAdvancedAttachmentContainerInterface;
class IFVAdvancedAttachmentAttachableInterface;

/**
 * Helper library for attachment containers and attachable objects.
 * Provides Blueprint wrappers around attachment interfaces plus utility queries for
 * components, sockets, and slot/tag-driven attachment operations.
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAttachmentsStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

#pragma region Bindings

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Bind On Attachment Changed")
	static bool BindToOnAttachmentChanged(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentContainerInterface")) UObject* Target, const FFVAttachmentChangedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Unbind From On Attachment Changed")
	static bool UnbindFromOnAttachmentChanged(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentContainerInterface")) UObject* Target, const FFVAttachmentChangedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Bind On Slot State Changed")
	static bool BindToOnSlotStateChanged(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentContainerInterface")) UObject* Target, const FFVSlotStateChangedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Unbind From On Slot State Changed")
	static bool UnbindFromOnSlotStateChanged(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentContainerInterface")) UObject* Target, const FFVSlotStateChangedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Bind On Container Cleared")
	static bool BindToOnContainerCleared(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentContainerInterface")) UObject* Target, const FFVContainerClearedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Unbind From On Container Cleared")
	static bool UnbindFromOnContainerCleared(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentContainerInterface")) UObject* Target, const FFVContainerClearedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Bind On Attachable Attached")
	static bool BindToOnAttachableAttached(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentAttachableInterface")) UObject* Target, const FFVAttachableAttachedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Unbind From On Attachable Attached")
	static bool UnbindFromOnAttachableAttached(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentAttachableInterface")) UObject* Target, const FFVAttachableAttachedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Bind On Attachable Detached")
	static bool BindToOnAttachableDetached(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentAttachableInterface")) UObject* Target, const FFVAttachableDetachedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Unbind From On Attachable Detached")
	static bool UnbindFromOnAttachableDetached(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedAttachmentAttachableInterface")) UObject* Target, const FFVAttachableDetachedBinding& Binding);

#pragma endregion
	
#pragma region Helpers
	
	/**
	 * Returns an array of mesh components available on the target actor.
	 * 
	 * @param Target  Actor to get components from
	 * @return  Array of mesh components available on the target actor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Helpers",
		meta=(FVGetter),
		DisplayName="Get Available Components")
	static TArray<USceneComponent*> GetAvailableComponents(const AActor* Target);

	/**
	 * Returns an array of names of mesh components available on the target actor.
	 * 
	 * @param Target  Actor to get component names from
	 * @return  Array of names of mesh components available on the target actor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Helpers",
		meta=(FVGetter),
		DisplayName="Get Available Components Names")
	static TArray<FName> GetAvailableComponentNames(const AActor* Target);

	/**
	 * Returns first found mesh component available on the target actor by name.
	 * 
	 * @param Target  Actor to get component names from
	 * @param ComponentName Name of component to search for.
	 * @return  Mesh component available on the target actor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Helpers",
		meta=(FVGetter),
		meta=(AutoCreateRefTerm="ComponentName"),
		DisplayName="Get Available Component By Name")
	static USceneComponent* GetAvailableComponentByName(const AActor* Target, const FName& ComponentName);
	
	/**
	 * Returns an array of socket names available on the specified component of the target actor.
	 * 
	 * @param Target  Actor to get socket names from
	 * @param ComponentName  Name of the component to get socket names from
	 * @return  Array of socket names available on the specified component of the target actor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Helpers",
		meta=(FVGetter),
		meta=(AutoCreateRefTerm="ComponentName"),
		DisplayName="Get Available Socket Names")
	static TArray<FName> GetAvailableSocketNames(const AActor* Target, const FName& ComponentName);

	/**
	 * Checks if the specified class is valid to be attachable.
	 * 
	 * @param TargetClass  Target class to validate for being Attachable
	 * @return  True if the class is valid, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Is Valid Attachable Class")
	static bool IsTargetClassValid(const UClass* TargetClass);
	
#pragma endregion 
	
#pragma region AttachmentContainer
	
	/**
	 * Returns an array of socket names available on the target actor.
	 * 
	 * @param Target  Target to get the owning actor from
	 * @return  The owning actor of the target attachment container interface.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|AttachmentContainer",
		meta=(FVGetter),
		DisplayName="Get Owning Actor")
	static AActor* GetOwningActor(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target);

	/**
	 * Returns the default attachment target name.
	 * 
	 * @param Target  Target to get the default attachment target from
	 * @return  The default attachment target name
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVGetter),
		DisplayName="Get Default Attachment Target")
	static FName GetDefaultAttachmentTarget(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target);

	/**
	 * Checks if the specified slot ID is valid.
	 * 
	 * @param Target  Target to validate the slot on
	 * @param SlotId  The slot ID to validate
	 * @return  True if the slot is valid, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVValidate),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Is Valid Slot")
	static bool IsValidSlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId);

	/**
	 * Gets the attachment slot by ID.
	 * 
	 * @param Target  Target to get the slot from
	 * @param SlotId  The slot ID to retrieve
	 * @return  The attachment slot, or null if not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVGetter),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Get Slot")
	static UFVAdvancedAttachmentSlot* GetSlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId);

	/**
	 * Checks if the specified slot is occupied.
	 * 
	 * @param Target  Target to check the slot on
	 * @param SlotId  The slot ID to check
	 * @return  True if the slot is occupied, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVValidate),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Is Slot Occupied")
	static bool IsSlotOccupied(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId);

	/**
	 * Disables the specified slot.
	 * 
	 * @param Target  Target to disable the slot on
	 * @param SlotId  The slot ID to disable
	 * @return  True if the slot was successfully disabled, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVSetter),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Disable Slot")
	static bool DisableSlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId);

	/**
	 * Tries to attach an attachment to the specified slot.
	 * 
	 * @param Target     Target to attach to
	 * @param SlotId     The slot ID to attach to
	 * @param Attachment The attachable object to attach. If implements Attachable interface, it will be used as such.
	 * @return  True if attachment was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVSetter),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Try Attach")
	static bool TryAttach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId, UObject* Attachment);

	/**
	 * Tries to detach from the specified slot.
	 * 
	 * @param Target  Target to detach from
	 * @param SlotId  The slot ID to detach from
	 * @return  True if detachment was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVSetter),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Try Detach")
	static bool TryDetach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId);

	/**
	 * Forces attachment to the specified slot.
	 * 
	 * @param Target     Target to attach to
	 * @param SlotId     The slot ID to attach to
	 * @param Attachment The attachable object to attach. If implements Attachable interface, it will be used as such.
	 * @return  True if attachment was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVSetter),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Force Attach")
	static bool ForceAttach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId, UObject* Attachment);

	/**
	 * Forces detachment from the specified slot.
	 * 
	 * @param Target  Target to detach from
	 * @param SlotId  The slot ID to detach from
	 * @return  True if detachment was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVSetter),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(AutoCreateRefTerm="SlotId"),
		DisplayName="Force Detach")
	static bool ForceDetach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId);

	/**
	 * Finds the first free slot that matches the required tags.
	 * 
	 * @param Target       Target to search for slots on
	 * @param RequiredTags Tags that the slot must match
	 * @return  The slot ID of the first free matching slot, or NAME_None if none found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVGetter),
		meta=(AutoCreateRefTerm="RequiredTags"),
		DisplayName="Find First Free Slot By Tags")
	static FName FindFirstFreeSlotWithTags(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, 
		UPARAM(meta=(Categories="FV_Inventory.AttachmentSlots,FV_Inventory.Category,FV_Inventory.Equipment")) const FGameplayTagContainer& RequiredTags);

	/**
	 * Gets the slot ID for the specified attachable component.
	 * 
	 * @param Target     Target to search for the attachable on
	 * @param Attachable The attachable component to find
	 * @return  The slot ID where the attachable is located, or NAME_None if not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVGetter),
		DisplayName="Find Slot Id For Attachable")
	static FName GetSlotIdForAttachable(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const UFVAttachableComponent* Attachable);

	/**
	 * Clears all attachments from all slots.
	 * 
	 * @param Target  Target to clear all attachments from
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVSetter),
		DisplayName="Clear All")
	static void ClearAll(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target);

	/**
	 * Gets the first empty slot ID.
	 * 
	 * @param Target  Target attachment container.
	 * @return Slot name of the first empty slot, or NAME_None if none is available.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVGetter),
		DisplayName="Find First Empty Slot")
	static FName GetFirstEmptySlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target);

	/**
	 * Gets all slots in the attachment container.
	 * 
	 * @param Target  Target attachment container.
	 * @return Array of attachment slots configured on the container.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|AttachmentContainer",
		meta=(FVGetter),
		DisplayName="Get All Attachment Slots")
	static TArray<UFVAdvancedAttachmentSlot*> GetAttachmentSlots(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target);

#pragma endregion

#pragma region Attachable

	/**
	 * Gets the container this attachable is attached to.
	 * 
	 * @param Target  Target to get the attached container from
	 * @return  The container this attachable is attached to
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVGetter),
		DisplayName="Get Attached To")
	static TScriptInterface<IFVAdvancedAttachmentContainerInterface> GetAttachedTo(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Gets the ID of the attachable.
	 * 
	 * @param Target  Target to get the ID from
	 * @return  The ID of the attachable
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVGetter),
		DisplayName="Get Attachable Id")
	static FName GetId(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Sets the ID of the attachable.
	 * 
	 * @param Target  Target to set the ID on
	 * @param NewId   The new ID to set
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVSetter),
		meta=(AutoCreateRefTerm="NewId"),
		DisplayName="Set Attachable Id")
	static void SetId(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const FName& NewId);

	/**
	 * Gets the display name of the attachable.
	 * 
	 * @param Target  Target to get the display name from
	 * @return  The display name of the attachable
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVGetter),
		DisplayName="Get Attachable Display Name")
	static FText GetDisplayName(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Sets the display name of the attachable.
	 * 
	 * @param Target          Target to set the display name on
	 * @param NewDisplayName  The new display name to set
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVSetter),
		meta=(AutoCreateRefTerm="NewDisplayName"),
		DisplayName="Set Attachable Display Name")
	static void SetDisplayName(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const FText& NewDisplayName);

	/**
	 * Gets the tags of the attachable.
	 * 
	 * @param Target  Target to get the tags from
	 * @return  The tags of the attachable
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVGetter),
		DisplayName="Get Attachable Tags")
	static FGameplayTagContainer GetTags(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Sets the tags of the attachable.
	 * 
	 * @param Target   Target to set the tags on
	 * @param NewTags  The new tags to set
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVSetter),
		meta=(AutoCreateRefTerm="NewTags"),
		DisplayName="Set Attachable Tags")
	static void SetTags(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, 
		UPARAM(meta=(Categories="FV_Inventory.AttachmentSlots,FV_Inventory.Category,FV_Inventory.Equipment")) const FGameplayTagContainer& NewTags);

	/**
	 * Gets the state of the attachable.
	 * 
	 * @param Target  Target to get the state from
	 * @return  The state of the attachable
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVGetter),
		DisplayName="Get Attachable State")
	static EAttachmentState GetState(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Sets the state of the attachable.
	 * 
	 * @param Target    Target to set the state on
	 * @param NewState  The new state to set
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVSetter),
		meta=(AutoCreateRefTerm="NewState"),
		DisplayName="Set Slot State")
	static void SetState(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const EAttachmentState NewState);

	/**
	 * Checks if the attachable is valid.
	 * 
	 * @param Target  Target to validate
	 * @return  True if the attachable is valid, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Is Valid Attachable")
	static bool IsValidAttachable(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Checks if the attachable can attach.
	 * 
	 * @param Target  Target to check
	 * @return  True if the attachable can attach, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Can Attachable Be Attached")
	static bool CanAttach(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Attaches the attachable to a specific slot.
	 * 
	 * @param Target          Target attachable to attach
	 * @param Container       Container to attach to
	 * @param SlotId          The slot ID to attach to
	 * @return  True if attachment was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Attach to Slot")
	static bool AttachToSlot(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Container, const FName& SlotId);

	/**
	 * Attaches the attachable to any available slot in the container.
	 * 
	 * @param Target     Target attachable to attach
	 * @param Container  Container to attach to
	 * @return  True if attachment was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Attach to Container")
	static bool AttachToContainer(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Container);

	/**
	 * Detaches the attachable from its current container.
	 * 
	 * @param Target  Target attachable to detach
	 * @return  True if detachment was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Detach")
	static bool Detach(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target);

	/**
	 * Checks if the attachable has a specific tag.
	 * 
	 * @param Target  Target to check
	 * @param Tag     The tag to check for
	 * @return  True if the attachable has the tag, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(AutoCreateRefTerm="Tag"),
		DisplayName="Has Attachable Tag")
	static bool HasTag(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, 
		UPARAM(meta=(Categories="FV_Inventory.AttachmentSlots,FV_Inventory.Category,FV_Inventory.Equipment")) const FGameplayTag& Tag);

	/**
	 * Checks if the attachable matches the given tags.
	 * 
	 * @param Target       Target to check
	 * @param OtherTags    Tags to match against
	 * @param bRequireAll  Whether all tags must match or just any
	 * @return  True if the tags match, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Attachments|Attachable",
		meta=(FVValidate),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(AutoCreateRefTerm="OtherTags"),
		DisplayName="Has Attachable Tags")
	static bool MatchesTags(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, 
		UPARAM(meta=(Categories="FV_Inventory.AttachmentSlots,FV_Inventory.Category,FV_Inventory.Equipment")) const FGameplayTagContainer& OtherTags, bool bRequireAll);
	
#pragma endregion 
};
