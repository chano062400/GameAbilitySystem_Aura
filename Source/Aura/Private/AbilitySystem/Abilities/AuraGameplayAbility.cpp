#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/AuraAttributeSet.h"

FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	// L - 넓은 문자. LoremIpusum - 채우기용 
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"), L"Default Ability Name - LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum ", Level);
}

FString UAuraGameplayAbility::GetNextLevelDescription(int32 NextLevel)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more Damage </>"), NextLevel);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 LevelRequirement)
{
	return FString::Printf(TEXT("<Default>This Spell Locked Until Level: %d</>"), LevelRequirement);
}

float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;

	// GameplayAbility의 CostGameplayEffect를 가져옴.
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		// CostGameplayEffect의 Modifier들을 탐색.
		for (FGameplayModifierInfo Modifier : CostEffect->Modifiers)
		{
			// Modifier의 Attribute가 ManaAttribute라면
			if (Modifier.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				/** GetStaticMagnitudeIfPossible() - Returns the magnitude as it was entered in data. Only applies to ScalableFloat or any other type that can return data without context */
				// 하드코딩된 값이나, FScalablefloat로 값이 설정된 경우에만 가져올 수 있음.
				Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}

	return ManaCost;
}

float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;

	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}

	return Cooldown;
}
