// Copyright Epic Games, Inc. All Rights Reserved.

#include "GulagCharacter.h"
#include "MyPlayerController.h"
#include "MyGameMode.h"
#include "GrenadeBase.h"
#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AGulagCharacter


void AGulagCharacter::BeginPlay() {

	Super::BeginPlay();

	if (HasAuthority())  // Server only
	{
		CurrentAmmo = MaxAmmo;
		bFrozenForRound = true;
	}

	if (IsLocallyControlled()) 
	{

		if (PlayerWidgetClass) {
			PlayerWidgetInstance = CreateWidget<UPlayerWidget>(GetWorld(), PlayerWidgetClass);

			if (PlayerWidgetInstance) {
				PlayerWidgetInstance->AddToViewport(-1);
				PlayerWidgetInstance->UpdateHealthBar(Health,MaxHealth);
				PlayerWidgetInstance->UpdateAmmoText(CurrentAmmo);
			}
		}
		
		if(CrosshairWidgetClass)
		{
			CrosshairWidgetInstance = CreateWidget<UCrosshairWidget>(GetWorld(), CrosshairWidgetClass);
			if (CrosshairWidgetInstance) {
				CrosshairWidgetInstance->AddToViewport(-1);
				Client_ShowCrosshair(false);
			}
		}
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Not locally controlled!"));
	}
}

void AGulagCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector Velocity = GetVelocity();
	Velocity.Z = 0; //Ignore vertical velocity
	isWalking = Velocity.SizeSquared() > 10.f;

}


void AGulagCharacter::Client_ShowCrosshair_Implementation(bool bShow)
{
	if (!IsLocallyControlled())
		return;

	if (CrosshairWidgetInstance) {
		CrosshairWidgetInstance->BP_ShowCrosshair(bShow);
	}

}

void AGulagCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);	

	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Removing Player Widget"));
		PlayerWidgetInstance->RemoveFromParent();
		PlayerWidgetInstance = nullptr;
	}

	if (CrosshairWidgetInstance)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Removing Crosshair Widget"));
		CrosshairWidgetInstance->RemoveFromParent();
		CrosshairWidgetInstance = nullptr;
	}
}



//registers the variables in the networking system for replication
void AGulagCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGulagCharacter, Health);
	DOREPLIFETIME(AGulagCharacter, CurrentAmmo);
	DOREPLIFETIME(AGulagCharacter, isReloading);
	DOREPLIFETIME(AGulagCharacter, canFire);
	DOREPLIFETIME(AGulagCharacter, CurrentFlashbangs);
	DOREPLIFETIME(AGulagCharacter, CurrentRevealGrenades);
	DOREPLIFETIME(AGulagCharacter, bFrozenForRound);
}


AGulagCharacter::AGulagCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	//GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetAllBodiesSimulatePhysics(false);

	isWalking = false;
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGulagCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AGulagCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGulagCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGulagCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AGulagCharacter::Sprint);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AGulagCharacter::StopSprint);

		//Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AGulagCharacter::Input_LMBPressed);

		//ADS
		EnhancedInputComponent->BindAction(AdsAction, ETriggerEvent::Started, this, &AGulagCharacter::StartADS);
		EnhancedInputComponent->BindAction(AdsAction, ETriggerEvent::Completed, this, &AGulagCharacter::StopADS);

		//Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AGulagCharacter::Input_Reload);

		//Throw Grenade
		EnhancedInputComponent->BindAction(ThrowFlashAction, ETriggerEvent::Started, this, &AGulagCharacter::Input_ThrowFlash);
		EnhancedInputComponent->BindAction(ThrowRevealAction, ETriggerEvent::Started, this, &AGulagCharacter::Input_ThrowReveal);

		EnhancedInputComponent->BindAction(GoToMainMenuAction, ETriggerEvent::Started, this, &AGulagCharacter::Input_GoToMainMenu);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}




