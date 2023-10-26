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
		FGameplayEffectSpecHandle EffectSpecHandle = TargetABSC->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContextHandle); 
		TargetABSC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()); // Data�� ����Ʈ������. Get()�� ���� raw Pointer�� ������ / �Ű������� ���������̶� ���� �����ͼ� �־�� ��(*EffectSpecHandle.Data.Get())
	}
}





