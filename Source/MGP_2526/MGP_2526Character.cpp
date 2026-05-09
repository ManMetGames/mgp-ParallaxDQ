// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGP_2526Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "EnemyCharacter.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MGP_2526.h"

AMGP_2526Character::AMGP_2526Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	// Create the combo component
	ComboComponent = CreateDefaultSubobject<UComboComponent>(TEXT("ComboComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMGP_2526Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Look);
		
		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AMGP_2526Character::Attack);
	}
	else
	{
		UE_LOG(LogMGP_2526, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMGP_2526Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMGP_2526Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMGP_2526Character::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMGP_2526Character::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMGP_2526Character::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AMGP_2526Character::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AMGP_2526Character::Attack()
{
	if (!ComboComponent) return;
	
	// Prevent attacking while airborne
	if (!GetCharacterMovement()->IsMovingOnGround()) return;

	// Advance the combo state and play the animation
	ComboComponent->AttemptAttack();
	PlayAttackMontage(ComboComponent->GetCurrentState());
}

void AMGP_2526Character::PlayAttackMontage(EComboState State)
{
	UAnimMontage* MontageToPlay = nullptr;

	switch (State)
	{
	case EComboState::Attack1:  MontageToPlay = Attack1Montage;  break;
	case EComboState::Attack2:  MontageToPlay = Attack2Montage;  break;
	case EComboState::Attack3:  MontageToPlay = Attack3Montage;  break;
	case EComboState::Finisher: MontageToPlay = FinisherMontage; break;
	default: return;
	}

	if (!MontageToPlay) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(MontageToPlay, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
	}
}

void AMGP_2526Character::OnAttackHitNotify()
{
	if (!ComboComponent) return;
	
	// Notify the combo component a hit landed so the UI counter updates
	ComboComponent->OnComboHit.Broadcast(ComboComponent->GetCurrentState(), ComboComponent->GetCurrentDamage());

	// Sphere sweep — wider hit detection than a single line trace
	const FVector Start = GetActorLocation();
	const FVector End = Start + (GetActorForwardVector() * 150.f);
	const float SweepRadius = 40.f; // adjust this to taste

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(SweepRadius),
		Params
	);

	if (bHit)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitResult.GetActor());
		if (Enemy)
		{
			const float Damage = ComboComponent->GetCurrentDamage();
			Enemy->TakeComboDamage(Damage);

			// Hit stop — briefly freezes time dilation to sell the impact
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.05f);

			// Restore normal speed after a short delay
			FTimerHandle HitStopTimer;
			GetWorld()->GetTimerManager().SetTimer(
				HitStopTimer,
				[this]()
				{
					// SetGlobalTimeDilation is restored after a short real-time delay
					UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
				},
				0.01f,  // real time seconds — tweak this to taste
				false
			);
		}
	}
}