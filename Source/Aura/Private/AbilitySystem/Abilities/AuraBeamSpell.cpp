#include "AbilitySystem/Abilities/AuraBeamSpell.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"

void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& MouseHitResult)
{
	if(MouseHitResult.bBlockingHit)
	{
		MouseHitLocation = MouseHitResult.ImpactPoint;
		MouseHitActor = MouseHitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, GetCurrentActivationInfo(), true);
	}
}

void UAuraBeamSpell::StoreOwnerPlayerController()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);

			FHitResult HitResult;

			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));

			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				10.f,
				TraceTypeQuery1,
				false,
				ActorsToIgnore,
				bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
				HitResult,
				true
			);

			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
	if (TScriptInterface<ICombatInterface> Interface = TScriptInterface<ICombatInterface>(MouseHitActor))
	{
		// IsAlreadyBound - Ư�� �Լ��� Bind���ִ��� Ȯ��.
		if (!Interface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::FirstTargetDead))
		{
			Interface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::FirstTargetDead);
		}
	}
}

void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> ActorsToIgnore;

	// �ڱ� �ڽŰ� First Target�� ����.
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);

	// MouseTarget�� �������� Radius�ȿ� �ִ� Overlapping Targets�� ������.
	UAuraAbilitySystemLibrary::GetLivePlayersWithInRadius(GetAvatarActorFromActorInfo(), 
		OverlappingActors, 
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation()
		);

	int32 NumOfAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MAxNumOfTargets);

	// Overlapping Targets�� AdditionalTargets�� ����� ������� ������.
	UAuraAbilitySystemLibrary::GetClosestTargets(NumOfAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());

	for (AActor* Actor : OutAdditionalTargets)
	{
		if (TScriptInterface<ICombatInterface> Interface = TScriptInterface<ICombatInterface>(Actor))
		{
			if (!Interface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::AdditionalTargetDead))
			{
				Interface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::AdditionalTargetDead);
			}
		}
	}
}
