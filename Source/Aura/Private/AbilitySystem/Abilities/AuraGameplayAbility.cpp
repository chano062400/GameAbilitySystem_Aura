#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/AuraAttributeSet.h"

FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	// L - ���� ����. LoremIpusum - ä���� 
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

	// GameplayAbility�� CostGameplayEffect�� ������.
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		// CostGameplayEffect�� Modifier���� Ž��.
		for (FGameplayModifierInfo Modifier : CostEffect->Modifiers)
		{
			// Modifier�� Attribute�� ManaAttribute���
			if (Modifier.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				/** GetStaticMagnitudeIfPossible() - Returns the magnitude as it was entered in data. Only applies to ScalableFloat or any other type that can return data without context */
				// �ϵ��ڵ��� ���̳�, FScalablefloat�� ���� ������ ��쿡�� ������ �� ����.
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
