#include "Actor/AuraEffectActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(Mesh);

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(RootComponent);
}

void AAuraEffectActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		//Gameplay Effect를 아직 배우지 않아서 억지로 const_cast후 값을 변경.
		if (const UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(Interface->GetAbilitySystemComponent()->GetAttributeSet(UAttributeSet::StaticClass()))) // const값이라 원래는 변경 못함.
		{
			UAuraAttributeSet* MutableAAS= const_cast<UAuraAttributeSet*>(AuraAttributeSet); 
			MutableAAS->SetHealth(AuraAttributeSet->GetHealth() + 50.f); 
			MutableAAS->SetMana(AuraAttributeSet->GetMana() - 25.f);
			Destroy();
		}

	}
}

void AAuraEffectActor::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraEffectActor::OnOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AAuraEffectActor::EndOverlap);
}


