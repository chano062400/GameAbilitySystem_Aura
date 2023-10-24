

#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{

	const UAuraAttributeSet* AuraAttributSet = CastChecked<UAuraAttributeSet>(AttributeSet); 
	
	//CastCheck는 유효하지 않으면 중단하므로 따로 if문을 쓸 필요X.
	OnHealthChanged.Broadcast(AuraAttributSet->GetHealth()); // ACCESSOR
	OnMaxHealthChanged.Broadcast(AuraAttributSet->GetMaxHealth()); // ACCESSOR

	OnManaChanged.Broadcast(AuraAttributSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const UAuraAttributeSet* AuraAttributSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	//FOnGameplayAttributeValueChange Delegate가 DYNAMIC이 아니라서 AddUObject를 사용.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributSet->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged); //Health가 변경될 때마다 HealthChanged가 호출됨.
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributSet->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged); //MaxHealth가 변경될 때마다 MaxHealthChanged가 호출됨.

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributSet->GetManaAttribute()).AddUObject(this, &UOverlayWidgetController::ManaChanged);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributSet->GetMaxManaAttribute()).AddUObject(this, &UOverlayWidgetController::MaxManaChanged);

}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue); //새로 변경된 값을 Broadcast함.
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue); //새로 변경된 값을 Broadcast함.
}

void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const
{
	OnManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxManaChanged.Broadcast(Data.NewValue);
}
