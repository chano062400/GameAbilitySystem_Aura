#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	
}

void UAuraProjectileSpell::SpawnProjectile()
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer) return;

	TScriptInterface<ICombatInterface> CombatInterface = TScriptInterface<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

		// To Do (�߻�ü ȸ�� ����)

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);

		//SpawnActor�Լ��� ������ ������Ʈ�� �ν��Ͻ�(��ü)�� �����ϰ� ���忡 ��ġ�ϴ� �ݸ�,
		//SpawnActorDeffered�Լ��� ���ϴ� ������Ʈ�� ��ü�� �����ϰ� ������ FinishSpawning�Լ��� ȣ�� �� ������ ���忡 ��ġ�Ѵ�.
		AAuraProjectile* AuraProjectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// To Do (GameplayEffect�� ���ؼ� Damage�� �ִ� ���)

		AuraProjectile->FinishSpawning(SpawnTransform);
	}
}


