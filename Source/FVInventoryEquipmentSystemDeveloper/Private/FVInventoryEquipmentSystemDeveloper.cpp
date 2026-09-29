#include "FVInventoryEquipmentSystemDeveloper.h"

#if WITH_EDITOR
#include "Factories/FVAdvancedInventoryPinFactory.h"
#include "EdGraphUtilities.h"
#endif

#include "Styling/FVAdvancedInventoryDeveloperStyle.h"

DEFINE_LOG_CATEGORY(FVInventoryEquipmentSystemDeveloper);

#define LOCTEXT_NAMESPACE "FFVInventoryEquipmentSystemDeveloper"

void FFVInventoryEquipmentSystemDeveloper::StartupModule()
{
	UE_LOG(FVInventoryEquipmentSystemDeveloper, Warning, TEXT("FVInventoryEquipmentSystemDeveloper module has been loaded"));

	// Register Style
	{
		FFVAdvancedInventoryDeveloperStyle::Initialize();
	}
	
#if WITH_EDITOR
	// Register Pin Factory
	{
		PinFactory = MakeShared<FFVInventoryPinFactory>();
		FEdGraphUtilities::RegisterVisualPinFactory(PinFactory);
	}
#endif
	
}

void FFVInventoryEquipmentSystemDeveloper::ShutdownModule()
{
	// Style Shutdown
	{
		FFVAdvancedInventoryDeveloperStyle::Shutdown();
	}
	
#if WITH_EDITOR
	// Unregister Pin Factory
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(PinFactory);
		PinFactory.Reset();
	}
#endif
	
	UE_LOG(FVInventoryEquipmentSystemDeveloper, Warning, TEXT("FVInventoryEquipmentSystemDeveloper module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVInventoryEquipmentSystemDeveloper, FVInventoryEquipmentSystemDeveloper)