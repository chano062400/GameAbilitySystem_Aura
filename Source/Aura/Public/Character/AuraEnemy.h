// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraEnemy.generated.h"

class UBehaviorTree;
class AAuraAIController;
/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:

	AAuraEnemy();

	virtual void PossessedBy(AController* NewController) override;

	/* Enemy Interface */
	
	virtual void HighlightActor() override;

	virtual void UnHighlightActor() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHightlight = false;

	/* Combat Interface */

	virtual int32 GetPlayerLevel() override;

protected:

	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Class Defauls")
		int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<AAuraAIController> AuraAIController;

private:


};