void AGulagCharacter::Move(const FInputActionValue& Value)
{
	if (bFrozenForRound) {
		isWalking = false;
		return;
	}

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AGulagCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	if (bFrozenForRound)
		return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGulagCharacter::Sprint() {
	
	if (bFrozenForRound)
		return;

	if (!isWalking)
		return;

	if (!IsAiming) {
		StopADS();
	}

	isRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AGulagCharacter::StopSprint() {
	
	isRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AGulagCharacter::Input_LMBPressed() //firing input
{
	if (bFrozenForRound)
		return;

	if(isRunning)
		return;

	if (IsLocallyControlled())
	{
		Server_FireWeapon(); //tell server to fire weapon

	}
}


//has the ammo check and reload check
void AGulagCharacter::Server_FireWeapon_Implementation()
{
	
	if (!canFire || isReloading) {
		UE_LOG(LogTemp, Warning, TEXT("Server_FireWeapon: blocked (canFire=%d, isReloading=%d)"),
		canFire, isReloading);  //reload check
		return;
	}
		
	//Block firing while sprinting
	if (isRunning)
		return;

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Server_FireWeapon executed on server"));
	if (CurrentAmmo > 0) {  //ammo check

		canFire = false;
		CurrentAmmo--;
		
		Multicast_SpawnMuzzleFlash(); // spawn muzzle flash on all clients
		Server_LineTrace(); //linetrace on server

		if (IsLocallyControlled())
		{
			if (PlayerWidgetInstance)
			{
				PlayerWidgetInstance->UpdateAmmoText(CurrentAmmo); //UI update on Client
			}
		}

		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle,this,&AGulagCharacter::ResetFire,FireRate,false);
		return; //stop the function no need to move further 
	}

	Server_ReloadAmmo(); //if no conditions met then reload

}

void AGulagCharacter::ResetFire() {
	canFire = true;
}

void AGulagCharacter::Multicast_SpawnMuzzleFlash_Implementation()
{
	BP_SpawnMuzzleFlash();
}

void AGulagCharacter::OnRep_CurrentAmmo()
{
	if (!PlayerWidgetInstance)
	{
		return;
	}
	PlayerWidgetInstance->UpdateAmmoText(CurrentAmmo);
}

//Conduct line trace on server and dela damage to hit actor
void AGulagCharacter::Server_LineTrace_Implementation()
{
	
	FVector Start;
	FVector End;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FRotator CamRot;
	PC->GetPlayerViewPoint(Start, CamRot);
	End = Start + CamRot.Vector() * TraceDistance;

	FHitResult Hit; 
	FCollisionQueryParams Params; 
	Params.AddIgnoredActor(this); 

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Pawn, Params
	);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, 0, 1.f);

	if (bHit)
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 16.f, 12, FColor::Green, false, 1.f);

		//casts the hit actor to the player class; if succes then deal damage
		AGulagCharacter* HitActor = Cast<AGulagCharacter>(Hit.GetActor());

		if (HitActor) {

			if (HitActor != this) {
				HitActor->Server_TakeDamage(10, GetController());
				HitActor->Client_PlayDamageSound();
				HitActor->Client_ShowDamageMarker(10);

				Client_PlayImpactSound();
				Client_PlayBloodVfx(Hit);
			}
			
		}
		else {
			Client_SpawnBulletDecal(Hit);
			Client_PlayWallHitVfx(Hit);
		}

	}

}

void AGulagCharacter::Client_SpawnBulletDecal_Implementation(const FHitResult& HitResult)
{
	BP_SpawnBulletDecal(HitResult);
}

void AGulagCharacter::Client_PlayImpactSound_Implementation()
{
	BP_PlayImpactSound();
}

void AGulagCharacter::Client_PlayDamageSound_Implementation()
{
	BP_PlayDamageSound();
}


void AGulagCharacter::StartADS()
{
	if (bFrozenForRound)
		return;

	if (isRunning)
	{
		StopSprint();
	}
	IsAiming = true;
}

void AGulagCharacter::StopADS()
{
	IsAiming = false;
}

void AGulagCharacter::Server_TakeDamage_Implementation(float DamageAmount, AController* InstigatorController)
{	

	if(HasAuthority())
	{

		if (isDead)
			return;

		LastDamageInstigator = InstigatorController;
		Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth); 
		Client_UpdateHealthUI(Health); //update client UI after server chnages values


		if (Health <= 0.f) {

			isDead = true;

			AMyGameMode* GM = GetWorld()->GetAuthGameMode<AMyGameMode>();
			if (GM)
			{
				AController* VictimController = GetController();
				AController* KillerController = LastDamageInstigator;

				if (KillerController == nullptr)
				{
					KillerController = VictimController; // fallback (suicide)
				}

				GM->PlayerDied(KillerController, VictimController);
			}

			Die();
	}
	

	}
}

