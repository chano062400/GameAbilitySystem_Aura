#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bIsServer = HasAuthority(&ActivationInfo);

	if (!bIsServer) return;


	TScriptInterface<ICombatInterface> CombatInterface = TScriptInterface<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

		// To Do (발사체 회전 설정)

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);

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
