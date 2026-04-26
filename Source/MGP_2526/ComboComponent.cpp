#include "ComboComponent.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"

UComboComponent::UComboComponent()
{
    // We don't need Tick — everything is timer-driven
    PrimaryComponentTick.bCanEverTick = false;
}

void UComboComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = EComboState::Idle;
}

// -----------------------------------------------------------------------
// PUBLIC: Called by the character every time the player presses attack
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

        // Cannot attack during finisher or cooldown — input is ignored
        case EComboState::Finisher:
        case EComboState::Cooldown:
        default:
            break;
    }
}

// -----------------------------------------------------------------------
// PRIVATE: The single place all state changes happen
// -----------------------------------------------------------------------
void UComboComponent::SetState(EComboState NewState)
{
    const EComboState OldState = CurrentState;
    CurrentState = NewState;

    // Cancel any running timers — each new state sets its own
    GetWorld()->GetTimerManager().ClearTimer(ComboWindowTimer);
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);

    // Broadcast the state change — animation Blueprint and UI listen here
    OnComboStateChanged.Broadcast(OldState, NewState);

    // Handle each state's entry behaviour
    switch (NewState)
    {
        case EComboState::Attack1:
        case EComboState::Attack2:
        case EComboState::Attack3:
        {
            // Apply the hit immediately, then open a window for the next press
            ApplyHit();

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
            // Finisher hits hard then goes into cooldown — no input window
            ApplyHit();
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
            // Nothing to set up — just wait for input
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
// PRIVATE: Broadcast the hit with the correct damage for this stage
// -----------------------------------------------------------------------
void UComboComponent::ApplyHit()
{
    const float Damage = GetDamageForState(CurrentState);
    UE_LOG(LogTemp, Log, TEXT("ComboComponent: Hit at state %d for %.1f damage."),
        (int32)CurrentState, Damage);

    // Broadcast — the character or ability system receives this and applies
    // damage to whatever the player is attacking
    OnComboHit.Broadcast(CurrentState, Damage);
}

// -----------------------------------------------------------------------
// PRIVATE: Camera shake + any other finisher feedback
// -----------------------------------------------------------------------
void UComboComponent::PlayFinisherFeedback()
{
    // Only play camera shake if a class was assigned in Blueprint
    if (!FinisherCameraShake) return;

    // Walk up to the owning player controller
    const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
    if (!PC) return;

    PC->ClientStartCameraShake(FinisherCameraShake);
}

// -----------------------------------------------------------------------
// PRIVATE: Maps a combo state to an entry in the HitDamages array
// -----------------------------------------------------------------------
float UComboComponent::GetDamageForState(EComboState State) const
{
    // Index matches the order: Attack1=0, Attack2=1, Attack3=2, Finisher=3
    const int32 Index = static_cast<int32>(State) - 1;  // Idle=0, so subtract 1

    if (HitDamages.IsValidIndex(Index))
    {
        return HitDamages[Index];
    }

    // Safety fallback — shouldn't happen if HitDamages has 4 entries
    UE_LOG(LogTemp, Warning, TEXT("ComboComponent: No damage configured for state %d"), (int32)State);
    return 0.f;
}