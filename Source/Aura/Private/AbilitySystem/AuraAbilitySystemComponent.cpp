// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, FString::Printf(TEXT("%s"), *GameplayTags.Attributes_Secondary_Armor.ToString()));
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const auto AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag); //DynamicAbiltyTag - ��Ÿ�ӿ� Tag�� �߰� / ���� ����.

			GiveAbility(AbilitySpec);
		}
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

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;

	EffectSpec.GetAllAssetTags(TagContainer); //GameplayEffectSpec�� ����� ��� GameplayEffectAssetTag�� �����ͼ� TagContainer�� Append��.

	EffectAssetTags.Broadcast(TagContainer);
}
