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
		FGameplayEffectSpecHandle EffectSpecHandle = TargetABSC->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContextHandle); 
		TargetABSC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()); // Data는 스마트포인터. Get()을 통해 raw Pointer로 가져옴 / 매개변수가 참조형식이라 값을 가져와서 넣어야 함(*EffectSpecHandle.Data.Get())
	}
}





