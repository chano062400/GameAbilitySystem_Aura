#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"
#include "AuraGameplayTags.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveSpellActivate);
		const bool bStartUpAblitiesActivated = AuraASC->bStartUpAbilitiesGiven;
		if (bStartUpAblitiesActivated)
		{
			if (AuraASC->GetStautusFromAbilityTag(PassiveSpellTag).MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
			{
				Activate();
			}
		}
	}
	else if(TScriptInterface<ICombatInterface> Interface = TScriptInterface<ICombatInterface>(GetOwner()))
	{
		Interface->GetOnASCRegisteredDelegate().AddLambda(
			[this](UAbilitySystemComponent* ASC)
			{
				if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(ASC))
				{
					AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveSpellActivate);
					const bool bStartUpAblitiesActivated = AuraASC->bStartUpAbilitiesGiven;
					if (bStartUpAblitiesActivated)
					{
						if (AuraASC->GetStautusFromAbilityTag(PassiveSpellTag).MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
						{
							Activate();
						}
					}
				}
			}
		);
	}
}

void UPassiveNiagaraComponent::OnPassiveSpellActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		if (bActivate && !IsActive())
		{
			Activate();
		}
		else
		{
			Deactivate();
		}
	}
	
}