void AGulagCharacter::Die()
{
	// Disable character movement
	GetCharacterMovement()->DisableMovement();

	// Disable capsule collision so the character falls naturally
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Stop all movement replication
	GetCharacterMovement()->StopMovementImmediately();

	//Disable input
	if (AController* MyController = GetController())
	{
		MyController->SetIgnoreMoveInput(true);
		MyController->SetIgnoreLookInput(true);
	}

	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	//Set collision
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// Enable ragdoll
	//GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);               // Turn on physics
	GetMesh()->SetAllBodiesSimulatePhysics(true);      // Ragdoll entire body
	GetMesh()->WakeAllRigidBodies();                   // Prevent sleeping
	GetMesh()->bPauseAnims = true;                     // Stop animation blueprint
	GetMesh()->bNoSkeletonUpdate = true;               // Avoid conflicts

	//
	//GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// Schedule respawn after delay
	/*AMyGameMode* GM = GetWorld()->GetAuthGameMode<AMyGameMode>();
	if (GM)
	{
		FTimerHandle RespawnTimer;
		GetWorldTimerManager().SetTimer(
			RespawnTimer,
			FTimerDelegate::CreateUObject(GM, &AMyGameMode::RequestRespawn, GetController()),
			3.0f, false
		);
	}*/
}

void AGulagCharacter::ResetCharacterForNewRound()
{
	Health = MaxHealth;
	isDead = false;

	//Tell client UI to update health
	Client_UpdateHealthUI(Health);
	Client_UpdateAmmoTextUI(CurrentAmmo);

	//Reset ammo and weapon stats flags on server
	if (HasAuthority())
	{
		CurrentAmmo = MaxAmmo;
		isReloading = false;
		canFire = true;
	}
}

void AGulagCharacter::FreezeForRound()
{
	bFrozenForRound = true;
	ApplyFreeze();
}

void AGulagCharacter::UnfreezeForRound()
{
	bFrozenForRound = false;
	ApplyUnfreeze();
}

void AGulagCharacter::OnRep_Frozen()
{
	if (bFrozenForRound)
		ApplyFreeze();
	else
		ApplyUnfreeze();
}

void AGulagCharacter::ApplyFreeze()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	MoveComp->StopMovementImmediately();
	MoveComp->SetMovementMode(MOVE_None);
}

void AGulagCharacter::ApplyUnfreeze()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AGulagCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bFrozenForRound)
	{
		ApplyFreeze();
	}
	else
	{
		ApplyUnfreeze();
	}
}

void AGulagCharacter::Client_UpdateHealthUI_Implementation(float NewHealth)
{
	if (PlayerWidgetInstance) {
		PlayerWidgetInstance->UpdateHealthBar(NewHealth, MaxHealth);
	}
}

void AGulagCharacter::Client_UpdateAmmoTextUI_Implementation(int32 NewAmmo)
 {
	if (PlayerWidgetInstance){
		PlayerWidgetInstance->UpdateAmmoText(NewAmmo);
	}
}

void AGulagCharacter::Input_Reload() {

	if (bFrozenForRound)
		return;

	if (isRunning)
		return;

	if (IsLocallyControlled())
	{
		Server_ReloadAmmo();
	}
}

//Reloads ammo -> stops firing, changes the ammo count; plays the reload animation
void AGulagCharacter::Server_ReloadAmmo_Implementation()
{
	if (bFrozenForRound)
		return;

	if (isRunning)
		return;

	if (CurrentAmmo < MaxAmmo) {
		
		isReloading = true;
		canFire = false;
		CurrentAmmo = MaxAmmo;

		// Tell owning client to play animation
		Client_PlayReloadAnimation();

		if (IsLocallyControlled())
		{
			if (PlayerWidgetInstance)
			{
				PlayerWidgetInstance->UpdateAmmoText(CurrentAmmo);
			}
		}
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Ammo Full, no need to reload"));
	}

	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AGulagCharacter::FinishReload,
		1.5f,
		false
	);
	
}

