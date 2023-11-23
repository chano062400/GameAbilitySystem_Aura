#include "Input/AuraInputConfig.h"
#include "InputAction.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (auto& InputAction : AbilityInputAction)
	{
		if (InputAction.InputAction && InputAction.InputTag.MatchesTagExact(InputTag))
		{
			return InputAction.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't Find Ability Input Action For InputTag [%s] on InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
