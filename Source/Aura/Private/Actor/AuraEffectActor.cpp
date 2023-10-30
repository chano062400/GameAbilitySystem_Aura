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
		//UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target); // �ѹ��� ������ ���� ����.
		if (TargetABSC == nullptr) return;

		check(GameplayEffectClass);
		FGameplayEffectContextHandle EffectContextHandle = TargetABSC->MakeEffectContext(); //FGameplayEffectContextHandle - FGameplayEffectContext�� �����ͷ� �����ϴ� Light Wrapper.
		
		EffectContextHandle.AddSourceObject(this); // Gameplay Context���� �׸��� �����ϴ� ���.  �� ��ü�� ���� ȿ���� �����״��� SourceObject�� ���� Ȯ�� ��������.
		
		// SpecHandle������ ���� Spec���� �������� ������ ���� Ȯ���غ� ��, �Ȱ��� GameplayEffectSpec�� �����ϴ� LightWrapper��.
		FGameplayEffectSpecHandle EffectSpecHandle = TargetABSC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle); 
		const FActiveGameplayEffectHandle ActiveEffectHandle = TargetABSC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()); // Data�� ����Ʈ������. Get()�� ���� raw Pointer�� ������ / �Ű������� ���������̶� ���� �����ͼ� �־�� ��(*EffectSpecHandle.Data.Get())
	
		// EffectSpecHandle.Data.Get()->Def - GameplayEffect �� ��ü�� ����. .Get() - ���������͸� �޾ƿ�.
		
		//Infinite Effect�� �����ϱ� ���� �ʿ�.
		const bool bInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite; // Infinite��� true
		if (bInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)  // Do Not Removal�̶�� ���� ���� �ʴ� ȿ���̱� ������ �ʿ�X
		{
			ActiveEffectHandles.Add(ActiveEffectHandle, TargetABSC); // TMap�� FActiveGameplayEffectHandle�� AbilitySystemComponent�� ����.

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

		// TargetActor�� InfiniteEffect ȿ���� �ް� �ִٸ�
		TArray<FActiveGameplayEffectHandle> HandlesToRemove; // for���� �� ���� �����ϱ� ���� ����.

		for (auto HandlePair : ActiveEffectHandles) 
		{
			if (TargetABSC == HandlePair.Value)
			{
				TargetABSC->RemoveActiveGameplayEffect(HandlePair.Key ,1); // ������ GameplayEffect�� ������ ���� ��(-1�̶�� ��ü ���� ����).
				HandlesToRemove.Add(HandlePair.Key);

			}
		}

		//�����س��� ActvieGameplayEffectHandle�� TMap���� ����.
		for (auto& Handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}





