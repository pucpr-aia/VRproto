// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Game/GrabComponent.h"
#include "GameFramework/Pawn.h"
#include "VRPawn.generated.h"

UCLASS()
class PROTOTYPES_API AVRPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVRPawn();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMotionControllerComponent* MotionControllerComponent_R;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMotionControllerComponent* MotionControllerComponent_L;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidgetInteractionComponent* WidgetInteractionComponent_R;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidgetInteractionComponent* WidgetInteractionComponent_L;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* RightMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* LeftMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USphereComponent* RightHandSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USphereComponent* LeftHandSphere;
	
	/** Referência do ator que está sendo segurado pelo controlador **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* AttachedElement;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY()
	int32 Score = 0;

	void GripLeft();
	void GripRight();
	void GripReleaseLeft();
	void GripReleaseRight();

	UGrabComponent* GetNearestGripComponent(UMotionControllerComponent* MotionControllerComponent, USphereComponent* Sphere);
	
	UPROPERTY()
	UGrabComponent* HeldComponentLeft;

	UPROPERTY()
	UGrabComponent* HeldComponentRight;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void IncrementScore();

	UFUNCTION()
	void DecrementScore();

	UFUNCTION()
	int32 GetScore() const;
};
