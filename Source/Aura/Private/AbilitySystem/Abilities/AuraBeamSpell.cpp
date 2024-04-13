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
		// IsAlreadyBound - 특정 함수가 Bind돼있는지 확인.
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

	// 자기 자신과 First Target은 무시.
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);

	// MouseTarget을 기준으로 Radius안에 있는 Overlapping Targets를 가져옴.
	UAuraAbilitySystemLibrary::GetLivePlayersWithInRadius(GetAvatarActorFromActorInfo(), 
		OverlappingActors, 
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation()
		);

	int32 NumOfAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MAxNumOfTargets);

	// Overlapping Targets중 AdditionalTargets를 가까운 순서대로 가져옴.
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
