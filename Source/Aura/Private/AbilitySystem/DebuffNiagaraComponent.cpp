#include "AbilitySystem/DebuffNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"
UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	bAutoActivate = false;
}

void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	TScriptInterface<ICombatInterface> CombatInterface = TScriptInterface<ICombatInterface>(GetOwner());
	
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		/** Allow events to be registered for specific gameplay tags being added or removed */
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
	}
	else if (CombatInterface)
	{
		//WeakLambda - Lambda에서 특정 NiagaraComponent를 캡처하면 자동적으로 GC에 수집되고, 참조를 보유하고 있는 한 자동으로 삭제되지 않음.
		// 그러나 WeakLambda는 참조를 증가하지 않고, 유지하는 특성이 있어서, 계속해서 GC에 수집됨.
		CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(
			this, [this](UAbilitySystemComponent* InASC)
			{
				InASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
			}
		);
	}
	if (CombatInterface)
	{
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDead);
	}
}

void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		Activate();
	}
	else
	{
		Deactivate();
	}
}

void UDebuffNiagaraComponent::OnOwnerDead(AActor* DeadActor)
{
	Deactivate();
}
