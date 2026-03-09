// Fill out your copyright notice in the Description page of Project Settings.


#include "RevealGrenade.h"
#include "GulagCharacter.h"
#include "MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ARevealGrenade::Explode()
{

    if (!HasAuthority())
        return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Reveal Exploding"));

	FVector ExplosionLocation = GetActorLocation();

    //start visual pulse everywhere
    Multicast_PlayPulseVFX(ExplosionLocation);

    //debug - show overlap sphere
    DrawDebugSphere(
        GetWorld(),
        ExplosionLocation,
        RevealRadius,
        32,                 // segments
        FColor::Green,      // color
        false,              // persistent
        2.0f,               // lifetime
        0,
        2.0f                // line thickness
    );

    
    PerformRevealScan();

    //Destroy();

}

void ARevealGrenade::PerformRevealScan()
{
    TArray<AActor*> OverlappedActors;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        RevealRadius,
        ObjectTypes,
        AGulagCharacter::StaticClass(),
        IgnoreActors,
        OverlappedActors
    );

    //Cache instigator once
    AController* InstigatorController = GetInstigatorController();

    for (AActor* Actor : OverlappedActors)
    {
        AGulagCharacter* HitChar = Cast<AGulagCharacter>(Actor);
        if (!HitChar)
        {
            continue;
        }
        //AController* InstigatorController = GetInstigatorController();
		//HitChar->Client_UpdateRevealedUI(true, RevealDuration);

        if (InstigatorController)
        {
            APawn* InstigatorPawn = InstigatorController->GetPawn();
            if (InstigatorPawn && InstigatorPawn == HitChar)
            {
                continue;
            }
            else {
                HitChar->Client_UpdateRevealedUI(true, RevealDuration);
            }
        }

        if (!InstigatorController)
            continue;

        AMyPlayerController* PC = Cast<AMyPlayerController>(InstigatorController);
        if (!PC)
            continue;

        PC->Client_RevealActor(HitChar, RevealDuration);
        //HitChar->Client_ApplyReveal(RevealDuration);
    }
    Multicast_StopTrailVFX();
}

void ARevealGrenade::Multicast_StopTrailVFX_Implementation()
{
	BP_StopTrailVFX();
}

void ARevealGrenade::Multicast_PlayPulseVFX_Implementation(FVector ExplosionLocation)
{
    PlayPulseVFX(ExplosionLocation);
}


