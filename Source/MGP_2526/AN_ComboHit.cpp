#include "AN_ComboHit.h"
#include "MGP_2526Character.h"

void UAN_ComboHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	// Walk up to the owning character
	AMGP_2526Character* Character = Cast<AMGP_2526Character>(MeshComp->GetOwner());
	if (!Character) return;

	// Tell the character to do the hit check now
	Character->OnAttackHitNotify();
}