// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();

	UFUNCTION(BlueprintPure)
	TSubclassOf<APawn> GetRandomMinionClass();

	// ��ȯ�ϴ� �̴Ͼ��� ��
	UPROPERTY(EditDefaultsOnly, Category = "Summon")
	int32 NumofMinions = 5;

	// ��ȯ�� �̴Ͼ���� ��� TArray
	UPROPERTY(EditDefaultsOnly, Category = "Summon")
	TArray<TSubclassOf<APawn>> MinionClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Summon")
	float MinSummonDistance = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Summon")
	float MaxSummonDistance = 250.f;

	// ��ȯ����� �������� ����.
	UPROPERTY(EditDefaultsOnly, Category = "Summon")
	float SummonSpread = 90.f;

};
