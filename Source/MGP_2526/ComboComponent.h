#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComboComponent.generated.h"

// -----------------------------------------------------------------------
// Enum: All possible states in the combo FSM
// -----------------------------------------------------------------------
UENUM(BlueprintType)
enum class EComboState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Attack1     UMETA(DisplayName = "Attack 1"),
    Attack2     UMETA(DisplayName = "Attack 2"),
    Attack3     UMETA(DisplayName = "Attack 3"),
    Finisher    UMETA(DisplayName = "Finisher"),
    Cooldown    UMETA(DisplayName = "Cooldown")
};

// -----------------------------------------------------------------------
// Delegate: Broadcasts whenever the combo state changes.
// Other systems (UI, animation, sound) can listen without coupling to this class.
// -----------------------------------------------------------------------
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnComboStateChanged,
    EComboState, OldState,
    EComboState, NewState
);

// -----------------------------------------------------------------------
// Delegate: Broadcasts when a hit should be applied.
// The character or an ability system reads Damage and applies it to a target.
// -----------------------------------------------------------------------
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnComboHit,
    EComboState, HitStage,
    float,        Damage
);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MGP_2526_API UComboComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UComboComponent();

    // -----------------------------------------------------------------------
    // UPROPERTIES — exposed to Blueprint for easy tuning without recompiling
    // -----------------------------------------------------------------------

    // How long (seconds) the player has after a hit to continue the combo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Timing")
    float ComboWindowDuration = 0.8f;

    // How long the Cooldown state lasts before returning to Idle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Timing")
    float CooldownDuration = 0.4f;

    // Damage values for Attack1, Attack2, Attack3, Finisher (index 0-3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Damage")
    TArray<float> HitDamages = { 10.f, 15.f, 20.f, 50.f };

    // Camera shake class to play on Finisher (assign in Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Feedback")
    TSubclassOf<UCameraShakeBase> FinisherCameraShake;

    // -----------------------------------------------------------------------
    // EVENTS — Blueprint and other C++ systems can bind to these
    // -----------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Combo|Events")
    FOnComboStateChanged OnComboStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combo|Events")
    FOnComboHit OnComboHit;

    // -----------------------------------------------------------------------
    // PUBLIC INTERFACE
    // -----------------------------------------------------------------------

    // Call this from your character's input binding each time attack is pressed
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void AttemptAttack();

    // Read-only access to current state (useful for animation Blueprint)
    UFUNCTION(BlueprintPure, Category = "Combo")
    EComboState GetCurrentState() const { return CurrentState; }
    
    // Returns the damage value for the current combo stage — used by the line trace
    UFUNCTION(BlueprintPure, Category = "Combo")
    float GetCurrentDamage() const { return CurrentAttackDamage; }

protected:
    virtual void BeginPlay() override;

private:
    // -----------------------------------------------------------------------
    // INTERNAL STATE
    // -----------------------------------------------------------------------

    EComboState CurrentState = EComboState::Idle;
    
    // Cached damage for the current attack — set when state changes
    float CurrentAttackDamage = 0.f;

    // Handle for the combo window timer — stored so we can clear it on reset
    FTimerHandle ComboWindowTimer;

    // Handle for the cooldown timer
    FTimerHandle CooldownTimer;

    // -----------------------------------------------------------------------
    // INTERNAL METHODS
    // -----------------------------------------------------------------------

    // Central state transition — all changes go through here
    void SetState(EComboState NewState);

    // Triggered when the combo window expires without another press
    void OnComboWindowExpired();

    // Triggered when cooldown ends — returns to Idle
    void OnCooldownFinished();

    // Applies the hit for the current state (broadcasts OnComboHit)
    void ApplyHit();

    // Plays finisher feedback (camera shake, etc.)
    void PlayFinisherFeedback();

    // Returns the damage for the current hit stage
    float GetDamageForState(EComboState State) const;
};