void AGulagCharacter::FinishReload()
{
	CurrentAmmo = MaxAmmo;
	isReloading = false;
	canFire = true;
}

void AGulagCharacter::Client_PlayReloadAnimation_Implementation()
{
	BP_PlayReloadAnimation();
}

void AGulagCharacter::Input_ThrowFlash()
{
	if (!IsLocallyControlled())
		return;

	if (bFrozenForRound)
		return;

	if (CurrentFlashbangs <= 0)
		return;

	FVector AimDir = GetGrenadeAimDirection();

	Server_ThrowGrenade(AimDir, EGrenadeType::Flash);
}

void AGulagCharacter::Input_ThrowReveal()
{
	if (!IsLocallyControlled())
		return;

	if (bFrozenForRound)
		return;

	if (CurrentRevealGrenades <= 0)
		return;

	FVector AimDir = GetGrenadeAimDirection();

	Server_ThrowGrenade(AimDir, EGrenadeType::Reveal);
}

void AGulagCharacter::Input_GoToMainMenu()
{
	if (!IsLocallyControlled())
		return;

	AMyPlayerController* PC = Cast<AMyPlayerController>(Controller);
	if (PC) {
		PC->ReturnToMainMenuLevel();
	}
}

//get direction where player is aiming the grenade
FVector AGulagCharacter::GetGrenadeAimDirection() const
{
	if (!Controller)
		return FVector::ZeroVector;

	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC)
		return FVector::ZeroVector;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceStart = CameraLocation;
	const FVector TraceEnd = TraceStart + (CameraRotation.Vector() * 10000.f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	FVector TargetPoint = bHit ? Hit.ImpactPoint : TraceEnd;
	
	// Direction from camera
	FVector AimDirection = (TargetPoint - TraceStart).GetSafeNormal();

	return AimDirection;
}

void AGulagCharacter::OnRep_Flashbangs()
{
	if (!PlayerWidgetInstance)
		return;
	if (CurrentFlashbangs <= 0) {
		PlayerWidgetInstance->BP_OnFlashbangsFinished();
	}
}

void AGulagCharacter::OnRep_Reveals()
{
	if (!PlayerWidgetInstance)
		return;
	if (CurrentRevealGrenades <= 0) {
		PlayerWidgetInstance->BP_OnRevealsFinished();
	}
}

//client tell server to throw grenade of which type -> chooses the grenade class
void AGulagCharacter::Server_ThrowGrenade_Implementation(FVector_NetQuantizeNormal
	AimDirection, EGrenadeType GrenadeType)
{
	if (!Controller || AimDirection.IsNearlyZero())
		return;

	TSubclassOf<AGrenadeBase> GrenadeClass = nullptr;

	switch (GrenadeType)
	{
	case EGrenadeType::Flash:
		GrenadeClass = FlashbangClass;
		break;

	case EGrenadeType::Reveal:
		GrenadeClass = RevealGrenadeClass;
		break;
	}

	if (!GrenadeClass)
		return;

	ConsumeGrenade(GrenadeType);

	 //Cache for Animnotify
	CachedGrenadeAimDirection = AimDirection;
	CachedGrenadeClass = GrenadeClass;

	Multicast_PlayGrenadeThrowAnim();

}

void AGulagCharacter::Client_RequestGrenadeSpawnFromNotify()
{
	if (!IsLocallyControlled())
		return;

	//Server_SpawnGrenadeFromAnimNotify();
}

