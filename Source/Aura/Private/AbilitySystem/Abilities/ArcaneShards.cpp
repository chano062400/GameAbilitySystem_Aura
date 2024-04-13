#include "AbilitySystem/Abilities/ArcaneShards.h"

FString UArcaneShards::GetDescription(int32 Level)
{
	int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	float ManaCost = GetManaCost();
	float Cooldown = GetCooldown();

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ARCANE SHARDS</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"

			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Summon a Shard of arcane energy, causing radial arcane damage </>"

			// Damage
			"<Damage>%d</><Default> at the shards origin.</>\n\n"),

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
			"<Title>ARCANE SHARDS</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"

			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Num of Targtes
			"<Default>Summon %d Shards of arcane energy, causing radial arcane damage</>"

			// Damage
			"<Damage>%d</><Default> at the shards origin </>\n\n"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(MaxNumOfShards, Level),
			ScaledDamage);
	}
}

FString UArcaneShards::GetNextLevelDescription(int32 NextLevel)
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
		"<Default>Summon %d Shards of arcane energy, causing radial arcane damage</>"

		// Damage
		"<Damage>%d</><Default> at the shards origin </>\n\n"),

		// Values
		NextLevel,
		ManaCost,
		Cooldown,
		FMath::Min(MaxNumOfShards, NextLevel),
		ScaledDamage);
}
