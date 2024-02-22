

#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/Data/LevelUpInfo.h"

void UOverlayWidgetController::BroadcastInitialValues()
{

	const UAuraAttributeSet* AuraAttributSet = CastChecked<UAuraAttributeSet>(AttributeSet); 
	
	//CastCheck�� ��ȿ���� ������ �ߴ��ϹǷ� ���� if���� �� �ʿ�X.
	OnHealthChanged.Broadcast(AuraAttributSet->GetHealth()); // ACCESSOR
	OnMaxHealthChanged.Broadcast(AuraAttributSet->GetMaxHealth()); // ACCESSOR

	OnManaChanged.Broadcast(AuraAttributSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies() // Attribute�� ����Ǿ�����.
{
	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	AuraPlayerState->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	
	// DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/);
	AuraPlayerState->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
		}
	);

	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	//FOnGameplayAttributeValueChange Delegate�� DYNAMIC�� �ƴ϶� AddUObject�� ���.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetHealthAttribute()).AddLambda(//Health�� ����� ������ HealthChanged�� ȣ���.
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
			);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetMaxHealthAttribute()).AddLambda( //MaxHealth�� ����� ������ MaxHealthChanged�� ȣ���.
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
			);


		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
			);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
			);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		if (AuraASC->bStartUpAbilitiesGiven) // Startup Ability�� �ο��� ���
		{
			OnInitializeStartUpAbilities(AuraASC);
		}
		else // �ο����� ���� ��쿡�� Bind�� ��Ŵ.
		{
			AuraASC->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartUpAbilities);
		}

		AuraASC->EffectAssetTags.AddLambda( //AddLamda - �Ｎ���� �Լ��� ����.
			[this](const FGameplayTagContainer& AssetTags /*�Ű�����*/)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					//"A.1".MatchesTag("A") will return true, but "A".MatchesTag("A.1") will return false
					//EX) "Message.HealthPotion".MatchesTag("Message") will return true, but "Message".MatchesTag("Message.HealthPotion") will return false
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));

					//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, FString::Printf(TEXT("%s"), *Tag.ToString())); //TagName�� ǥ��.

					if (Tag.MatchesTag(MessageTag)) // MessageTag�� ��쿡�� MessageWidgetTable���� Ž��.
					{
						//Lamba�Լ��� �����Լ��� �ٷ� ����� �� ������, ����Լ��� []�ȿ� �ش� ����Լ��� Ŭ���� ĸó�� �ؾ� ��� ����.(this)
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);


	}

}

void UOverlayWidgetController::OnInitializeStartUpAbilities(UAuraAbilitySystemComponent* AuraAbilitySystemComponent)
{
	if (!AuraAbilitySystemComponent->bStartUpAbilitiesGiven) return;

	FForEachAbility BroadCastDelegate;
	BroadCastDelegate.BindLambda(
		[this, AuraAbilitySystemComponent](const FGameplayAbilitySpec& AbilitySpec)
		{
			FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AuraAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
			Info.InputTag = AuraAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(Info);
		}
	);

	AuraAbilitySystemComponent->ForEachAbility(BroadCastDelegate);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP) const
{
	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	const ULevelUpInfo* LevelUpInfo = AuraPlayerState->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Can't find the LevelUpInfo, Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level > 0 && Level <= MaxLevel)
	{
		const int32 LevelRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreLevelRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
		
		const int32 DifLevelRequirement = LevelRequirement - PreLevelRequirement;
		const int32 CurXP = NewXP - PreLevelRequirement;

		// 1���� 300 2���� 900 NewXP = 700. CurXP = 400. XPPercent = 400 / 600  
		const float XPBarPercent = static_cast<float>(CurXP) / static_cast<float>(DifLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}

}

