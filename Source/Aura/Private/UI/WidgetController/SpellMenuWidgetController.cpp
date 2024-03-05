#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"
#include "AuraGameplayTags.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	OnSpellPointChanged.Broadcast(GetAuraPS()->GetSpellPoint());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraASC()->AbilityStatusChanged.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
		{
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
			{
				SelectedAbility.Status = StatusTag;
			
				bool bEnableSpendPointButton = false;
				bool bEnableEquippedButton = false;

				ShouldEnableButton(StatusTag, CurSpellPoint, bEnableSpendPointButton, bEnableEquippedButton);

				FString Description, NextLevelDescription;
				GetAuraASC()->GetDescriptionByAbilityTag(AbilityTag, Description, NextLevelDescription);

				SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointButton, bEnableEquippedButton, Description, NextLevelDescription);
			}			
			
			if (AbilityInfo)
			{
				FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		}
	);

	GetAuraPS()->OnSpellPointChangedDelegate.AddLambda(
		[this](int32 SpellPoints)
		{
			OnSpellPointChanged.Broadcast(SpellPoints);
			CurSpellPoint = SpellPoints;

			bool bEnableSpendPointButton = false;
			bool bEnableEquippedButton = false;

			ShouldEnableButton(SelectedAbility.Status, CurSpellPoint, bEnableSpendPointButton, bEnableEquippedButton);

			FString Description, NextLevelDescription;
			GetAuraASC()->GetDescriptionByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);

			SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointButton, bEnableEquippedButton, Description, NextLevelDescription);
		}
	);
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	// Selection Anim이 재생중이라면
	if (bWaitingForEquipSelection)
	{
		// 클릭했었던 AbilityType Broadcast
		FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitForEquipSelectionDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}

	const int32 SpellPoint = GetAuraPS()->GetSpellPoint();
	FGameplayTag StatusTag;

	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(FAuraGameplayTags::Get().Abilities_None);
	FGameplayAbilitySpec* AbilitySpec = GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	
	if(!bTagValid || bTagNone || !bSpecValid) 
	{
		StatusTag = FAuraGameplayTags::Get().Abilities_Status_Locked;
	}
	else
	{
		StatusTag = GetAuraASC()->GetStatusFromSpec(*AbilitySpec);
	}

	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = StatusTag;

	bool bEnableSpendPointButton = false;
	bool bEnableEquippedButton = false;
	
	// 함수 내에 있는 지역변수를 매개변수로 넣어서 변경할 것이므로 bool&로 받음
	ShouldEnableButton(StatusTag, SpellPoint, bEnableSpendPointButton, bEnableEquippedButton);

	FString Description, NextLevelDescription;
	GetAuraASC()->GetDescriptionByAbilityTag(AbilityTag, Description, NextLevelDescription);

	SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointButton, bEnableEquippedButton, Description, NextLevelDescription);
}

void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (GetAuraASC())
	{
		GetAuraASC()->ServerSpendPoint(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::GlobeDeselect()
{
	// Selection Anim이 재생중이라면
	if (bWaitingForEquipSelection)
	{
		// 클릭했었던 AbilityType Broadcast
		FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitForEquipSelectionDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}

	SelectedAbility.Ability = FAuraGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FAuraGameplayTags::Get().Abilities_Status_Locked;

	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	WaitForEquipSelectionDelegate.Broadcast(AbilityType);

	bWaitingForEquipSelection = true;
}

void USpellMenuWidgetController::ShouldEnableButton(const FGameplayTag& StatusTag, int32 SpellPoint, bool& bShouldEnableSpellPointButton, bool& bShouldEnableEquippedButton)
{
	bShouldEnableEquippedButton = false;
	bShouldEnableSpellPointButton = false;

	if (StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		bShouldEnableEquippedButton = true;
		if (SpellPoint > 0)
		{
			bShouldEnableSpellPointButton = true;
		}
	}
	else if (StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_UnLocked))
	{
		bShouldEnableEquippedButton = true;
		if (SpellPoint > 0)
		{
			bShouldEnableSpellPointButton = true;
		}
	}
	else if (StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Eligible))
	{
		if (SpellPoint > 0)
		{
			bShouldEnableSpellPointButton = true;
		}
	}

}
