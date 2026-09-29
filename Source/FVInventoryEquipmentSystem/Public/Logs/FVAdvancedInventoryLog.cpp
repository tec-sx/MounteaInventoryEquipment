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

#include "FVAdvancedInventoryLog.h"

#include "FVInventoryEquipmentSystemLogTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/FVAdvancedInventorySettings.h"

// Log category definition
DEFINE_LOG_CATEGORY(LogFVInventoryEquipmentSystem);

void PrintLog(const ELogVerbosity::Type Verbosity, const FString& Message, FLinearColor Color, float Duration)
{
	if (!GWorld) return;

	bool isVerbosityAllowed = false;
	
	if (const UFVAdvancedInventorySettings* inventorySettings = GetDefault<UFVAdvancedInventorySettings>())
	{
		const EFVAdvancedInventoryLoggingVerbosity AllowedLogging = inventorySettings->GetAllowedLoggVerbosity();
		
		switch (Verbosity)
		{
		case ELogVerbosity::Fatal:
		case ELogVerbosity::Error:
			isVerbosityAllowed = EnumHasAnyFlags(AllowedLogging, EFVAdvancedInventoryLoggingVerbosity::Error);
			break;
		case ELogVerbosity::Warning:
		case ELogVerbosity::Verbose:
			isVerbosityAllowed = EnumHasAnyFlags(AllowedLogging, EFVAdvancedInventoryLoggingVerbosity::Warning);
			break;
		case ELogVerbosity::Display:
			isVerbosityAllowed = EnumHasAnyFlags(AllowedLogging, EFVAdvancedInventoryLoggingVerbosity::Info);
			break;
		case ELogVerbosity::VeryVerbose:
			isVerbosityAllowed = false;
			break;
		case ELogVerbosity::Log:
			isVerbosityAllowed = false;
			break;
		default:
			isVerbosityAllowed = false;
			break;
		}
	}
	
	if (isVerbosityAllowed)
	{
#if WITH_EDITOR
		FMsg::Logf(__FILE__, __LINE__, LogFVInventoryEquipmentSystem.GetCategoryName(), Verbosity, TEXT("%s"), *Message);
#endif
		
		UKismetSystemLibrary::PrintString(GWorld, Message, true, true, Color, Duration);
	}
}