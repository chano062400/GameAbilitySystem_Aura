

#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AuraGameplayTags.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	//CastCheck는 유효하지 않으면 중단하므로 따로 if문을 쓸 필요X.
	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth()); // ACCESSOR
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth()); // ACCESSOR

	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies() // Attribute가 변경되었을때.
{
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	
	// DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/);
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);

	//FOnGameplayAttributeValueChange Delegate가 DYNAMIC이 아니라서 AddUObject를 사용.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetHealthAttribute()).AddLambda(//Health가 변경될 때마다 HealthChanged가 호출됨.
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
			);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetMaxHealthAttribute()).AddLambda( //MaxHealth가 변경될 때마다 MaxHealthChanged가 호출됨.
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
			);


		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
			);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
			);

	if (GetAuraASC())
	{
		GetAuraASC()->AbilityEquippedDelegate.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);

		if (GetAuraASC()->bStartUpAbilitiesGiven) // Startup Ability가 부여된 경우
		{
			BroadcastAbilityInfo();
		}
		else // 부여되지 않은 경우에는 Bind만 시킴.
		{
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}

		GetAuraASC()->EffectAssetTags.AddLambda( //AddLamda - 즉석에서 함수를 정의.
			[this](const FGameplayTagContainer& AssetTags /*매개변수*/)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					//"A.1".MatchesTag("A") will return true, but "A".MatchesTag("A.1") will return false
					//EX) "Message.HealthPotion".MatchesTag("Message") will return true, but "Message".MatchesTag("Message.HealthPotion") will return false
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));

					//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, FString::Printf(TEXT("%s"), *Tag.ToString())); //TagName을 표시.

					if (Tag.MatchesTag(MessageTag)) // MessageTag인 경우에만 MessageWidgetTable에서 탐색.
					{
						//Lamba함수는 전역함수는 바로 사용할 수 있지만, 멤버함수는 []안에 해당 멤버함수의 클래스 캡처를 해야 사용 가능.(this)
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);


	}

}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Can't find the LevelUpInfo, Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level > 0 && Level <= MaxLevel)
	{
		const int32 LevelRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreLevelRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
		
		const int32 DifLevelRequirement = LevelRequirement - PreLevelRequirement;
		const int32 CurXP = NewXP - PreLevelRequirement;

		// 1레벨 300 2레벨 900 NewXP = 700. CurXP = 400. XPPercent = 400 / 600  
		const float XPBarPercent = static_cast<float>(CurXP) / static_cast<float>(DifLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}

}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FAuraGameplayTags::Get().Abilities_Status_UnLocked;
	LastSlotInfo.InputTag = PrevSlot;
	LastSlotInfo.AbilityTag = FAuraGameplayTags::Get().Abilities_None;

	// 이전에 이미 Ability가 장착돼있던 Slot에 Ability를 장착하려는 경우에만 빈 AbilityInfo를 Broadcast함.
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// 장착하려는 Ability.
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;

	AbilityInfoDelegate.Broadcast(Info);
}

