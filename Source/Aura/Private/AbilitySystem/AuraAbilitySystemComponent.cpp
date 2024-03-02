// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Aura/Public/AuraLogChannel.h"
#include "Interaction/PlayerInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Data/AbilityInfo.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const auto AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag); //DynamicAbiltyTag - ��Ÿ�ӿ� Tag�� �߰� / ���� ����.

			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);

			GiveAbility(AbilitySpec);
		}
	}
	bStartUpAbilitiesGiven = true;

	AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const auto PassiveAbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(PassiveAbilityClass, 1);

		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (auto& AbilitySpec : GetActivatableAbilities()) //Ȱ��ȭ �� �� �ִ� Ability�� ��� ������.
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) // InputTag�� ������ �ִ� Ability���� Ȯ��.
		{
			AbilitySpecInputPressed(AbilitySpec);

			if (!AbilitySpec.IsActive()) //��Ȱ�� Ability���
			{
				TryActivateAbility(AbilitySpec.Handle); //Ȱ��ȭ ��Ŵ.
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (auto& AbilitySpec : GetActivatableAbilities()) //Ȱ��ȭ �� �� �ִ� Ability�� ��� ������.
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) // InputTag�� ������ �ִ� Ability���� Ȯ��.
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// �ɷ��� �ݺ��ϴ� ���� �ɷ� �ý��� ���� ��ҿ��� �ɷ��� �����ϴ� ���� �����ϴ� �� ���˴ϴ�.
	FScopedAbilityListLock ActiveScopeLock(*this);

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(AuraLog, Error, TEXT("Failed to Execute Delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (const FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			// Abilities ���� �±׸� ��� Ž��.
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		// InputTag �±׿� �ش��ϴ� �±����� Ȯ��.
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}

	return FGameplayTag();
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoint(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddAttributePointsReward(GetAvatarActor(), -1);
	}
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	/** Used to stop us from removing abilities from an ability system component while we're iterating through the abilities */
	FScopedAbilityListLock ActiveScopeLoc(*this);

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// �ش� Ability�� �����ִ� Tag
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}

	// �ش��ϴ� Ability�� ������ nullptr ��ȯ.
	return nullptr;
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	if (UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor()))
	{
		for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformations)
		{
			if (!Info.AbilityTag.IsValid()) continue;
			if (Info.LevelRequirement > Level) continue;

			if (Info.LevelRequirement <= Level)
			{
				// ���� ASC�� ��ϵ��� ���� Ability���
				if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
				{
					FGameplayAbilitySpec AbilitySpec(Info.Ability, 1);
					//���� ������ �޼������Ƿ� Eligible Status�� ����.
					AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
					GiveAbility(AbilitySpec);

					/** Call to mark that an ability spec has been modified */
					// ���� ������Ʈ���� ��ٸ��� �ʰ� AbilitySpec�� �ٷ� Replicate��.
					MarkAbilitySpecDirty(AbilitySpec);

					ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, AbilitySpec.Level);
				}
			}
		}
	}
}

void UAuraAbilitySystemComponent::ServerSpendPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		// Spell Point�� �Һ��ϹǷ� Spell Point 1 ����.
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddSpellPointsReward(GetAvatarActor(), -1);
		}

		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);

		// Eligble�� SpellPoint�� �Ἥ UnLocked ���·� ����.
		if (Status.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Eligible))
		{
			AbilitySpec->DynamicAbilityTags.RemoveTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_UnLocked);
			Status = FAuraGameplayTags::Get().Abilities_Status_UnLocked;
		}
		// UnLocked�� Equipped�� AbilityLevel�� ������.
		else if (Status.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped) || Status.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_UnLocked))
		{
			AbilitySpec->Level += 1;
		}

		// AbilityStatus�� ����� ���� Broadcast
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		// ���� ������Ʈ���� ��ٸ��� �ʰ� AbilitySpec�� �ٷ� Replicate��.
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartUpAbilitiesGiven)
	{
		bStartUpAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;

	EffectSpec.GetAllAssetTags(TagContainer); //GameplayEffectSpec�� ����� ��� GameplayEffectAssetTag�� �����ͼ� TagContainer�� Append��.

	EffectAssetTags.Broadcast(TagContainer);
}
