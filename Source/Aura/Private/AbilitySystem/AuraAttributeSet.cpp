#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Gameframework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	InitHealth(50.f);
	InitMaxHealth(100.f);
	InitMana(25.f);
	InitMaxMana(50.f);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None ,REPNOTIFY_Always); //���� ������� �ʾƵ� �׻� �˸�.
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("Health : %f"), NewValue);
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
		UE_LOG(LogTemp, Warning, TEXT("After Clamping Health : %f"), NewValue);
	}

	
	if (Attribute == GetManaAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("Mana : %f"), NewValue);
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
		UE_LOG(LogTemp, Warning, TEXT("After Clamping Mana : %f"), NewValue);
	}

}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties EffectProperties;
	
	SetEffectProperties(Data, EffectProperties);

}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& PrevHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, PrevHealth); //GAS�� Health�� Replication ������ �˰Ե�.
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& PrevMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, PrevMaxHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& PrevMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, PrevMana);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& PrevMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, PrevMaxMana);
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	//Source = Effect�� Causer , Target = Effect�� Target(AttributeSet�� Owner)

	Props.EffectContextHandle = Data.EffectSpec.GetContext();

	Props.SourceABSC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent(); //Gameplay Effect�� ������ AbilitySystem��ȯ.

	if (IsValid(Props.SourceABSC) && Props.SourceABSC->AbilityActorInfo.IsValid() && Props.SourceABSC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceABSC->AbilityActorInfo->AvatarActor.Get();
		const AController* SourceController = Props.SourceABSC->AbilityActorInfo->PlayerController.Get();

		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr) // Controller�� ������ �ȵ��ִٸ� ����.
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}

		if (Props.SourceController != nullptr)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn()); // AbilitySystem�� ������ Character.
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();

		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();

		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);

		Props.TargetABSC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}

}
