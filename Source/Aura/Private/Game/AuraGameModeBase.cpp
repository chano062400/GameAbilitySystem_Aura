#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Game/AuraGameInstance.h"

void AAuraGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	DeleteSlot(LoadSlot->LoadSlotName, SlotIndex);
	
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->SaveSlotStatus = ESaveSlotStatus::Taken;
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;

	// SaveGame을 Index에 해당하는 Slot에 저장.
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->LoadSlotName, SlotIndex);
}

void AAuraGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	// index에 해당하는 Slot에 이미 SaveGame이 있다면 지움.
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

ULoadScreenSaveGame* AAuraGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}

	return Cast<ULoadScreenSaveGame>(SaveGameObject);
}

void AAuraGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	Maps.FindChecked(Slot->GetMapName());

	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps[Slot->GetMapName()]);
}

AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	return nullptr;
}

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	Maps.Add(DefaultMapName, DefaultMap);
}
