// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Interaction/EnemyInterface.h"
#include "GameplayTagContainer.h"
#include "Input/AuraInputComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "Components/SplineComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "UI/Widgets/DamageTextComponent.h"
#include "NiagaraFunctionLibrary.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraMappingContext); // AuraMappingContext가 nullptr이면 실행 중단.

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraMappingContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); 
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	
}

void AAuraPlayerController::SetupInputComponent()
{
	APlayerController::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent); // CastChecked에 Assertion과정이 있어서 따로 check 안해줘도 됨.

	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &AAuraPlayerController::AbilityInputTagPressed, &AAuraPlayerController::AbilityInputTagReleased, &AAuraPlayerController::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& Value)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed)) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControllPawn = GetPawn<APawn>())
	{
		ControllPawn->AddMovementInput(Forward, MovementVector.X);
		ControllPawn->AddMovementInput(Right, MovementVector.Y);
	}
}

void AAuraPlayerController::CursorTrace()
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		if(ThisActor) ThisActor->UnHighlightActor();
		if(LastActor) LastActor->UnHighlightActor();
		ThisActor = nullptr;
		LastActor = nullptr;
		return;
	}

	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = TScriptInterface<IEnemyInterface>(CursorHit.GetActor());

	/* 
	   1. LastActor , ThisActor 모두 nullptr - Do Nothing
	   2. LastActor만 nullptr - 해당엑터에 처음 마우스 커서를 갖다댄 상황 / ThisActor->HighlightActor()
	   3. ThisActor만 nullptr - 해당엑터에서 마우스커서가 벗어난 상황 / LastActor->UnHighlightActor()
	   4. LastActor , ThisActor 모두 유효지만 LastActor != ThisActor - 다른 엑터에 마우스 커서를 갖다댄 상황. / LastActor->UnHighlightActor(), ThisActor->HighlightActor()
	   5. LastActor , ThisActor 모두 유효하고 LastActor == ThisActor - 같은 적에서 마우스커서가 맴돌고 있는 상황. / Do Nothing
	 */

	if (LastActor == nullptr)
	{
		if (ThisActor) // 2번 케이스.
		{
			ThisActor->HighlightActor();
		}
	}
	else
	{
		if(ThisActor)
		{
			if(LastActor != ThisActor) // 4번 케이스
			{
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
		}
		else if(ThisActor == nullptr) //3번 케이스.
		{
			LastActor->UnHighlightActor();
		}
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed)) return;

	/*GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, *InputTag.ToString());*/
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = ThisActor ? true : false; // ThisActor가 nullptr이 아니라면 적을 클릭했다는 것.
		bAutoRunning = false;

	}
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased)) return;

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) //왼쪽 마우스 클릭이 아닌 경우
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag); // targeting 했거나, Shift를 누르고 클릭하고 뗐을 때 

	if (!bTargeting && !bShiftKeyPressed) // Targeting 하지 않고 , Shift를 누르지 않았을 때-> 이동 
	{
		const APawn* ControlledPawn = GetPawn();
		if (ControlledPawn && FollowTime <= ShortPressThreshold)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World); // Spline에 목적지 위치를 추가.
					DrawDebugSphere(GetWorld(), PointLoc, 12.f, 12, FColor::Red, false, 5.f);
				}
				if (NavPath->PathPoints.Num() > 0) /*클릭했을 때 지점으로 가지 않고, 먼 곳으로 가는 경우는 NavPath배열에 경로가 없는 경우이다.
													따라서 NavPath배열에 경로가 1개 이상일 경우에만 자동이동을 실행하도록 한다. */
				{
					CachedDestination = NavPath->PathPoints.Last();
					bAutoRunning = true; //Spline에 목적지를 추가했으므로 자동 이동.
				}
			}
			if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld)) return;

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) //왼쪽 마우스 클릭이 아닌 경우
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	if (bTargeting || bShiftKeyPressed) // Enemy에 왼쪽 마우스 클릭한 경우(공격, 스킬) or Shift키를 누르고 왼쪽 마우스 클릭한 경우
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
	}
	else // 이동
	{
		FollowTime += GetWorld()->GetDeltaSeconds();


		if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit))
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - GetPawn()->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}

	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World); //캐릭터와 가장 가까운 Spline의 위치.
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World); // 이동할 방향.
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius) // 허용반경 안에 들어왔으면 자동이동을 멈춤.
		{
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();

	AutoRun();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float Damage, ACharacter* TargetCharacter, bool bIsBlockedHit, bool bIsCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		//수동으로 등록.
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		//생성되자마자 애니메이션이 실행되는데, 계속 Attach상태라면 Enemy를 따라다닐 것이기 때문에, Detach해줘서 맞은 자리에서 애니메이션이 재생돼서 사라지도록.
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(Damage, bIsBlockedHit, bIsCriticalHit);
	}
}
