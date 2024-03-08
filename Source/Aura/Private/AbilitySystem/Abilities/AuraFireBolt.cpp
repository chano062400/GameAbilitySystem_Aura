#include "AbilitySystem/Abilities/AuraFireBolt.h"
#include "AuraGameplayTags.h"

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
