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
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

		FRotator Rotation = (TargetLocation - SocketLocation).Rotation(); //�߻� ����
		Rotation.Pitch = 0.f; //����� �����ϰ� ������.

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
		const FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
		
		FAuraGameplayTags AuraGameplayTag = FAuraGameplayTags::Get();
		const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel()); // AbilityLevel�� �´� Curve ���� ��ȯ.
		GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Black, FString::Printf(TEXT("Damage = %f"), ScaledDamage));
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, AuraGameplayTag.Damage, ScaledDamage);
		AuraProjectile->DamageEffectSpecHandle = EffectSpecHandle;

		AuraProjectile->FinishSpawning(SpawnTransform);
	}
}

