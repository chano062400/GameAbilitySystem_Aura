
#include "AsyncTasks/WaitCooldownChange.h"
#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(WaitCooldownChange) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// Cooldown�� ���ŵ��� ��
	AbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag, 
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange, &UWaitCooldownChange::CooldownTagChanged
		);

	/** Called on both client and server whenever a duraton based GE is added (E.g., instant GEs do not trigger this). */
	// 	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGameplayEffectAppliedDelegate, UAbilitySystemComponent*, const FGameplayEffectSpec&, FActiveGameplayEffectHandle);
	// Cooldown�� ������� ��
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;

	// Remove All -������ UserObject�� ���ε��� �� ��Ƽĳ��Ʈ �븮���� ȣ�� ��Ͽ��� ��� �Լ��� �����մϴ�. ��ǥ���� ������ �������� ���� ���� �ִٴ� ���� �����ϼ���!
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	/** Call when the action is completely done, this makes the action free to delete, and will unregister it with the game instance */
	SetReadyToDestroy();

	/**Marks this object as Garbage.*/
	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// ��ٿ��� ����.
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}

}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTasgs;
	SpecApplied.GetAllGrantedTags(GrantedTasgs);

	if (AssetTags.HasTagExact(CooldownTag) || GrantedTasgs.HasTagExact(CooldownTag))
	{
		//CooldownTag�� ���Ե� TagContainter�� �����ͼ� EffectQuery�� ����.
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());

		// Query�� ��� Effect�� TimeRemaing�� ��ȯ��.
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			float RemainingTime = TimesRemaining[0]; 
			//�ִ� ã��
			for (int i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > RemainingTime)
				{
					RemainingTime = TimesRemaining[i];
				}
			}

			CooldownStart.Broadcast(RemainingTime);
		}
	}
}
