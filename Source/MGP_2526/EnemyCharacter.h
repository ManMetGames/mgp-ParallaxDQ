#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

/**
 * A basic enemy character that receives damage from the player's combo system.
 * Plays hit reaction and death montages, and broadcasts health changes to Blueprint
 * for UI updates via the OnHealthChanged event.
 */
UCLASS()
class MGP_2526_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	// Maximum health of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Health")
	float MaxHealth = 100.f;

	// Current health — Blueprint can read this to drive a health bar
	UPROPERTY(BlueprintReadOnly, Category="Enemy|Health")
	float CurrentHealth = 100.f;

	// Animation montage to play when hit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Animations")
	UAnimMontage* HitReactionMontage;

	// Animation montage to play on death
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Animations")
	UAnimMontage* DeathMontage;

	// How long after death before the actor is removed from the world
	UPROPERTY(EditAnywhere, Category="Enemy|Health")
	float DestroyDelay = 2.f;

	// Whether the enemy has already died — prevents double death
	bool bIsDead = false;

	// Plays the hit reaction montage
	void PlayHitReaction();

	// Handles death — plays montage and destroys actor
	void Die();

	// Called by the timer to remove the actor after death
	void OnDestroyDelayFinished();

	FTimerHandle DestroyTimer;

public:
	// Called by the combo component via the OnComboHit delegate
	UFUNCTION(BlueprintCallable, Category="Enemy|Health")
	void TakeComboDamage(float Damage);

	// Blueprint event — fires when health changes so UI can react
	UFUNCTION(BlueprintImplementableEvent, Category="Enemy|Health")
	void OnHealthChanged(float NewHealth, float MaxHealthValue);

	// Blueprint event — fires on death
	UFUNCTION(BlueprintImplementableEvent, Category="Enemy|Health")
	void OnDeath();
};