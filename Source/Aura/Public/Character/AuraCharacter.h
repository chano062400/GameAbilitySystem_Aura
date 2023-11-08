#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"


class AAuraPlayerController;
class AAuraHUD;
/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()
	
public:
	
	AAuraCharacter();
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	/* Combat interface */

	virtual int32 GetPlayerLevel() override;

protected:


private:
	 
	virtual void InitAbilityActorInfo() override;
};
