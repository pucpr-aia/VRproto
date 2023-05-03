// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "Data/Data.h"
#include "Components/SceneComponent.h"
#include "GrabComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrabbed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRelease);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROTOTYPES_API UGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMotionControllerComponent* MotionControllerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGrabType GrabType;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	FRotator PrimaryGrabRelativeRotation;

	UPROPERTY()
	bool bSimulateOnDrop;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnGrabbed OnGrabbed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnRelease OnRelease;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHeld = false;
	
	UFUNCTION(BlueprintCallable)
	bool TryGrab(UMotionControllerComponent* MotionControllerComponent);
	UFUNCTION(BlueprintCallable)
	bool TryRelease();
	void SetShouldSimulateOnDrop();
	void SetPrimitiveCompPhysics(bool bSimulate);
	
	void AttachParentToMotionController(UMotionControllerComponent* MotionControllerComponent);
	EControllerHand GetHandFromMotionSource(UMotionControllerComponent* MotionControllerComponent);
};
