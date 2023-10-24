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

	check(AuraMappingContext); // AuraMappingContext�� nullptr�̸� ���� �ߴ�.

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

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent); // CastChecked�� Assertion������ �־ ���� check �����൵ ��.

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
	   1. LastActor , ThisActor ��� nullptr - Do Nothing
	   2. LastActor�� nullptr - �ش翢�Ϳ� ó�� ���콺 Ŀ���� ���ٴ� ��Ȳ / ThisActor->HighlightActor()
	   3. ThisActor�� nullptr - �ش翢�Ϳ��� ���콺Ŀ���� ��� ��Ȳ / LastActor->UnHighlightActor()
	   4. LastActor , ThisActor ��� ��ȿ���� LastActor != ThisActor - �ٸ� ���Ϳ� ���콺 Ŀ���� ���ٴ� ��Ȳ. / LastActor->UnHighlightActor(), ThisActor->HighlightActor()
	   5. LastActor , ThisActor ��� ��ȿ�ϰ� LastActor == ThisActor - ���� ������ ���콺Ŀ���� �ɵ��� �ִ� ��Ȳ. / Do Nothing
	 */

	if (LastActor == nullptr)
	{
		if (ThisActor) // 2�� ���̽�.
		{
			ThisActor->HighlightActor();
		}
	}
	else
	{
		if(ThisActor)
		{
			if(LastActor != ThisActor) // 4�� ���̽�
			{
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
		}
		else if(ThisActor == nullptr) //3�� ���̽�.
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
