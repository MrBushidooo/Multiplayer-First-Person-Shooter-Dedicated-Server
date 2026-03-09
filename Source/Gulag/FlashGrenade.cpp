// Fill out your copyright notice in the Description page of Project Settings.


#include "FlashGrenade.h"
#include "GulagCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


void AFlashGrenade::Explode()
{
    if (!HasAuthority())
        return;

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Flash Exploding"));

    const FVector ExplosionLocation = GetActorLocation();

    TArray<AActor*> Ignored;
    Ignored.Add(this);

    TArray<AActor*> Hits;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        ExplosionLocation,
        FlashRadius,
        { UEngineTypes::ConvertToObjectType(ECC_Pawn) },
        AGulagCharacter::StaticClass(),
        Ignored,
        Hits
    );

    for (AActor* A : Hits)
    {
        AGulagCharacter* Character = Cast<AGulagCharacter>(A);
        if (!Character) continue;

        APlayerController* PC = Cast<APlayerController>(Character->GetController());
        if (!PC) continue;

        //Los check
        FHitResult Hit;
        FVector EyeLoc;
        FRotator EyeRot;
        Character->GetActorEyesViewPoint(EyeLoc, EyeRot);

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Character);

        bool bBlocked = GetWorld()->LineTraceSingleByChannel(
            Hit,
            EyeLoc,
            ExplosionLocation,
            VisibilityChannel,
            Params
        );

        if (bBlocked && Hit.GetActor() != this)
        {
            continue;
        }

        //Pov check
        FVector ToExplosion = (ExplosionLocation - EyeLoc).GetSafeNormal();
        float FacingDot = FVector::DotProduct(EyeRot.Vector(), ToExplosion);

        if (FacingDot < MinFacingDot)
            continue;

        //Distnace check
        float Dist = FVector::Distance(Character->GetActorLocation(), ExplosionLocation);
        float Alpha = FMath::Clamp(Dist / FlashRadius, 0.f, 1.f);
        float FlashDuration = FMath::Lerp(MaxFlashDuration, MinFlashDuration, Alpha);

        if (FlashDuration <= 0.f)
            continue;

        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Flashing Player"));
        Character->Client_ApplyFlash(FlashDuration, ExplosionLocation);
    }

	Multicast_StopTrailVFX();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Flash Duration: %.2f"), MaxFlashDuration));
    Destroy();
}

void AFlashGrenade::Multicast_StopTrailVFX_Implementation()
{
	BP_StopTrailVFX();
}

