// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Player/AuraPlayerState.h"
#include "Player/AuraPlayerController.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true; // 캐릭터의 이동을 평면(NavMesh)으로 제한.
	GetCharacterMovement()->bSnapToPlaneAtStart = true; //시작할 때 캐릭터의 위치가 평면을 벗어난 상태라면 가까운 평면(NavMesh)으로 붙여서 시작되도록 한다

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

}

void AAuraCharacter::PossessedBy(AController* NewController) // 서버
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();

}

void AAuraCharacter::OnRep_PlayerState() // 클라이언트
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo(); 

}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	
	/* Assertion은 멀티 게임에서 로컬 플레이어의 PlayerController 말고도 다른 클라이언트들의 PlayerController가 유효하지 않으면 중단됨(모든 클라이언트의 PlayerController가 유효해야 진행)
	그러나 우리는 단지 nullptr가 아니면 진행하고 싶으므로 if문으로 래핑함.*/ 
	/*check(AuraPlayerController);*/
	AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController());
	if (AuraPlayerController) 
	{
		AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD());
		if (AuraHUD)
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	InitializeDefaultAttributes();
}

