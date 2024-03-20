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

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent); // CastChecked�� Assertion������ �־ ���� check �����൵ ��.

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

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed)) return;

	/*GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, *InputTag.ToString());*/
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = ThisActor ? true : false; // ThisActor�� nullptr�� �ƴ϶�� ���� Ŭ���ߴٴ� ��.
		bAutoRunning = false;

	}
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased)) return;

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) //���� ���콺 Ŭ���� �ƴ� ���
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag); // targeting �߰ų�, Shift�� ������ Ŭ���ϰ� ���� �� 

	if (!bTargeting && !bShiftKeyPressed) // Targeting ���� �ʰ� , Shift�� ������ �ʾ��� ��-> �̵� 
	{
		const APawn* ControlledPawn = GetPawn();
		if (ControlledPawn && FollowTime <= ShortPressThreshold)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World); // Spline�� ������ ��ġ�� �߰�.
					DrawDebugSphere(GetWorld(), PointLoc, 12.f, 12, FColor::Red, false, 5.f);
				}
				if (NavPath->PathPoints.Num() > 0) /*Ŭ������ �� �������� ���� �ʰ�, �� ������ ���� ���� NavPath�迭�� ��ΰ� ���� ����̴�.
													���� NavPath�迭�� ��ΰ� 1�� �̻��� ��쿡�� �ڵ��̵��� �����ϵ��� �Ѵ�. */
				{
					CachedDestination = NavPath->PathPoints.Last();
					bAutoRunning = true; //Spline�� �������� �߰������Ƿ� �ڵ� �̵�.
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

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) //���� ���콺 Ŭ���� �ƴ� ���
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	if (bTargeting || bShiftKeyPressed) // Enemy�� ���� ���콺 Ŭ���� ���(����, ��ų) or ShiftŰ�� ������ ���� ���콺 Ŭ���� ���
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
	}
	else // �̵�
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
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World); //ĳ���Ϳ� ���� ����� Spline�� ��ġ.
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World); // �̵��� ����.
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius) // ���ݰ� �ȿ� �������� �ڵ��̵��� ����.
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
		//�������� ���.
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		//�������ڸ��� �ִϸ��̼��� ����Ǵµ�, ��� Attach���¶�� Enemy�� ����ٴ� ���̱� ������, Detach���༭ ���� �ڸ����� �ִϸ��̼��� ����ż� ���������.
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(Damage, bIsBlockedHit, bIsCriticalHit);
	}
}
