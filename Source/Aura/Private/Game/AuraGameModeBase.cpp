#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Game/AuraGameInstance.h"
#include "EngineUtils.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Interaction/SaveInterface.h"

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

void AAuraGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveGameObject)
{
	if (UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance()))
	{
		const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
		const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

		AuraGameInstance->PlayerStartTag = SaveGameObject->PlayerStartTag;
		UGameplayStatics::SaveGameToSlot(SaveGameObject, InGameLoadSlotName, InGameLoadSlotIndex);
	}
}

void AAuraGameModeBase::SaveWorldState(UWorld* World) const
{
	// 현재 레벨의 이름만을 가져옴.
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	if (UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance()))
	{
		if (ULoadScreenSaveGame* SaveGameObject = GetSaveSlotData(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex))
		{
			if (!SaveGameObject->HasMap(WorldName))
			{
				FSavedMap NewSavedMap;
				NewSavedMap.MapAssetName = WorldName;
				SaveGameObject->SavedMap.Add(NewSavedMap);
			}

			FSavedMap SavedMap = SaveGameObject->GetSavedMapWithMapName(WorldName);
			SavedMap.SavedActors.Empty(); // 지우고, 새롭게 다시 저장.


			/**
			 * Actor iterator
			 * Note that when Playing In Editor, this will find actors only in CurrentWorld
			 */

			// SavedActor 저장.
			for (FActorIterator It(World); It; ++It)
			{
				AActor* Actor = *It;

				if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

				FSavedActor SavedActor;
				SavedActor.ActorName = Actor->GetFName();
				SavedActor.Transform = Actor->GetTransform();

				/**
				* Archive for storing arbitrary data to the specified memory location
				*/
				FMemoryWriter MemoryWriter(SavedActor.Bytes);

				// Implements a proxy archive that serializes UObjects and FNames as string data.
				FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
				Archive.ArIsSaveGame = true;

				Actor->Serialize(Archive);

				SavedMap.SavedActors.AddUnique(SavedActor);
			}

			// Map 교체.
			for (FSavedMap& MapToReplace : SaveGameObject->SavedMap)
			{
				if (MapToReplace.MapAssetName == WorldName)
				{
					MapToReplace = SavedMap;
				}
			}

			UGameplayStatics::SaveGameToSlot(SaveGameObject, AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex);
		}
	}

}

void AAuraGameModeBase::LoadWorldState(UWorld* World) const
{
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	if (UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance()))
	{
		ULoadScreenSaveGame* SaveGameObject = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex));
		if (SaveGameObject == nullptr) return;

		if (UGameplayStatics::DoesSaveGameExist(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex))
		{
			for (FActorIterator It(World); It; ++It)
			{
				AActor* Actor = *It;

				if (!Actor->Implements<USaveInterface>()) continue;

				for (FSavedActor SavedActor : SaveGameObject->GetSavedMapWithMapName(WorldName).SavedActors)
				{
					if (SavedActor.ActorName == Actor->GetFName())
					{
						if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
						{
							Actor->SetActorTransform(SavedActor.Transform);

						}

						FMemoryWriter MemoryReader(SavedActor.Bytes);

						FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
						Archive.ArIsSaveGame = true;
						// DeSerialize - Converts Binary Bytes Back Into Variables. 
						Actor->Serialize(Archive);

						ISaveInterface::Execute_LoadActor(Actor);
					}
				}
				
			}
		}
	}
}

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	Maps.Add(DefaultMapName, DefaultMap);
}
