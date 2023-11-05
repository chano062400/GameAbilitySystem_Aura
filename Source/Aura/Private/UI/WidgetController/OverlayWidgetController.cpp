

#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UOverlayWidgetController::BroadcastInitialValues()
{

	const UAuraAttributeSet* AuraAttributSet = CastChecked<UAuraAttributeSet>(AttributeSet); 
	
	//CastCheck�� ��ȿ���� ������ �ߴ��ϹǷ� ���� if���� �� �ʿ�X.
	OnHealthChanged.Broadcast(AuraAttributSet->GetHealth()); // ACCESSOR
	OnMaxHealthChanged.Broadcast(AuraAttributSet->GetMaxHealth()); // ACCESSOR

	OnManaChanged.Broadcast(AuraAttributSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies() // Attribute�� ����Ǿ�����.
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	//FOnGameplayAttributeValueChange Delegate�� DYNAMIC�� �ƴ϶� AddUObject�� ���.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetHealthAttribute()).AddLambda(//Health�� ����� ������ HealthChanged�� ȣ���.
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
			);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetMaxHealthAttribute()).AddLambda( //MaxHealth�� ����� ������ MaxHealthChanged�� ȣ���.
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
			);


		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
			);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
			);

	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda( //AddLamda - �Ｎ���� �Լ��� ����.
		[this](const FGameplayTagContainer& AssetTags /*�Ű�����*/)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				//"A.1".MatchesTag("A") will return true, but "A".MatchesTag("A.1") will return false
				//EX) "Message.HealthPotion".MatchesTag("Message") will return true, but "Message".MatchesTag("Message.HealthPotion") will return false
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));

				//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, FString::Printf(TEXT("%s"), *Tag.ToString())); //TagName�� ǥ��.

				if (Tag.MatchesTag(MessageTag)) // MessageTag�� ��쿡�� MessageWidgetTable���� Ž��.
				{
					//Lamba�Լ��� �����Լ��� �ٷ� ����� �� ������, ����Լ��� []�ȿ� �ش� ����Լ��� Ŭ���� ĸó�� �ؾ� ��� ����.(this)
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row);
					
					 
				}
			}
		}
	);

}

