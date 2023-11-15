
#include "AI/BTService_FindNearestPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BTFunctionLibrary.h"

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* OwningPawn = AIOwner->GetPawn();

	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player"); // Enemy는 Enemy Tag를 갖고 있으므로 Player가 됨. / Player만 Player태그를 갖고 있음.

	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag); // Player라는 Tag를 가진 Actor를 ActosWithTag에 넣어서 반환.

	float ClosestDistance = TNumericLimits<float>::Max(); // 원하는 형식의 최대 값을 가져옴.
	AActor* ClosestActor = nullptr;
	for (auto Actor : ActorsWithTag)
	{
		GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *Actor->GetName());

		if (IsValid(Actor) && IsValid(OwningPawn))
		{
			const float Distance = OwningPawn->GetDistanceTo(Actor);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = Actor;
			}
		}
	}

	UBTFunctionLibrary::SetBlackboardValueAsObject(this, TargetToFollowSelector, ClosestActor); // 블랙보드의 변수 값을 Object형식 ClosestActor로 설정.
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance); 
}
