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
#include "AbilitySystem/DebuffNiagaraComponent.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AuraGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

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

	LoadProgress();

}

void AAuraCharacter::LoadProgress()
{
	if (AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		if (UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance()))
		{
			ULoadScreenSaveGame* SaveGameObject = AuraGameMode->GetSaveSlotData(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex);
			if (SaveGameObject == nullptr) return;

			if (SaveGameObject->bFirstTimeLoad)
			{
				InitializeDefaultAttributes();
				AddCharacterAbilities();
			}
			else
			{
				if (AAuraPlayerState* PS = Cast<AAuraPlayerState>(GetPlayerState()))
				{
					PS->SetLevel(SaveGameObject->PlayerLevel);
					PS->SetXP(SaveGameObject->XP);
					PS->SetSpellPoint(SaveGameObject->SpellPoints);
					PS->SetAttributePoint(SaveGameObject->AttributePoints);
				}

				UAuraAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, GetAbilitySystemComponent(), SaveGameObject);
			}
		}
	}
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

void AAuraCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		AuraPlayerController->ShowMagicCircle(DecalMaterial);
		AuraPlayerController->bShowMouseCursor = false;
	}
}

void AAuraCharacter::HideMagicCircle_Implementation()
{
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		AuraPlayerController->HideMagicCircle();
		AuraPlayerController->bShowMouseCursor = true;
	}
}

void AAuraCharacter::SaveProgress_Implementation(const FName& CheckPointTag)
{
	if (AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		if (UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance()))
		{
			ULoadScreenSaveGame* SaveGameObject = AuraGameMode->GetSaveSlotData(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex);
			if (SaveGameObject == nullptr) return;

			SaveGameObject->PlayerStartTag = CheckPointTag;
			
			if(AAuraPlayerState* PS = Cast<AAuraPlayerState>(GetPlayerState()))
			{
				SaveGameObject->PlayerLevel = PS->GetPlayerLevel();
				SaveGameObject->XP = PS->GetXP();
				SaveGameObject->SpellPoints = PS->GetSpellPoint();
				SaveGameObject->AttributePoints = PS->GetAttributePoint();
			}

			SaveGameObject->Strength = UAuraAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
			SaveGameObject->Intelligence = UAuraAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
			SaveGameObject->Resilience = UAuraAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
			SaveGameObject->Vigor = UAuraAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());
			SaveGameObject->bFirstTimeLoad = false;

			AuraGameMode->SaveInGameProgressData(SaveGameObject);
		}
	}
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

void AAuraCharacter::OnRep_bIsStunned()
{
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_CursorTrace);
		BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputPressed);
		BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputHeld);
		BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputReleased);

		if (bIsStunned)
		{		
			/*GameplayEffect�� ���� �������� �ʴ� Loose GameplayTag�� �߰��� �� �ֵ��� ����մϴ�.
			* �̷��� �߰��� Tag�� Replicate���� �ʽ��ϴ�! Replicate�� �ʿ��� ��� �̷��� ����� 'Replicated' ������ ����Ͻʽÿ�.
			* �ʿ��� ��� �̷��� �±װ� Ŭ���̾�Ʈ/������ �߰��Ǿ����� Ȯ���ϴ� ���� ȣ���ϴ� GameCode�� �޷� �ֽ��ϴ�.*/
			AuraASC->AddLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Activate();
		}
		else
		{
			AuraASC->RemoveLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Deactivate();
		}
	}
}

void AAuraCharacter::OnRep_bIsBurned()
{
	if (bIsBurned)
	{
		BurnDebuffComponent->Activate();
	}
	else
	{
		BurnDebuffComponent->Deactivate();
	}
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
	};
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

