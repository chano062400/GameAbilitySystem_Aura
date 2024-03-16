#include "AbilitySystem/Abilities/AuraFireBolt.h"
#include "AuraGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{

	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"
			
			// Level
			"<Small>Level: </><Level>%d</>\n"
			
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Launched a bolt of fire, exploding on impact and dealing: </>"
			
			// Damage
			"<Damage>%d</><Default> fire damage with "
			"a chance to burn</>\n\n"), 
			
			// Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"

			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			// Num of Projectiles
			"<Default>Launched %d bolt of fire, exploding on impact and dealing: </>"

			// Damage
			"<Damage>%d</><Default> fire damage with "
			"a chance to burn</>\n\n"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(NumOfProjectiles, Level),
			ScaledDamage);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 NextLevel)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(NextLevel);
	const float ManaCost = FMath::Abs(GetManaCost(NextLevel));
	const float Cooldown = GetCooldown(NextLevel);

	return FString::Printf(TEXT(
		// Title
		"<Title>NEXT LEVEL</>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"

		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

		// Num of Projectiles
		"<Default>Launched %d bolt of fire, exploding on impact and dealing: </>"

		// Damage
		"<Damage>%d</><Default> fire damage with "
		"a chance to burn</>\n\n"),

		// Values
		NextLevel,
		ManaCost,
		Cooldown,
		FMath::Min(NumOfProjectiles, NextLevel),
		ScaledDamage);
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	TScriptInterface<ICombatInterface> CombatInterface = TScriptInterface<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		if (bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}

		const FVector Forward = Rotation.Vector();
		const int32 EffectiveNumOfProjectiles = FMath::Min(MaxNumOfProjectiles, GetAbilityLevel());
		TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumOfProjectiles);

		for (const FRotator& Rot : Rotators)
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SocketLocation);
			SpawnTransform.SetRotation(Rot.Quaternion());

			AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>
				(
					ProjectileClass,
					SpawnTransform,
					GetOwningActorFromActorInfo(),
					Cast<APawn>(GetOwningActorFromActorInfo()),
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn
				);

			Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults(nullptr);

			if (IsValid(HomingTarget) && HomingTarget->Implements<UCombatInterface>())
			{
				/**
				* The current target we are homing towards. Can only be set at runtime (when projectile is spawned or updating).
				* @see bIsHomingProjectile
				*/
				Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
			}
			else
			{
				Projectile->HomingTargetComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
				Projectile->HomingTargetComponent->SetWorldLocation(ProjectileTargetLocation);
				Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetComponent;
			}
			/** The magnitude of our acceleration towards the homing target. Overall velocity magnitude will still be limited by MaxSpeed. */
			Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
			
			/**
			* If true, we will accelerate toward our homing target. HomingTargetComponent must be set after the projectile is spawned.
			* @see HomingTargetComponent, HomingAccelerationMagnitude
			*/
			Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
			Projectile->FinishSpawning(SpawnTransform);
		}
	}

	
}
 