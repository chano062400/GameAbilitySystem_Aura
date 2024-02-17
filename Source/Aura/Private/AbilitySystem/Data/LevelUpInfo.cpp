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

		// �ش� ���� �䱸 XP���� ���� ���(������)
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
