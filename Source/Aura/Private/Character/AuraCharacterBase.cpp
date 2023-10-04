#include "Character/AuraCharacterBase.h"
#include "DrawDebugHelpers.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAuraCharacterBase::HighlightActor()
{
	
}

void AAuraCharacterBase::UnHighlightActor()
{

}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}
