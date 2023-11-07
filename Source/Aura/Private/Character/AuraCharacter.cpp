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
	GetCharacterMovement()->bConstrainToPlane = true; // ĳ������ �̵��� ���(NavMesh)���� ����.
	GetCharacterMovement()->bSnapToPlaneAtStart = true; //������ �� ĳ������ ��ġ�� ����� ��� ���¶�� ����� ���(NavMesh)���� �ٿ��� ���۵ǵ��� �Ѵ�

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

}

void AAuraCharacter::PossessedBy(AController* NewController) // ����
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();

}

void AAuraCharacter::OnRep_PlayerState() // Ŭ���̾�Ʈ
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

	
	/* Assertion�� ��Ƽ ���ӿ��� ���� �÷��̾��� PlayerController ���� �ٸ� Ŭ���̾�Ʈ���� PlayerController�� ��ȿ���� ������ �ߴܵ�(��� Ŭ���̾�Ʈ�� PlayerController�� ��ȿ�ؾ� ����)
	�׷��� �츮�� ���� nullptr�� �ƴϸ� �����ϰ� �����Ƿ� if������ ������.*/ 
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

