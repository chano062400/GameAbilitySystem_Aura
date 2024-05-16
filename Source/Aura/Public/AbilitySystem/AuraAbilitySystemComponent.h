#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);

DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);

DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, int32 /*AbilityLevel*/);

DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*Status*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*Ability Tag*/);

DECLARE_MULTICAST_DELEGATE_TwoParams(FActivatePassiveEffect, const FGameplayTag& /*Ability Tag*/, bool /*bAcitvate*/);

class ULoadScreenSaveGame;

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	
	void AbilityActorInfoSet(); // AbilityActorInfo가 Set됐을 때 호출.

	FEffectAssetTags EffectAssetTags;

	FAbilitiesGiven AbilitiesGivenDelegate;

	FAbilityStatusChanged AbilityStatusChanged;

	FAbilityEquipped AbilityEquippedDelegate;

	FDeactivatePassiveAbility DeactivatePassiveAbility;

	FActivatePassiveEffect ActivatePassiveEffect;

	bool bStartUpAbilitiesGiven = false;

	void AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveGameObject);

	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	void ForEachAbility(const FForEachAbility& Delegate);

	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	FGameplayTag GetSlotFromAbilityTag(const FGameplayTag& AbilityTag);

	FGameplayTag GetStautusFromAbilityTag(const FGameplayTag& AbilityTag);

	FGameplayTag GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag);

	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);

	bool SlotIsEmpty(const FGameplayTag& Slot);

	static bool AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

	static bool AbilityHasAnySlot(FGameplayAbilitySpec* Spec);

	FGameplayAbilitySpec* GetSpecWithSlot(const FGameplayTag& Slot);

	bool IsPassiveAbility(const FGameplayAbilitySpec& Spec);

	static void AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

	// 해당 Ability의 InputTag 제거
	static void ClearSlot(FGameplayAbilitySpec* Spec);

	// 해당 InputTag를 가진 모든 Ability의 InputTag 제거.
	void ClearAbilitiesOfSlot(const FGameplayTag& Slot);

	// Ability가 장착돼있는지 반환.
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);

	UFUNCTION(NetMulticast, UnReliable)
	void MulticastActivatePassiveEffect(const FGameplayTag& AbilityTag, bool bActivate);

	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

	void UpdateAbilityStatuses(int32 Level);

	UFUNCTION(Server, Reliable)
	void ServerSpendPoint(const FGameplayTag& AbilityTag);

	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);

	UFUNCTION(Client, Reliable)
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot);

	bool GetDescriptionByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);

protected:

	virtual void OnRep_ActivateAbilities() override;

	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);
};
