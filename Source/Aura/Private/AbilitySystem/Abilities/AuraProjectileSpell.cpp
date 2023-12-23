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

		const UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
		const FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
		
		FAuraGameplayTags AuraGameplayTag = FAuraGameplayTags::Get();
		const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel()); // AbilityLevel에 맞는 Curve 값을 반환.
		GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Black, FString::Printf(TEXT("Damage = %f"), ScaledDamage));
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, AuraGameplayTag.Damage, ScaledDamage);
		AuraProjectile->DamageEffectSpecHandle = EffectSpecHandle;

		AuraProjectile->FinishSpawning(SpawnTransform);
	}
}


