#include "Character/AuraCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "AuraGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/DebuffNiagaraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>("Dissolve Timeline");

	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("BurnDebuffComponent");
	BurnDebuffComponent->SetupAttachment(GetRootComponent());
	BurnDebuffComponent->DebuffTag = FAuraGameplayTags::Get().Debuff_Burn;

	StunDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("StunDebuffComponent");
	StunDebuffComponent->SetupAttachment(GetRootComponent());
	StunDebuffComponent->DebuffTag = FAuraGameplayTags::Get().Debuff_Stun;
}

void AAuraCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraCharacterBase, bIsStunned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBurned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBeingShocked);
}

void AAuraCharacterBase::HighlightActor()
{
	
}

void AAuraCharacterBase::UnHighlightActor()
{

}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAuraCharacterBase::Die(const FVector& DeathImpulse)
{
	FDetachmentTransformRules Rule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	Weapon->DetachFromComponent(Rule);
	MulticastHandleDeath(DeathImpulse);
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());

	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Weapon->AddImpulse(DeathImpulse * .1f, NAME_None, true);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	
	// bVelChange가 true인 경우 강도는 충격 대신 속도 변화로 간주됩니다(즉, 질량은 영향을 미치지 않습니다).
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Dissolve();

	bDead = true;

	BurnDebuffComponent->Deactivate();
	StunDebuffComponent->Deactivate();

	OnDeath.Broadcast(this);
}

void AAuraCharacterBase::OnRep_bIsStunned()
{
	
}

void AAuraCharacterBase::OnRep_bIsBurned()
{
}

void AAuraCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (DissolveCurve)
	{
		DissolveTimelineUpdate.BindDynamic(this, &AAuraCharacterBase::UpdateDissolve);
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTimelineUpdate);
	}

}

void AAuraCharacterBase::InitAbilityActorInfo()
{

}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(WeaponTipSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Tail))
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}

	return FVector();
}

bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

TArray<FTaggedMontage> AAuraCharacterBase::GetAttackMontage_Implementation()
{
	return AttackMontage;
}

UNiagaraSystem* AAuraCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage AAuraCharacterBase::GetTaggedMontage_Implementation(const FGameplayTag& MontageTag)
{
	for (FTaggedMontage TaggedMontage : AttackMontage)
	{
		if (TaggedMontage.MontageTag.MatchesTagExact(MontageTag))
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}

int32 AAuraCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void AAuraCharacterBase::UpdateMinionCount_Implementation(int32 Amount)
{
	MinionCount += Amount;
}

FOnASCRegistered& AAuraCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnASCRegistered;
}

FOnDeath& AAuraCharacterBase::GetOnDeathDelegate()
{
	return OnDeath;
}

bool AAuraCharacterBase::IsBeingShocked_Implementation() const
{
	return bIsBeingShocked;
}

void AAuraCharacterBase::SetIsBeingShocked_Implementation(bool bInIsBeingShocked)
{
	bIsBeingShocked = bInIsBeingShocked;
}

void AAuraCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);

	FGameplayEffectContextHandle EffectContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);

	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);

	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void AAuraCharacterBase::AddCharacterAbilities()
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);

	if (!HasAuthority()) return; 

	AuraASC->AddCharacterAbilities(StartupAbilities);

	AuraASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);	
}

void AAuraCharacterBase::Dissolve()
{
	if (IsValid(DissolveMI))
	{
		DynamicDissolveMI = UMaterialInstanceDynamic::Create(DissolveMI, this);
		DynamicDissolveMI->SetScalarParameterValue(FName("Dissolve"), -.1f);
		DynamicDissolveMI->SetScalarParameterValue(FName("Glow"), 250.f);
		GetMesh()->SetMaterial(0, DynamicDissolveMI);
	}
	
	if (IsValid(WeaponDissolveMI))
	{
		WeaponDynamicDissolveMI = UMaterialInstanceDynamic::Create(WeaponDissolveMI, this);
		WeaponDynamicDissolveMI->SetScalarParameterValue(FName("Dissolve"), -.1f);
		WeaponDynamicDissolveMI->SetScalarParameterValue(FName("Glow"), 250.f);
		Weapon->SetMaterial(0, WeaponDynamicDissolveMI);
	}

	DissolveTimeline->PlayFromStart();
}

void AAuraCharacterBase::UpdateDissolve(float DeltaTime)
{
	if(DynamicDissolveMI) DynamicDissolveMI->SetScalarParameterValue(FName("Dissolve"), DeltaTime);
	if(WeaponDynamicDissolveMI) WeaponDynamicDissolveMI->SetScalarParameterValue(FName("Dissolve"), DeltaTime);
}
