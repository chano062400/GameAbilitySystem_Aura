
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

	// Cooldown이 제거됐을 때
	AbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag, 
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange, &UWaitCooldownChange::CooldownTagChanged
		);

	/** Called on both client and server whenever a duraton based GE is added (E.g., instant GEs do not trigger this). */
	// 	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGameplayEffectAppliedDelegate, UAbilitySystemComponent*, const FGameplayEffectSpec&, FActiveGameplayEffectHandle);
	// Cooldown이 적용됐을 때
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;

	// Remove All -지정된 UserObject에 바인딩된 이 멀티캐스트 대리자의 호출 목록에서 모든 함수를 제거합니다. 대표자의 순서가 유지되지 않을 수도 있다는 점에 유의하세요!
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	/** Call when the action is completely done, this makes the action free to delete, and will unregister it with the game instance */
	SetReadyToDestroy();

	/**Marks this object as Garbage.*/
	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 쿨다운이 끝남.
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
		//CooldownTag만 포함된 TagContainter를 가져와서 EffectQuery를 만듦.
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());

		// Query의 모든 Effect의 TimeRemaing을 반환함.
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			float RemainingTime = TimesRemaining[0]; 
			//최댓값 찾기
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
