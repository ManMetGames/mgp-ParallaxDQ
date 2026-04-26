#include "ComboWidget.h"

void UComboWidget::InitialiseWithComboComponent(UComboComponent* InComboComponent)
{
	// Safety check — don't bind if we got a null pointer
	if (!InComboComponent) return;

	ComboComponent = InComboComponent;

	// Bind to the delegates we built in ComboComponent
	ComboComponent->OnComboStateChanged.AddDynamic(this, &UComboWidget::OnComboStateChanged);
	ComboComponent->OnComboHit.AddDynamic(this, &UComboWidget::OnComboHit);
}

void UComboWidget::OnComboStateChanged(EComboState OldState, EComboState NewState)
{
	// Reset the counter when returning to Idle
	if (NewState == EComboState::Idle)
	{
		ComboCount = 0;
		ComboStateText = "Idle";
	}
	else if (NewState == EComboState::Attack1) { ComboStateText = "Attack 1"; }
	else if (NewState == EComboState::Attack2) { ComboStateText = "Attack 2"; }
	else if (NewState == EComboState::Attack3) { ComboStateText = "Attack 3"; }
	else if (NewState == EComboState::Finisher) { ComboStateText = "FINISHER!"; }
	else if (NewState == EComboState::Cooldown) { ComboStateText = "Cooldown"; }

	// Tell Blueprint the state changed so it can play animations
	OnStateUpdated();
}

void UComboWidget::OnComboHit(EComboState HitStage, float Damage)
{
	// Increment the counter on every hit
	ComboCount++;

	// Tell Blueprint a hit landed so it can flash the counter
	OnHitRegistered();
}