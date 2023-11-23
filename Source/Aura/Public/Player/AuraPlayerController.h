// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class IEnemyInterface;
class UAuraInputConfig;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	AAuraPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	void Move(const struct FInputActionValue& Value);

	void CursorTrace();

private:

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputMappingContext> AuraMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TScriptInterface<IEnemyInterface> LastActor;

	UPROPERTY()
	TScriptInterface<IEnemyInterface> ThisActor;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	
	void AbilityInputTagReleased(FGameplayTag InputTag);
	
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UAuraInputConfig> InputConfig;


};
