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

	GetAuraASC()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

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
	// Selection Anim�� ������̶��
	if (bWaitingForEquipSelection)
	{
		// Ŭ���߾��� AbilityType Broadcast
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
	
	// �Լ� ���� �ִ� ���������� �Ű������� �־ ������ ���̹Ƿ� bool&�� ����
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
	// Selection Anim�� ������̶��
	if (bWaitingForEquipSelection)
	{
		// Ŭ���߾��� AbilityType Broadcast
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
	// Offensive or Passive
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	WaitForEquipSelectionDelegate.Broadcast(AbilityType);

	bWaitingForEquipSelection = true;

	// ������ Spell Globe�� Status
	const FGameplayTag SelectedStatus = GetAuraASC()->GetStautusFromAbilityTag(SelectedAbility.Ability);
	if (SelectedStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		// ������ Spell Globe�� InputTag.
		SelectedSlot = GetAuraASC()->GetInputTagFromAbilityTag(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::EquippedSpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	// Equip��ư�� ������ �ʾ��� ���.
	if (!bWaitingForEquipSelection) return;

	// ������ Spell�� AbilityType(Offensive or Passive)
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	
	// EquippedSpellRow���� ������ AbilityType�� ������ Spell�� AbilityType�� �ٸ��� - Ex) Offensive Spell�� �����ϰ�, Passive Spell�� �����Ϸ���.
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;
	
	GetAuraASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	bWaitingForEquipSelection = false;

	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FAuraGameplayTags::Get().Abilities_Status_UnLocked;
	LastSlotInfo.InputTag = PrevSlot;
	LastSlotInfo.AbilityTag = FAuraGameplayTags::Get().Abilities_None;

	// ������ �̹� Ability�� �������ִ� Slot�� Ability�� �����Ϸ��� ��쿡�� �� AbilityInfo�� Broadcast��.
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// �����Ϸ��� Ability.
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;

	AbilityInfoDelegate.Broadcast(Info);

	// Selection �ִϸ��̼� ����.
	StopWaitForEquipSelectionDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SpellGlobeReassigned.Broadcast(AbilityTag);
	GlobeDeselect();
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
