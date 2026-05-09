#include "ComboComponent.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"

UComboComponent::UComboComponent()
{
    // Everything is timer-driven
    PrimaryComponentTick.bCanEverTick = false;
}

void UComboComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = EComboState::Idle;
}

// -----------------------------------------------------------------------
// PUBLIC: Called by the character every time the player attacks
// -----------------------------------------------------------------------
void UComboComponent::AttemptAttack()
{
    switch (CurrentState)
    {
        case EComboState::Idle:
            SetState(EComboState::Attack1);
            break;

        case EComboState::Attack1:
            SetState(EComboState::Attack2);
            break;

        case EComboState::Attack2:
            SetState(EComboState::Attack3);
            break;

        case EComboState::Attack3:
            SetState(EComboState::Finisher);
            break;

        // Can't attack during finisher or cooldown — input is ignored
        case EComboState::Finisher:
        case EComboState::Cooldown:
        default:
            break;
    }
}

// -----------------------------------------------------------------------
// PRIVATE: State changes
// -----------------------------------------------------------------------
void UComboComponent::SetState(EComboState NewState)
{
    const EComboState OldState = CurrentState;
    CurrentState = NewState;
    
    // Cache the damage for this state immediately before timers fire
    if (NewState != EComboState::Idle && NewState != EComboState::Cooldown)
    {
        CurrentAttackDamage = GetDamageForState(NewState);
    }

    // Cancel any running timers — each new state sets its own
    GetWorld()->GetTimerManager().ClearTimer(ComboWindowTimer);
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);

    // Broadcast the state change
    OnComboStateChanged.Broadcast(OldState, NewState);

    // Handle each state's entry behaviour
    switch (NewState)
    {
        case EComboState::Attack1:
        case EComboState::Attack2:
        case EComboState::Attack3:
        {
            GetWorld()->GetTimerManager().SetTimer(
                ComboWindowTimer,
                this,
                &UComboComponent::OnComboWindowExpired,
                ComboWindowDuration,
                false  // don't loop
            );
            break;
        }

        case EComboState::Finisher:
        {
            PlayFinisherFeedback();

            GetWorld()->GetTimerManager().SetTimer(
                CooldownTimer,
                this,
                &UComboComponent::OnCooldownFinished,
                CooldownDuration,
                false
            );
            break;
        }

        case EComboState::Cooldown:
        {
            GetWorld()->GetTimerManager().SetTimer(
                CooldownTimer,
                this,
                &UComboComponent::OnCooldownFinished,
                CooldownDuration,
                false
            );
            break;
        }

        case EComboState::Idle:
        default:
            // Nothing to set up — wait for input
            break;
    }
}

// -----------------------------------------------------------------------
// PRIVATE: Player didn't press attack in time — reset to Idle
// -----------------------------------------------------------------------
void UComboComponent::OnComboWindowExpired()
{
    UE_LOG(LogTemp, Log, TEXT("ComboComponent: Window expired, resetting to Idle."));
    SetState(EComboState::Idle);
}

// -----------------------------------------------------------------------
// PRIVATE: Cooldown finished — player can attack again
// -----------------------------------------------------------------------
void UComboComponent::OnCooldownFinished()
{
    SetState(EComboState::Idle);
}

// -----------------------------------------------------------------------
// PRIVATE: Camera shake + any other finisher feedback
// -----------------------------------------------------------------------
void UComboComponent::PlayFinisherFeedback()
{
    if (!FinisherCameraShake) return;

    // Delay the shake slightly so it hits at the point of impact
    FTimerHandle ShakeTimer;
    GetWorld()->GetTimerManager().SetTimer(
        ShakeTimer,
        [this]()
        {
            const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
            if (!OwnerChar) return;

            APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
            if (!PC) return;

            PC->ClientStartCameraShake(FinisherCameraShake);
        },
        0.85f,  // delay in seconds — adjust to change duration
        false
    );
}

// -----------------------------------------------------------------------
// PRIVATE: Maps a combo state to an entry in the HitDamages array
// -----------------------------------------------------------------------
float UComboComponent::GetDamageForState(EComboState State) const
{
    // Index matches the order: Attack1=0, Attack2=1, etc
    const int32 Index = static_cast<int32>(State) - 1;  // Idle=0, so subtract 1

    if (HitDamages.IsValidIndex(Index))
    {
        return HitDamages[Index];
    }

    // Safety fallback
    UE_LOG(LogTemp, Warning, TEXT("ComboComponent: No damage configured for state %d"), (int32)State);
    return 0.f;
}