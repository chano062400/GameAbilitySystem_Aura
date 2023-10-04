#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/EnemyInterface.h"
#include "AuraCharacterBase.generated.h"

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IEnemyInterface
{
	GENERATED_BODY()

public:

	AAuraCharacterBase();

	virtual void HighlightActor() override;
	
	virtual void UnHighlightActor() override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

private:


};
