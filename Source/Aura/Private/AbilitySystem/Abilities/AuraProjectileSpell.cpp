#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Aura/Public/AuraGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"

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
		const FVector SocketLocation = CombatInterface->Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), FAuraGameplayTags::Get().Montage_Attack_Weapon);

		FRotator Rotation = (TargetLocation - SocketLocation).Rotation(); //�߻� ����
		//Rotation.Pitch = 0.f; //����� �����ϰ� ������.

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation); // ������ġ ����
		SpawnTransform.SetRotation(Rotation.Quaternion()); //�������� ����.

		//SpawnActor�Լ��� ������ ������Ʈ�� �ν��Ͻ�(��ü)�� �����ϰ� ���忡 ��ġ�ϴ� �ݸ�,
		//SpawnActorDeffered�Լ��� ���ϴ� ������Ʈ�� ��ü�� �����ϰ� ������ FinishSpawning�Լ��� ȣ�� �� ������ ���忡 ��ġ�Ѵ�.
		AAuraProjectile* AuraProjectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		const UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		
		//AbilityInstanceNotReplicated, AbilityCDO, AbilityLevel ����.
		EffectContextHandle.SetAbility(this);
		
		EffectContextHandle.AddSourceObject(AuraProjectile);
		
		TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Add(AuraProjectile);
		EffectContextHandle.AddActors(Actors);
		
		FHitResult HitResult;
		HitResult.Location = TargetLocation;
		EffectContextHandle.AddHitResult(HitResult);

		const FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
		
		FAuraGameplayTags AuraGameplayTag = FAuraGameplayTags::Get();
		
		for (auto& pair : DamageTypes)
		{
			const float  ScaledDamage = pair.Value.GetValueAtLevel(GetAbilityLevel()); //  Scalable Float - Curve Table���� AbilityLevel�� �� = Damage
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, pair.Key, ScaledDamage); // DamageTypes �߿��� Tag(Key��)�� ã�Ƽ� Damage ���� ����.(Value��)
		}

		AuraProjectile->DamageEffectSpecHandle = EffectSpecHandle;

		AuraProjectile->FinishSpawning(SpawnTransform);
	}
}


