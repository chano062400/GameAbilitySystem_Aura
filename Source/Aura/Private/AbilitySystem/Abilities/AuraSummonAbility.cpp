#include "AbilitySystem/Abilities/AuraSummonAbility.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
    const FVector SummonerForwardVector = GetAvatarActorFromActorInfo()->GetActorForwardVector();
    const FVector SummonerLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
    
    // 소환 각도 / 소환될 미니언의 수  - 정해진 각도 안 구역의 수
    const float NumofRotateLocation = SummonSpread / NumofMinions;

    TArray<FVector> SummonLocations;

    const FVector LeftofSpread = SummonerForwardVector.RotateAngleAxis(-SummonSpread / 2.f, FVector::UpVector);
    for (int32 i = 0; i < NumofMinions; i++)
    {
        const FVector Direction = LeftofSpread.RotateAngleAxis(NumofRotateLocation * i, FVector::UpVector);
        //UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SummonerLocation, SummonerLocation + Direction * MaxSummonDistance, 4.f, FLinearColor::Green, 5.f);

        FVector ChosenSummonLocation = SummonerLocation + Direction * FMath::FRandRange(MinSummonDistance, MaxSummonDistance);
        //DrawDebugSphere(GetWorld(), ChosenSummonLocation, 15.f, 12, FColor::Red, false, 5.f);
        SummonLocations.Add(ChosenSummonLocation);

        FHitResult HitResult;
        GetWorld()->LineTraceSingleByChannel(HitResult,
            ChosenSummonLocation + FVector(0.f, 0.f, 400.f),
            ChosenSummonLocation + FVector(0.f, 0.f, -400.f),
            ECollisionChannel::ECC_Visibility
        );

        if (HitResult.bBlockingHit)
        {
            ChosenSummonLocation = HitResult.ImpactPoint; 
        }
    }

    return SummonLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
    int32 RandomIndex = FMath::RandRange(0, MinionClasses.Num() - 1);

    return MinionClasses[RandomIndex];
}
