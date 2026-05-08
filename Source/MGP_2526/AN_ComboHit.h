#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ComboHit.generated.h"

/**
 * Animation Notify that fires at the moment of impact in an attack montage.
 * Triggers the line trace and damage application on the owning character,
 * ensuring hits only register at the correct frame of the animation.
 */
UCLASS()
class MGP_2526_API UAN_ComboHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	// This fires at the exact frame the notify is placed in the montage
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};