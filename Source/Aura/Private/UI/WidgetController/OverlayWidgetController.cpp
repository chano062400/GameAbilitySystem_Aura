

#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{

	const UAuraAttributeSet* AuraAttributSet = CastChecked<UAuraAttributeSet>(AttributeSet); 
	
	//CastCheck�� ��ȿ���� ������ �ߴ��ϹǷ� ���� if���� �� �ʿ�X.
	OnHealthChanged.Broadcast(AuraAttributSet->GetHealth()); // ACCESSOR
	OnMaxHealthChanged.Broadcast(AuraAttributSet->GetMaxHealth()); // ACCESSOR

	OnManaChanged.Broadcast(AuraAttributSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const UAuraAttributeSet* AuraAttributSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	//FOnGameplayAttributeValueChange Delegate�� DYNAMIC�� �ƴ϶� AddUObject�� ���.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributSet->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged); //Health�� ����� ������ HealthChanged�� ȣ���.
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributSet->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged); //MaxHealth�� ����� ������ MaxHealthChanged�� ȣ���.

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributSet->GetManaAttribute()).AddUObject(this, &UOverlayWidgetController::ManaChanged);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributSet->GetMaxManaAttribute()).AddUObject(this, &UOverlayWidgetController::MaxManaChanged);

}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue); //���� ����� ���� Broadcast��.
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue); //���� ����� ���� Broadcast��.
}

void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const
{
	OnManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxManaChanged.Broadcast(Data.NewValue);
}
