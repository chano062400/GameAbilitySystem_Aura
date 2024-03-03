#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

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
