#include "Character/AuraCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>("Dissolve Timeline");

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

void AAuraCharacterBase::Die()
{
	FDetachmentTransformRules Rule = FDetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	Weapon->DetachFromComponent(Rule);
	MulticastHandleDeath();
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Dissolve();
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

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation()
{
	check(Weapon);
	return Weapon->GetSocketLocation(WeaponTipSocketName);
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
	DynamicDissolveMI->SetScalarParameterValue(FName("Dissolve"), DeltaTime);
	WeaponDynamicDissolveMI->SetScalarParameterValue(FName("Dissolve"), DeltaTime);
}
