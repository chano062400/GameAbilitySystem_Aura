

#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UOverlayWidgetController::BroadcastInitialValues()
{

	const UAuraAttributeSet* AuraAttributSet = CastChecked<UAuraAttributeSet>(AttributeSet); 
	
	//CastCheck는 유효하지 않으면 중단하므로 따로 if문을 쓸 필요X.
	OnHealthChanged.Broadcast(AuraAttributSet->GetHealth()); // ACCESSOR
	OnMaxHealthChanged.Broadcast(AuraAttributSet->GetMaxHealth()); // ACCESSOR

	OnManaChanged.Broadcast(AuraAttributSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies() // Attribute가 변경되었을때.
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	//FOnGameplayAttributeValueChange Delegate가 DYNAMIC이 아니라서 AddUObject를 사용.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetHealthAttribute()).AddLambda(//Health가 변경될 때마다 HealthChanged가 호출됨.
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
			);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetMaxHealthAttribute()).AddLambda( //MaxHealth가 변경될 때마다 MaxHealthChanged가 호출됨.
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

	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda( //AddLamda - 즉석에서 함수를 정의.
		[this](const FGameplayTagContainer& AssetTags /*매개변수*/)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				//"A.1".MatchesTag("A") will return true, but "A".MatchesTag("A.1") will return false
				//EX) "Message.HealthPotion".MatchesTag("Message") will return true, but "Message".MatchesTag("Message.HealthPotion") will return false
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));

				//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, FString::Printf(TEXT("%s"), *Tag.ToString())); //TagName을 표시.

				if (Tag.MatchesTag(MessageTag)) // MessageTag인 경우에만 MessageWidgetTable에서 탐색.
				{
					//Lamba함수는 전역함수는 바로 사용할 수 있지만, 멤버함수는 []안에 해당 멤버함수의 클래스 캡처를 해야 사용 가능.(this)
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row);
					
					 
				}
			}
		}
	);

}

