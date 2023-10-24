// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
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

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent); // CastChecked에 Assertion과정이 있어서 따로 check 안해줘도 됨.

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& Value)
{
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
	FHitResult CursorHit;
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

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}
