// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Game/GrabComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "VRCharacter.generated.h"

class UInputComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class USoundBase;
class UAnimMontage;

UCLASS()
class PROTOTYPES_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
	
public:
	// Sets default values for this character's properties
	AVRCharacter();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* RightHand;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* LeftHand;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMotionControllerComponent* MotionControllerComponent_R;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMotionControllerComponent* MotionControllerComponent_L;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UWidgetInteractionComponent* WidgetInteractionComponent_R;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UWidgetInteractionComponent* WidgetInteractionComponent_L;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USphereComponent* RightHandSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USphereComponent* LeftHandSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPhysicsHandleComponent *PhysicsHandleComponent = nullptr;
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	
protected:
	virtual void BeginPlay() override;
	
	/** Handler for a touch input beginning. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Structure that handles touch data so we can process the various stages of touch. */
	struct TouchData
	{
		TouchData() { bIsPressed = false; Location = FVector::ZeroVector; }
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};

	/*
	 * Handle begin touch event.
	 * Stores the index and location of the touch in a structure
	 *
	 * @param	FingerIndex	The touch index
	 * @param	Location	Location of the touch
	 */
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/*
	 * Handle end touch event.
	 * If there was no movement processed this will fire a projectile, otherwise this will reset pressed flag in the touch structure
	 *
	 * @param	FingerIndex	The touch index
	 * @param	Location	Location of the touch
	 */
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/*
	 * Handle touch update.
	 * This will update the look position based on the change in touching position
	 *
	 * @param	FingerIndex	The touch index
	 * @param	Location	Location of the touch
	 */
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);

	// Structure to handle touch updating
	TouchData	TouchItem;
	
	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	void TryEnableTouchscreenMovement(UInputComponent* InputComponent);

	void GripLeft();
	void GripRight();
	// void GripRelease();
	void GripReleaseLeft();
	void GripReleaseRight();
	// void TryGrab(UMotionControllerComponent* MotionControllerComponent, USphereComponent* Sphere);
	UGrabComponent* GetNearestGripComponent(const UMotionControllerComponent* MotionControllerComponent, USphereComponent* Sphere);

	FHitResult GetComponentInSight(USphereComponent* SphereComponentRef);
	
	UPROPERTY()
	UGrabComponent* HeldComponentLeft;

	UPROPERTY()
	UGrabComponent* HeldComponentRight;
	
public:	
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Referência do ator que está sendo segurado pelo controlador **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* AttachedElement;

	bool bIsGrabbing = false;
	float GrabDistance;
	UPROPERTY()
	UPrimitiveComponent* HitComponent;
};
