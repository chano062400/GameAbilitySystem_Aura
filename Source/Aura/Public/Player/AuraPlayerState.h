
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AuraPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/);

class ULevelUpInfo;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AAuraPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	FOnPlayerStatChanged OnXPChangedDelegate;

	FOnPlayerStatChanged OnLevelChangedDelegate;
	
	FOnPlayerStatChanged OnAttributePointChangedDelegate;

	FOnPlayerStatChanged OnSpellPointChangedDelegate;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;

	FORCEINLINE int32 GetPlayerLevel() { return Level; }

	FORCEINLINE int32 GetXP() { return XP; }
	
	FORCEINLINE int32 GetAttributePoint() { return AttributePoint; }

	FORCEINLINE int32 GetSpellPoint() { return SpellPoint; }

	void AddToXP(int32 InXP);

	void SetXP(int32 InXP);

	void AddToLevel(int32 InLevel);

	void SetLevel(int32 InLevel);

	void AddToAttributePoint(int32 InAttributePoint);

	void SetAttributePoint(int32 InAttributePoint);

	void AddToSpellPoint(int32 InSpellPoint);

	void SetSpellPoint(int32 InSpellPoint);

protected:

	// Player¿ë AbilitySystem, AttributeSet

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<class UAttributeSet> AttributeSet;

private:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 1;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 1;
		
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_AttributePoint)
	int32 AttributePoint = 0;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SpellPoint)
	int32 SpellPoint = 0;

	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UFUNCTION()
	void OnRep_XP(int32 OldXP);
	
	UFUNCTION()
	void OnRep_AttributePoint(int32 OldAttributePoint);	
	
	UFUNCTION()
	void OnRep_SpellPoint(int32 OldAttributePoint);
};
