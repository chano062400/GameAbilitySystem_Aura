#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	SetWidgetSwitcherIndex.Broadcast(2);
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	// Property를 설정하고 Broadcast함.
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}
