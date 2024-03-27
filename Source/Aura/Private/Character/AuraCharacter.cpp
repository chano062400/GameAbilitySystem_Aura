// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Player/AuraPlayerState.h"
#include "Player/AuraPlayerController.h"
#include "UI/HUD/AuraHUD.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AuraGameplayTags.h"

AAuraCharacter::AAuraCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true; // ĳ������ �̵��� ���(NavMesh)���� ����.
	GetCharacterMovement()->bSnapToPlaneAtStart = true; //������ �� ĳ������ ��ġ�� ����� ��� ���¶�� ����� ���(NavMesh)���� �ٿ��� ���۵ǵ��� �Ѵ�

	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm Component"));
	CameraSpringArm->SetupAttachment(GetRootComponent());
	//bAbsoluteRotation�� ������ ���� ȸ���� ��Ʈ ������Ʈ�� ���� ������Ʈ�� ������ �ʰ� ���� ��ǥ���� ȸ���� �������� �Ѵ�.
	CameraSpringArm->SetUsingAbsoluteRotation(true);
	//bDoCollisionTest�� ī�޶� ���� ������ �浹 ����� ���� ī�޶�� ĳ������ �Ÿ��� ���� ī�޶� ���� ���� �ʰ� ������ִ� ������Ƽ������, Rpg���ӿ����� ������ �ʴ� �ɼ��̱� ������ false�� �����Ѵ�.
	CameraSpringArm->bDoCollisionTest = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDown Camera Component"));
	//SocketName - SpringArm�� ��.
	TopDownCamera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;

	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LevelUpNiagaraComponent"));
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void AAuraCharacter::PossessedBy(AController* NewController) // ����
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();

	AddCharacterAbilities();

}

void AAuraCharacter::OnRep_PlayerState() // Ŭ���̾�Ʈ
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo(); 

}

void AAuraCharacter::AddToXP_Implementation(int32 InXP)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	AuraPlayerState->AddToXP(InXP);
}

int32 AAuraCharacter::GetXP_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	return AuraPlayerState->GetXP();
}

void AAuraCharacter::LevelUp_Implementation()
{
	MulticastPlayLevelUpEffect();
}

void AAuraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToLevel(InPlayerLevel);

	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpdateAbilityStatuses(AuraPlayerState->GetPlayerLevel());
}

int32 AAuraCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	int32 Level = AuraPlayerState->LevelUpInfo->FindLevelForXP(InXP);

	return Level;
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 CurLevel) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	int32 AttributePointReward = AuraPlayerState->LevelUpInfo->LevelUpInformation[CurLevel].AttributePointAward;

	return AttributePointReward;
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 CurLevel) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	int32 SpellPointReward = AuraPlayerState->LevelUpInfo->LevelUpInformation[CurLevel].SpellPointAward;

	return SpellPointReward;
}

void AAuraCharacter::AddAttributePointsReward_Implementation(int32 InAttributePoint)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToAttributePoint(InAttributePoint);
}

void AAuraCharacter::AddSpellPointsReward_Implementation(int32 InSpellPoint)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToSpellPoint(InSpellPoint);
}

int32 AAuraCharacter::GetAttributePoint_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	
	return AuraPlayerState->GetAttributePoint();
}

int32 AAuraCharacter::GetSpellPoint_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	return AuraPlayerState->GetSpellPoint();
}

int32 AAuraCharacter::GetPlayerLevel_Implementation()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	return AuraPlayerState->GetPlayerLevel();
}

USkeletalMeshComponent* AAuraCharacter::GetWeapon_Implementation()
{
	return Weapon;
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraCharacter::StunTagChanged);
	AttributeSet = AuraPlayerState->GetAttributeSet();
	OnASCRegistered.Broadcast(AbilitySystemComponent);

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

void AAuraCharacter::MulticastPlayLevelUpEffect_Implementation()
{
	// LevelUpNiagara�� ī�޶� ������ ȸ�����Ѽ� ���.
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLoc = TopDownCamera->GetComponentLocation();
		const FVector NiagaraLoc = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLoc - NiagaraLoc).Rotation();

		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

