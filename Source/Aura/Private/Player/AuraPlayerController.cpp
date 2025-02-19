// Copyright Michał Szcząchor


#include "Player/AuraPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	if(!CursorHit.bBlockingHit) return;

	//Store last actor 
	LastActor = ThisActor;
	//Check if actor has EnemyInterface, if not return nullptr
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	/**
	 * Line trace from cursor. There are several scenarios:
	 * A. LastActor is null && ThisActor is null  (Cursor hover on anything but EnemyInterface)
	 *	- Do nothing
	 * B. LastActor is null && ThisActor is valid  (Cursor hover on ThisActor that is EnemyInterface)
	 *	- Highlight ThisActor
	 * C. LastActor is valid && ThisActor is null  (Cursor hover on EnemyInterface last frame, but this frame not)
	 *	- UnHighlight LastActor
	 * D. Both actors are valid, but LastActor != ThisActor  (Cursor hover on two EnemyInterfaces)
	 *  - UnHighlight LastActor and Highlight ThisActor
	 * E. Both actors are valid and are the same actor  (Cursor hover on the same EnemyInterface this frame as on last)
	 *  - Do nothing
	 */

	if(LastActor == nullptr)
	{
		if(ThisActor != nullptr)
		{
			// Case B
			ThisActor->HighlightActor();
		}
		else
		{
			// Case A
		}
	}
	else // LastActor is valid
	{
		if(ThisActor == nullptr)
		{
			// Case C
			LastActor->UnHighlightActor();
		}
		else // Both actors are valid
		{
			if(LastActor != ThisActor)
			{
				// Case D
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
			else
			{
				// Case E
			}
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//check is going to halt execution if condition fails. Game will crash
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);

	//Adding mapping context to player controller can retrieve data
	Subsystem->AddMappingContext(AuraContext, 0);

	//Mouse settings from PlayerController
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	//Ability to use input in Game as well as in UI (widgets in this game)
	FInputModeGameAndUI InputModeData;
	//Do not lock mouse to viewport
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//As soon as cursor is captured into the viewport, cursor will not hide
	InputModeData.SetHideCursorDuringCapture(false);

	//Use InputModeData
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//if CastChecked fails, game will crash
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	//Bound Move function to Move action
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	//-1 to 1 based on which key player pressed
	const FVector2d InputAxisVector = InputActionValue.Get<FVector2d>();
	const FRotator Rotation = GetControlRotation();
	//zeroing out the pitch and roll 
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	//gives the forward and right normalized vector that corresponds yaw rotation vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if(APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
