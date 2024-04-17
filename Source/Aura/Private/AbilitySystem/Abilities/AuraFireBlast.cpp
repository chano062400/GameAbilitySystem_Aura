#include "AbilitySystem/Abilities/AuraFireBlast.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraAbilityTypes.h"
#include "Actor/AuraFireBall.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	return FString::Printf(TEXT(
		// Title
		"<Title>FIRE BOLT</>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"

		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

		// Num of FireBalls
		"<Default>Launched %d Fire Balls </>"
		"<Default>fire balls of in all directions, each coming back and </>"
		"<Default>exploding upon return, causing"

		// Damage
		"<Damage>%d</><Default>radial fire damage with "
		"a chance to burn</>\n\n"),

		// Values
		Level,
		ManaCost,
		Cooldown,
		NumOfFireBalls,
		ScaledDamage);

}

FString UAuraFireBlast::GetNextLevelDescription(int32 NextLevel)
{
	return GetDescription(NextLevel);
}

TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> FireBalls;

	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	TArray<FRotator> Rotators =	UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumOfFireBalls);

	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetAvatarActorFromActorInfo()->GetActorLocation());
		SpawnTransform.SetRotation(Rotator.Quaternion());
		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass, 
			SpawnTransform, 
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults(nullptr);
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();

		FireBalls.AddUnique(FireBall);

		FireBall->FinishSpawning(SpawnTransform);
	}

	return FireBalls;
}
