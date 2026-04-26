#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ComboHit.generated.h"

UCLASS()
class MGP_2526_API UAN_ComboHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	// This fires at the exact frame the notify is placed in the montage
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};