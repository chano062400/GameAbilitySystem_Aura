#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/SaveInterface.h"
#include "CheckPoint.generated.h"

class USphereComponent;

/**
 * 
 */
UCLASS()
class AURA_API ACheckPoint : public APlayerStart, public ISaveInterface
{
	GENERATED_BODY()

public:

	ACheckPoint(const FObjectInitializer& ObjectInitializer);

	virtual bool ShouldLoadTransform_Implementation() override { return false; };

	virtual void LoadActor_Implementation() override;

	// SaveGame Specifier - Serialize«ÿ¡‹.
	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bReached = false;

protected:

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OverlapActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromsweep, const FHitResult& HitResult);

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void CheckPointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	void HandleGlowEffect();

private:
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CheckPointMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