void AGulagCharacter::Server_SpawnGrenadeFromAnimNotify_Implementation(const FTransform& SpawnTransform, const FVector& AimDirection)
{
	if (!HasAuthority())
		return;

	if (!CachedGrenadeClass)
		return;

	//SpawnGrenade(CachedGrenadeAimDirection, CachedGrenadeClass);

	//CachedGrenadeClass = nullptr;

	//const FRotator AimRot = AimDirection.Rotation();
	//const FTransform SpawnTransform(AimRot, SpawnLocation);
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	
	AGrenadeBase * Grenade = GetWorld()->SpawnActor<AGrenadeBase>(CachedGrenadeClass, SpawnTransform, Params);
	if (!Grenade)
	 {
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Server_SpawnGrenadeFromAnimNotify: SpawnActor failed for %s"), *GetName());
		CachedGrenadeClass = nullptr;
		return;
	 }
	
	//Apply initial impulse based on the provided aim direction
	if (UPrimitiveComponent* PhysComp = Cast<UPrimitiveComponent>(Grenade->GetRootComponent()))
	{
		FVector Velocity = AimDirection.GetSafeNormal() * GrenadeThrowStrength;
		Velocity += GetCharacterMovement()->Velocity * 0.5f;
		PhysComp->AddImpulse(Velocity, NAME_None, true);

		const FVector InitialAngularVelocityDeg = FVector(600.f, 300.f, 1200.f);
		PhysComp->SetPhysicsAngularVelocityInDegrees(InitialAngularVelocityDeg, false);
	}
	
	// Clear cached data so doesn't spawn again
	CachedGrenadeClass = nullptr;
}

//spawns the grenade a/t the grenade class passed and applies impulse to it
void AGulagCharacter::SpawnGrenade(const FVector& AimDirection, TSubclassOf<AGrenadeBase> GrenadeClass)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Spawning Grenade"));
			if (!HasAuthority() || !GrenadeClass)
				return;

			FTransform SpawnTransform =
				GetMesh()->GetSocketTransform(GrenadeSocketName, RTS_World);

			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.Instigator = this;

			AGrenadeBase* Grenade = GetWorld()->SpawnActor<AGrenadeBase>(
				GrenadeClass,
				SpawnTransform,
				Params
			);

			if (!Grenade)
				return;

			UPrimitiveComponent* PhysComp =
				Cast<UPrimitiveComponent>(Grenade->GetRootComponent());

			if (PhysComp && PhysComp->IsSimulatingPhysics())
			{
				FVector Velocity = AimDirection * GrenadeThrowStrength;
				Velocity += GetCharacterMovement()->Velocity * 0.5f;

				PhysComp->AddImpulse(Velocity, NAME_None, true);
			}
}

//reduces grenade count based on type
void AGulagCharacter::ConsumeGrenade(EGrenadeType GrenadeType)
{
	if (!HasAuthority())
		return;

	switch (GrenadeType)
	{
	case EGrenadeType::Flash:
		if (CurrentFlashbangs > 0)
		{
			CurrentFlashbangs--;
		}
		break;

	case EGrenadeType::Reveal:
		if (CurrentRevealGrenades > 0)
		{
			CurrentRevealGrenades--;
		}
		break;

	default:
		break;
	}
}

void AGulagCharacter::Multicast_PlayGrenadeThrowAnim_Implementation()
{
	// Local player already played animation instantly
	if (IsLocallyControlled())
		return;

	// Play grenade throw montage here
	if (GrenadeThrowMontage && GetMesh())
	{
		PlayAnimMontage(GrenadeThrowMontage);
	}
}

//Flash effect application on client
void AGulagCharacter::Client_ApplyFlash_Implementation(float FlashDuration, FVector FlashLocation)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Client_ApplyFlash"));
	
	float CurrentTime = GetWorld()->GetTimeSeconds();

	FlashEndTime = FMath::Max(FlashEndTime, CurrentTime + FlashDuration);
	CurrentFlashTime = FlashDuration;

	//blueprint post process entry
	OnFlashStarted(FlashDuration, FlashLocation);
}

void AGulagCharacter::Client_UpdateRevealedUI_Implementation(bool bIsRevealed, float duration)
{
	BP_UpdateRevealedUI(bIsRevealed,duration);

}

void AGulagCharacter::Client_PlayBloodVfx_Implementation(const FHitResult& HitResult)
{
	BP_PlayBloodVfx(HitResult);
}

void AGulagCharacter::Client_PlayWallHitVfx_Implementation(const FHitResult& HitResult)
{
	BP_PlayWallHitVfx(HitResult);
}

void AGulagCharacter::Client_ShowDamageMarker_Implementation(const float DamageAmount)
{
	BP_ShowDamageMarker(DamageAmount);
}
