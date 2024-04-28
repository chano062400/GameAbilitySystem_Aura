#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	SetWidgetSwitcherIndex.Broadcast(SaveSlotStatus.GetValue());
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	// Property�� �����ϰ� Broadcast��.
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}
