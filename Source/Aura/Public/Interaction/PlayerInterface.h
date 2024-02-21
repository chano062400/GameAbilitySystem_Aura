#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IPlayerInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(int32 InXP) const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP() const;

	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(int32 InXP);

	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();

	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(int32 InPlayerLevel);

	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 CurLevel) const;
	
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointsReward(int32 CurLevel) const;

	UFUNCTION(BlueprintNativeEvent)
	void AddAttributePointsReward(int32 InAttributePoint);
	
	UFUNCTION(BlueprintNativeEvent)
	void AddSpellPointsReward(int32 InSpellPoint);


};
