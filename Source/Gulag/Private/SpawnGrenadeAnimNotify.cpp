// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnGrenadeAnimNotify.h"
#include "GulagCharacter.h"

void USpawnGrenadeAnimNotify::Notify(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation)
{

    GEngine->AddOnScreenDebugMessage(
        -1,
        5.f,
        FColor::Green,
        TEXT("SpawnGrenadeAnimNotify Triggered")
	);

    if (!MeshComp) return;

    AGulagCharacter* Character =
        Cast<AGulagCharacter>(MeshComp->GetOwner());

    if (!Character)
        return;

  

    //get socket transform on the anim's mesh
    const FName SocketName = TEXT("GrenadeSocket"); // 
    const FTransform SocketTransform = MeshComp->GetSocketTransform(SocketName, RTS_World);
    const FVector SpawnLocation = SocketTransform.GetLocation();
    const FVector AimDirection = SocketTransform.GetRotation().GetForwardVector();
    
    Character->Server_SpawnGrenadeFromAnimNotify(SocketTransform, AimDirection);
}




