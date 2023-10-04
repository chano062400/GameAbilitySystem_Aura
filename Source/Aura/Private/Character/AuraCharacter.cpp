// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Player/AuraPlayerState.h"

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

	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
}

