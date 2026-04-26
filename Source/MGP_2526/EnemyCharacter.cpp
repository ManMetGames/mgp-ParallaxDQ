#include "EnemyCharacter.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void AEnemyCharacter::TakeComboDamage(float Damage)
{
	// Don't take damage if already dead
	if (bIsDead) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);

	// Tell Blueprint the health changed so it can update a health bar
	OnHealthChanged(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		PlayHitReaction();
	}
}

void AEnemyCharacter::PlayHitReaction()
{
	if (!HitReactionMontage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(HitReactionMontage);
	}
}

void AEnemyCharacter::Die()
{
	bIsDead = true;

	// Play death montage if we have one
	if (DeathMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}

	// Disable collision so the player doesn't collide with a corpse
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Tell Blueprint death happened — it can play effects, sounds etc
	OnDeath();

	// Remove the actor from the world after a delay
	GetWorld()->GetTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AEnemyCharacter::OnDestroyDelayFinished,
		DestroyDelay,
		false
	);
}

void AEnemyCharacter::OnDestroyDelayFinished()
{
	Destroy();
}