#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"


class AAuraPlayerController;
class AAuraHUD;
class UNiagaraComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
	
public:
	
	AAuraCharacter();
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	/* Player Interface */

	virtual void AddToXP_Implementation(int32 InXP) override;

	virtual int32 GetXP_Implementation() const override;

	virtual void LevelUp_Implementation() override;

	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;

	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;

	virtual int32 GetAttributePointsReward_Implementation(int32 CurLevel) const override;

	virtual int32 GetSpellPointsReward_Implementation(int32 CurLevel) const override;

	virtual void AddAttributePointsReward_Implementation(int32 InAttributePoint)override;

	virtual void AddSpellPointsReward_Implementation(int32 InSpellPoint)override;

	virtual int32 GetAttributePoint_Implementation() const override;

	virtual int32 GetSpellPoint_Implementation() const override;

	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial = nullptr) override;

	virtual void HideMagicCircle_Implementation() override;

	/* Combat interface */

	virtual int32 GetPlayerLevel_Implementation() override;

	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

	virtual void OnRep_bIsStunned() override;

	virtual void OnRep_bIsBurned() override;

protected:


private:
	 
	virtual void InitAbilityActorInfo() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayLevelUpEffect();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCamera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraSpringArm;
};
