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
		//WeakLambda - Lambda���� Ư�� NiagaraComponent�� ĸó�ϸ� �ڵ������� GC�� �����ǰ�, ������ �����ϰ� �ִ� �� �ڵ����� �������� ����.
		// �׷��� WeakLambda�� ������ �������� �ʰ�, �����ϴ� Ư���� �־, ����ؼ� GC�� ������.
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
