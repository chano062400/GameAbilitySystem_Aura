// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:

	AAuraEnemy();

	virtual void HighlightActor() override;

	virtual void UnHighlightActor() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHightlight = false;

protected:

	virtual void BeginPlay() override;

private:


};
