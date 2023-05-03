// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/GrabComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UGrabComponent::UGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabComponent::BeginPlay()
{
	Super::BeginPlay();
	SetShouldSimulateOnDrop();
	if(Cast<UPrimitiveComponent>(GetAttachParent()))
		Cast<UPrimitiveComponent>(GetAttachParent())->SetCollisionProfileName(FName("PhysicsActor"), true);
	
}


// Called every frame
void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UGrabComponent::TryGrab(UMotionControllerComponent* MotionControllerComponent)
{
	switch(GrabType)
	{
		case EGrabType::None:
			break;
		case EGrabType::Free:
			SetPrimitiveCompPhysics(false);
			AttachParentToMotionController(MotionControllerComponent);
			bIsHeld = true;
			break;
		case EGrabType::Snap:
			SetPrimitiveCompPhysics(false);
			AttachParentToMotionController(MotionControllerComponent);
			GetAttachParent()->SetRelativeRotation(UKismetMathLibrary::NegateRotator(GetRelativeRotation()), false, nullptr, ETeleportType::TeleportPhysics);
			GetAttachParent()->SetWorldLocation(UKismetMathLibrary::Add_VectorVector(MotionControllerRef->K2_GetComponentLocation(), UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::Subtract_VectorVector(K2_GetComponentLocation(), GetAttachParent()->K2_GetComponentLocation()), -1.f)), false, nullptr, ETeleportType::TeleportPhysics);
			bIsHeld = true;
			break;
		case EGrabType::Custom:
			bIsHeld = true;
			break;
	}
	if(bIsHeld)
	{
		MotionControllerRef = MotionControllerComponent;
		return true;
	}
	return false;
}

bool UGrabComponent::TryRelease()
{
	switch (GrabType)
	{
	case EGrabType::None:
		break;
	case EGrabType::Free:
		if(bSimulateOnDrop)
		{
			SetPrimitiveCompPhysics(true);
		}
		else
		{
			GetOwner()->K2_DetachFromActor(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld);
		}
		bIsHeld = false;
		break;
	case EGrabType::Snap:
		break;
	case EGrabType::Custom:
		bIsHeld = false;
		break;
	}
	if(bIsHeld)
	{
		return false;
	}
	return true;
}

void UGrabComponent::SetShouldSimulateOnDrop()
{
	if(Cast<UPrimitiveComponent>(GetAttachParent()))
	{
		if(Cast<UPrimitiveComponent>(GetAttachParent())->IsAnySimulatingPhysics())
		{
			bSimulateOnDrop = true;
		}
	}
}

void UGrabComponent::SetPrimitiveCompPhysics(bool bSimulate)
{
	Cast<UPrimitiveComponent>(GetAttachParent())->SetSimulatePhysics(bSimulate);
}

void UGrabComponent::AttachParentToMotionController(UMotionControllerComponent* MotionControllerComponent)
{
	GetAttachParent()->K2_AttachToComponent(MotionControllerComponent, FName("None"), EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
}

EControllerHand UGrabComponent::GetHandFromMotionSource(UMotionControllerComponent* MotionControllerComponent)
{
	if(MotionControllerComponent->MotionSource == FName("Left"))
	{
		return EControllerHand::Left;
	}
	return EControllerHand::Right;
}



