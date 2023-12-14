#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& TargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer) return;

	TScriptInterface<ICombatInterface> CombatInterface = TScriptInterface<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

		FRotator Rotation = (TargetLocation - SocketLocation).Rotation(); //발사 방향
		Rotation.Pitch = 0.f; //지면과 평행하게 가도록.

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation); // 스폰위치 설정
		SpawnTransform.SetRotation(Rotation.Quaternion()); //스폰방향 설정.

		//SpawnActor함수는 지정한 오브젝트의 인스턴스(객체)를 생성하고 월드에 배치하는 반면,
		//SpawnActorDeffered함수는 원하는 오브젝트의 객체를 생성하고 액터의 FinishSpawning함수를 호출 할 때에만 월드에 배치한다.
		AAuraProjectile* AuraProjectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// To Do (GameplayEffect를 통해서 Damage를 주는 기능)

		AuraProjectile->FinishSpawning(SpawnTransform);
	}
}


