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
	GetCharacterMovement()->bConstrainToPlane = true; // 캐릭터의 이동을 평면(NavMesh)으로 제한.
	GetCharacterMovement()->bSnapToPlaneAtStart = true; //시작할 때 캐릭터의 위치가 평면을 벗어난 상태라면 가까운 평면(NavMesh)으로 붙여서 시작되도록 한다

	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm Component"));
	CameraSpringArm->SetupAttachment(GetRootComponent());
	//bAbsoluteRotation은 스프링 암의 회전이 루트 컴포넌트와 상위 컴포넌트를 따르지 않고 월드 좌표계의 회전을 따르도록 한다.
	CameraSpringArm->SetUsingAbsoluteRotation(true);
	//bDoCollisionTest는 카메라가 벽에 닿으면 충돌 계산을 통해 카메라와 캐릭터의 거리를 좁혀 카메라가 벽을 뚫지 않게 만들어주는 프로퍼티이지만, Rpg게임에서는 사용되지 않는 옵션이기 때문에 false로 설정한다.
	CameraSpringArm->bDoCollisionTest = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDown Camera Component"));
	//SocketName - SpringArm의 끝.
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

void AAuraCharacter::PossessedBy(AController* NewController) // 서버
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();

	AddCharacterAbilities();

}

void AAuraCharacter::OnRep_PlayerState() // 클라이언트
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

void AAuraCharacter::MulticastPlayLevelUpEffect_Implementation()
{
	// LevelUpNiagara를 카메라 쪽으로 회전시켜서 재생.
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLoc = TopDownCamera->GetComponentLocation();
		const FVector NiagaraLoc = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLoc - NiagaraLoc).Rotation();

		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

