#include "Actor/AuraEffectActor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
	
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();

}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	TScriptInterface<IAbilitySystemInterface> ASCInterface = TScriptInterface<IAbilitySystemInterface>(TargetActor);
	if (ASCInterface)
	{
		UAbilitySystemComponent* TargetABSC = ASCInterface->GetAbilitySystemComponent();
		//UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target); // 한번에 가져올 수도 있음.
		if (TargetABSC == nullptr) return;

		check(GameplayEffectClass);
		FGameplayEffectContextHandle EffectContextHandle = TargetABSC->MakeEffectContext(); //FGameplayEffectContextHandle - FGameplayEffectContext를 포인터로 저장하는 Light Wrapper.
		
		EffectContextHandle.AddSourceObject(this); // Gameplay Context관련 항목을 저장하는 기능.  이 객체가 무슨 효과를 일으켰는지 SourceObject를 통해 확인 가능해짐.
		
		// SpecHandle이지만 보통 Spec으로 변수명을 지으니 유형 확인해볼 것, 똑같이 GameplayEffectSpec을 저장하는 LightWrapper임.
		FGameplayEffectSpecHandle EffectSpecHandle = TargetABSC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle); 
		const FActiveGameplayEffectHandle ActiveEffectHandle = TargetABSC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()); // Data는 스마트포인터. Get()을 통해 raw Pointer로 가져옴 / 매개변수가 참조형식이라 값을 가져와서 넣어야 함(*EffectSpecHandle.Data.Get())
	
		// EffectSpecHandle.Data.Get()->Def - GameplayEffect 그 자체를 뜻함. .Get() - 원시포인터를 받아옴.
		
		//Infinite Effect를 제거하기 위해 필요.
		const bool bInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite; // Infinite라면 true
		if (bInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)  // Do Not Removal이라면 제거 하지 않는 효과이기 때문에 필요X
		{
			ActiveEffectHandles.Add(ActiveEffectHandle, TargetABSC); // TMap에 FActiveGameplayEffectHandle과 AbilitySystemComponent를 저장.

		}
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::EndOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
 	ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}

	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetABSC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor); 
		if (TargetABSC == nullptr) return;

		// TargetActor가 InfiniteEffect 효과를 받고 있다면
		TArray<FActiveGameplayEffectHandle> HandlesToRemove; // for문을 다 돌고서 제거하기 위해 저장.

		for (auto HandlePair : ActiveEffectHandles) 
		{
			if (TargetABSC == HandlePair.Value)
			{
				TargetABSC->RemoveActiveGameplayEffect(HandlePair.Key ,1); // 제거할 GameplayEffect와 제거할 스택 수(-1이라면 전체 스택 삭제).
				HandlesToRemove.Add(HandlePair.Key);

			}
		}

		//저장해놓은 ActvieGameplayEffectHandle을 TMap에서 제거.
		for (auto& Handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}





