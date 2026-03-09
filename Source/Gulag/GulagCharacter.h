// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerWidget.h"
#include "CrosshairWidget.h"
#include "GrenadeBase.h"
#include "Logging/LogMacros.h"
#include "GulagCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EGrenadeType : uint8
{
	Flash UMETA(DisplayName = "Flash"),
	Reveal    UMETA(DisplayName = "Reveal")
};

UCLASS(config=Game)
class AGulagCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	/*UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	/*UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	//Shift Sprint Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShiftAction;

	//LMB Fire Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	//ADS Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AdsAction;

	//Reload Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	//Throw Grenade Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ThrowFlashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ThrowRevealAction;

	//Main Menu Ation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* GoToMainMenuAction;

	//Client calls this on input, runs on client only
	void Input_LMBPressed();
	
	void Input_ThrowFlash();

	void Input_ThrowReveal();
	
	void Input_GoToMainMenu();


	//Debug trace length
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float TraceDistance = 7500.f;

	//Collision channel for hitscan
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	
	

public:
	AGulagCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool isRunning = false;

	//ADS settings
	UPROPERTY(BlueprintReadOnly, Category = "ADS")
	bool IsAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS")
	float DefaultFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS")
	float ADSFOV = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS")
	float ADSSpeed = 15.0f;

	// Camera offset during ADS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADS")
	FVector ADSCameraOffset = FVector(0.f, 35.f, -5.f);

	UFUNCTION(BlueprintCallable, Category = "ADS")
	void StartADS();

	UFUNCTION(BlueprintCallable, Category = "ADS")
	void StopADS();

	//Death
	UFUNCTION()
	void ResetCharacterForNewRound();

	bool isDead = false;

	//Freeze
	void FreezeForRound();
	void UnfreezeForRound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Frozen, Category = "Freeze")
	bool bFrozenForRound = true;

	UFUNCTION()
	void OnRep_Frozen();

	void ApplyFreeze();
	void ApplyUnfreeze();

	UFUNCTION()
	void Client_RequestGrenadeSpawnFromNotify();

	//Flashbang effect - Client only
	UFUNCTION(Client, Reliable)
	void Client_ApplyFlash(float FlashDuration, FVector FlashLOcation);

	UPROPERTY()
	float CurrentFlashTime = 0.f;

	UPROPERTY()
	float FlashEndTime = 0.f;

	UFUNCTION(BlueprintImplementableEvent)
	void OnFlashStarted(float Duration, FVector Location);

	//called on client that gets revealed
	//UFUNCTION(Client, Reliable)
	//void Client_ApplyReveal(float RevealDuration);

	//UFUNCTION(BlueprintImplementableEvent)
	//void OnRevealStarted(float Duration);

	UFUNCTION(Server, Reliable)
	void Server_SpawnGrenadeFromAnimNotify(const FTransform& SpawnTransform, const FVector& AimDirection);

	//'Revealed' UI update
	UFUNCTION(Client, Reliable)
	void Client_UpdateRevealedUI(bool bIsRevealed, float duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void BP_UpdateRevealedUI(bool bIsRevealed, float duration);


	//Crosshair
	UPROPERTY()
	UCrosshairWidget* CrosshairWidgetInstance;

	UFUNCTION(Client, Reliable)
	void Client_ShowCrosshair(bool bShow);

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	bool isWalking = false;

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Sprint();
	void StopSprint();

	// Server RPC 
	UFUNCTION(Server, Reliable)
	void Server_LineTrace();

	//Firing
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SpawnMuzzleFlash();

	// Server multicast to show flash on everyone
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnMuzzleFlash();

	UFUNCTION(Client, Unreliable)
	void Client_PlayImpactSound();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayImpactSound();

	UFUNCTION(Client, Unreliable)
	void Client_PlayBloodVfx(const FHitResult& HitResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void BP_PlayBloodVfx(const FHitResult& HitResult);

	UFUNCTION(Client, Unreliable)
	void Client_ShowDamageMarker(const float DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void BP_ShowDamageMarker(const float DamageAmount);

	UFUNCTION(Client, Unreliable)
	void Client_PlayWallHitVfx(const FHitResult& HitResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void BP_PlayWallHitVfx(const FHitResult& HitResult);

	UFUNCTION(Client, Unreliable)
	void Client_PlayDamageSound();

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void BP_PlayDamageSound();

	UFUNCTION(Client, Unreliable)
	void Client_SpawnBulletDecal(const FHitResult& HitResult );

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void BP_SpawnBulletDecal(const FHitResult& HitResult);



	//UI 
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlayerWidget> PlayerWidgetClass;

	UPROPERTY(BlueprintReadWrite)
	UPlayerWidget* PlayerWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	

	//Health And Damage
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(float DamageAmount, AController* InstigatorController);

	//UFUNCTION()
	//void OnRep_Health();

	UFUNCTION(Client, Reliable)
	void Client_UpdateHealthUI(float NewHealth);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Health = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	AController* LastDamageInstigator = nullptr;

	//Death
	UFUNCTION()
	void Die();

	

	//Ammo
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentAmmo;

	UFUNCTION()
	void OnRep_CurrentAmmo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo = 30;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool isReloading = false;	

	void Input_Reload();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ReloadAmmo();

	FTimerHandle ReloadTimerHandle;

	void FinishReload();

	UFUNCTION(Client, Reliable)
	void Client_PlayReloadAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayReloadAnimation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate = 0.25f; 

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool canFire = true;

	FTimerHandle FireTimerHandle;

	UFUNCTION(BlueprintCallable)
	void ResetFire();

	UFUNCTION(Client, Reliable)
	void Client_UpdateAmmoTextUI(int32 NewAmmo);



	//Grenade 
	FVector GetGrenadeAimDirection() const;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float GrenadeCooldown = 1.0f;

	// Runtime (server only)
	float LastGrenadeThrowTime = -100.f;

	int32 CurrentGrenades = 3;

	//Grenade - classes
	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	TSubclassOf<AGrenadeBase> FlashbangClass;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	TSubclassOf<AGrenadeBase> RevealGrenadeClass;

	UPROPERTY(ReplicatedUsing = OnRep_Flashbangs, EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	int32 CurrentFlashbangs = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	int32 MaxFlashbangs = 1;
	
	UFUNCTION()
	void OnRep_Flashbangs();

	UPROPERTY(ReplicatedUsing = OnRep_Reveals, EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	int32 CurrentRevealGrenades = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	int32 MaxRevealGrenades = 1;

	UFUNCTION()
	void OnRep_Reveals();

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float GrenadeThrowStrength = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	FName GrenadeSocketName = TEXT("GrenadeSocket");

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	UAnimMontage* GrenadeThrowMontage;

	UFUNCTION(Server, Reliable)
	void Server_ThrowGrenade(FVector_NetQuantizeNormal AimDirection, EGrenadeType GrenadeType);

	void SpawnGrenade(
		const FVector& AimDirection,
		TSubclassOf<AGrenadeBase> GrenadeClass
	);

	UFUNCTION()
	void ConsumeGrenade(EGrenadeType GrenadeType);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayGrenadeThrowAnim();

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float GrenadeSpawnDelay = 0.28f; // match the Anim notify time 
	FTimerHandle GrenadeSpawnTimerHandle;

	//Cached data used by Anim notify
	UPROPERTY()
	FVector CachedGrenadeAimDirection;

	UPROPERTY()
	TSubclassOf<AGrenadeBase> CachedGrenadeClass;

	




protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	//FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	//FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

