#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/CombatInterface.h"
#include "Components/TimelineComponent.h"
#include "AuraCharacterBase.generated.h"

class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:

	AAuraCharacterBase();

	virtual void HighlightActor();
	
	virtual void UnHighlightActor();

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	virtual UAnimMontage* GetHitReactMontage_Implementation() override { return HitReactMontage; }

	virtual void Die() override;

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();

protected:

	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo();
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	virtual FVector GetCombatSocketLocation() override;

	// Enemy용 AbilitySystem ,AttributeSet

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Seconary Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Vital Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	virtual void InitializeDefaultAttributes() const;

	void AddCharacterAbilities();

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	/* Dissolv Effect */

	void Dissolve();

	UFUNCTION()
	void UpdateDissolve(float DeltaTime);

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> DissolveCurve;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMaterialInstanceDynamic> DynamicDissolveMI;

	TObjectPtr<UTimelineComponent> DissolveTimeline;

	FOnTimelineFloat DissolveTimelineUpdate;


	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInstance> DissolveMI;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMaterialInstanceDynamic> WeaponDynamicDissolveMI;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInstance> WeaponDissolveMI;

private:

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities; // 게임 시작시 부여되는 능력.

};
