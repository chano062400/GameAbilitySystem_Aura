#include "AbilitySystem/Abilities/Electrocute.h"

FString UElectrocute::GetDescription(int32 Level)
{
	int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	float ManaCost = GetManaCost();
	float Cooldown = GetCooldown();

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ELECTROCUTE</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"

			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Emit a beam of lightning, connecting with the target, repeatedly causing </>"

			// Damage
			"<Damage>%d</><Default> lightning damage with "
			"a chance to stun</>\n\n"),

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
			"<Title>ELECTROCUTE</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"

			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Num of Targtes
			"<Default>Emit a beam of lightning, Propagating to %d additional targets nearby, causing</>"

			// Damage
			"<Damage>%d</><Default> lightning damage with "
			"a chance to stun</>\n\n"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(MAxNumOfTargets, Level),
			ScaledDamage);
	}
}

FString UElectrocute::GetNextLevelDescription(int32 NextLevel)
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
		"<Default>Emit a beam of lightning, Propagating to %d additional targets nearby, causing</>"

		// Damage
		"<Damage>%d</><Default> lightning damage with "
		"a chance to stun</>\n\n"),

		// Values
		NextLevel,
		ManaCost,
		Cooldown,
		FMath::Min(MAxNumOfTargets, NextLevel),
		ScaledDamage);
}
