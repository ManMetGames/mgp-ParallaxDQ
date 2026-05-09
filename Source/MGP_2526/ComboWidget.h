#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ComboComponent.h"
#include "ComboWidget.generated.h"

UCLASS()
class MGP_2526_API UComboWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// Call this once when the widget is created, passing in the combo component
	UFUNCTION(BlueprintCallable, Category="Combo UI")
	void InitialiseWithComboComponent(UComboComponent* InComboComponent);

protected:

	// The combo component this widget is watching
	UPROPERTY()
	UComboComponent* ComboComponent;

	// Running count of hits in the current combo
	UPROPERTY(BlueprintReadOnly, Category="Combo UI")
	int32 ComboCount = 0;

	// The current state as a readable string — Blueprint uses this to drive text
	UPROPERTY(BlueprintReadOnly, Category="Combo UI")
	FString ComboStateText = "Idle";

	// Called by the delegate when the combo state changes
	UFUNCTION()
	void OnComboStateChanged(EComboState OldState, EComboState NewState);

	// Called by the delegate when a hit lands
	UFUNCTION()
	void OnComboHit(EComboState HitStage, float Damage);

	// Blueprint can override this to play animations when the state changes
	UFUNCTION(BlueprintImplementableEvent, Category="Combo UI")
	void OnStateUpdated();

	// Blueprint can override this to flash the counter on a hit
	UFUNCTION(BlueprintImplementableEvent, Category="Combo UI")
	void OnHitRegistered();
};