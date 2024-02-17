#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		//LevelUpInformation[1] = 1 Level
		//LevelUpInformation[2] = 2 Level
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		// 해당 레벨 요구 XP보다 많은 경우(레벨업)
		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}

	return Level;
}
