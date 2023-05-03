#include "Player/VRCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Game/GrabComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0, 0, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	RightHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand"));
	LeftHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand"));
	RightHand->SetupAttachment(FirstPersonCameraComponent);
	LeftHand->SetupAttachment(FirstPersonCameraComponent);
	
	MotionControllerComponent_R = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_R"));
	MotionControllerComponent_R->SetupAttachment(RightHand);
	MotionControllerComponent_L = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_L"));
	MotionControllerComponent_L->SetupAttachment(LeftHand);

	WidgetInteractionComponent_L = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction_L"));
	WidgetInteractionComponent_L->SetupAttachment(MotionControllerComponent_L);
	WidgetInteractionComponent_R = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction_R"));
	WidgetInteractionComponent_R->SetupAttachment(MotionControllerComponent_R);

	RightHandSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Right Hand Sphere"));
	LeftHandSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Left Hand Sphere"));

	RightHandSphere->SetupAttachment(RightHand);
	LeftHandSphere->SetupAttachment(LeftHand);

	RightHandSphere->SetSphereRadius(15.f);
	LeftHandSphere->SetSphereRadius(15.f);

	RightHandSphere->ComponentTags.Add(FName("Hand"));
	LeftHandSphere->ComponentTags.Add(FName("Hand"));
}

void AVRCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);
	
	// Set up gameplay key bindings

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	// Attempt to enable touch screen movement
	TryEnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVRCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AVRCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVRCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, this, &AVRCharacter::GripLeft);
	PlayerInputComponent->BindAction("GrabLeft", IE_Released, this, &AVRCharacter::GripReleaseLeft);
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, this, &AVRCharacter::GripRight);
	PlayerInputComponent->BindAction("GrabRight", IE_Released, this, &AVRCharacter::GripReleaseRight);
}

void AVRCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// If touch is already pressed check the index. If it is not the same as the current touch assume a second touch and thus we want to fire
	if (TouchItem.bIsPressed == false)
	{
		// Cache the finger index and touch location and flag we are processing a touch
		TouchItem.bIsPressed = true;
		TouchItem.FingerIndex = FingerIndex;
		TouchItem.Location = Location;
		TouchItem.bMoved = false;
	}
}

void AVRCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// If we didn't record the start event do nothing, or this is a different index
	if((TouchItem.bIsPressed == false) || ( TouchItem.FingerIndex != FingerIndex) )
	{
		return;
	}

	// Flag we are no longer processing the touch event
	TouchItem.bIsPressed = false;
}

void AVRCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// If we are processing a touch event and this index matches the initial touch event process movement
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (GetWorld() != nullptr)
		{
			UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
			if (ViewportClient != nullptr)
			{
				FVector MoveDelta = Location - TouchItem.Location;
				FVector2D ScreenSize;
				ViewportClient->GetViewportSize(ScreenSize);
				FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
				if (FMath::Abs(ScaledDelta.X) >= (4.0f / ScreenSize.X))
				{
					TouchItem.bMoved = true;
					float Value = ScaledDelta.X * BaseTurnRate;
					AddControllerYawInput(Value);
				}
				if (FMath::Abs(ScaledDelta.Y) >= (4.0f / ScreenSize.Y))
				{
					TouchItem.bMoved = true;
					float Value = ScaledDelta.Y* BaseTurnRate;
					AddControllerPitchInput(Value);
				}
				TouchItem.Location = Location;
			}
			TouchItem.Location = Location;
		}
	}
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	PhysicsHandleComponent = GetOwner()-> FindComponentByClass<UPhysicsHandleComponent>();
	if(PhysicsHandleComponent)
	{
		// Physics Handle is found
		UE_LOG(LogTemp, Warning, TEXT("%s physics handle component available"), *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s missing physics handle component"), *GetOwner()->GetName());
	}
}

void AVRCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AVRCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AVRCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AVRCharacter::LookUpAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AVRCharacter::TryEnableTouchscreenMovement(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AVRCharacter::BeginTouch);
	PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AVRCharacter::EndTouch);
	PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AVRCharacter::TouchUpdate);	
}

void AVRCharacter::GripLeft()
{
	return;
	UE_LOG(LogTemp, Warning, TEXT("GripLeft"));
	if(bIsGrabbing)
	{
		return;
	}
	
	// HitComponent = GetComponentInSight(LeftHandSphere);
	FHitResult Result = GetComponentInSight(LeftHandSphere);
	HitComponent = Result.GetComponent();
	if(!HitComponent)
	{
		return;
	}
	bIsGrabbing = true;
	HitComponent->SetSimulatePhysics(true);
	PhysicsHandleComponent->GrabComponentAtLocationWithRotation(HitComponent, NAME_None, HitComponent->K2_GetComponentLocation(), FRotator(0,0,0));
	GrabDistance = UKismetMathLibrary::Vector_Distance(HitComponent->K2_GetComponentLocation(), LeftHandSphere->K2_GetComponentLocation());
	
	return;

	UE_LOG(LogTemp, Warning, TEXT("GripLeft"));
	//TryGrab(MotionControllerComponent_L, LeftHandSphere);
	UGrabComponent* NearestGripComponent = GetNearestGripComponent(MotionControllerComponent_L, LeftHandSphere);
	if(NearestGripComponent)
	{
		AttachedElement = NearestGripComponent->GetOwner();
		NearestGripComponent->TryGrab(MotionControllerComponent_L);
		HeldComponentLeft = NearestGripComponent;
		
		if(HeldComponentLeft == HeldComponentRight)
		{
			HeldComponentRight = nullptr;
		}
	}
}

void AVRCharacter::GripRight()
{
	return;
	UE_LOG(LogTemp, Warning, TEXT("GripRight"));
	UGrabComponent* NearestGripComponent = GetNearestGripComponent(MotionControllerComponent_R, RightHandSphere);
	if(NearestGripComponent)
	{
		AttachedElement = NearestGripComponent->GetOwner();
		NearestGripComponent->TryGrab(MotionControllerComponent_R);
		HeldComponentRight = NearestGripComponent;
		
		if(HeldComponentRight == HeldComponentLeft)
		{
			HeldComponentLeft = nullptr;
		}
	}
}

// void AVRCharacter::GripRelease()
// {
// 	// ADial* DialRef = Cast<ADial>(AttachedElement);
// 	// if(DialRef)
// 	// 	DialRef->Release();
// 	if(HeldComponentLeft)
// 	{
// 		HeldComponentLeft->TryRelease();
// 		HeldComponentLeft = nullptr;
// 	}
//
// 	if(HeldComponentRight)
// 	{
// 		HeldComponentRight->TryRelease();
// 		HeldComponentRight = nullptr;
// 	}
// 	
// 	AttachedElement = nullptr;
// 	// UE_LOG(LogMathVR, Log, TEXT("GripRelease() called"));
// }

void AVRCharacter::GripReleaseLeft()
{
	return;
	if(!bIsGrabbing)
	{
		return;
	}
	if(!HitComponent)
		return;
	
	PhysicsHandleComponent->ReleaseComponent();
	bIsGrabbing = false;
	HitComponent->SetSimulatePhysics(false);
	HitComponent = nullptr;
	
	return;
	if(HeldComponentLeft)
	{
		HeldComponentLeft->TryRelease();
		if(HeldComponentLeft->TryRelease())
		{
			HeldComponentLeft = nullptr;
		}
	}
}

void AVRCharacter::GripReleaseRight()
{
	if(HeldComponentRight)
	{
		HeldComponentRight->TryRelease();
		if(HeldComponentRight->TryRelease())
		{
			HeldComponentRight = nullptr;
		}
	}
}

// void AVRCharacter::TryGrab(UMotionControllerComponent* MotionControllerComponent, USphereComponent* Sphere)
// {
// 	TArray<AActor*> OverlappingActors;
// 	Sphere->GetOverlappingActors(OverlappingActors);
//
// 	for(auto Actor : OverlappingActors)
// 	{
// 		if(Actor->ActorHasTag("Dial"))
// 		{
// 			AttachedElement = Actor;
// 			ADial* DialRef = Cast<ADial>(Actor);
// 			if(DialRef)
// 				DialRef->Pick(MotionControllerComponent);
// 				return;
// 		}
// 	}
// }

UGrabComponent* AVRCharacter::GetNearestGripComponent(const UMotionControllerComponent* MotionControllerComponent, USphereComponent* Sphere)
{
	FHitResult OutHit;
	const TArray<AActor*> ActorsToIgnore;
	//ActorsToIgnore.Add(GetOwner());
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));
	UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), MotionControllerComponent->K2_GetComponentLocation(), MotionControllerComponent->K2_GetComponentLocation(),
		6.f, ObjectTypesArray, false, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true, FLinearColor::Gray, FLinearColor::Blue, 1.0f);

	UGrabComponent* LocalNearestGripComponent = nullptr;
	if(OutHit.Actor == nullptr)
	{
		return LocalNearestGripComponent;
	}
	TArray<UActorComponent*> Components = OutHit.Actor->K2_GetComponentsByClass(UGrabComponent::StaticClass());
	
	// if(Components.Num() > 0)
	// {
	for(auto Comp : Components)
	{
		if(Comp)
		{
			UGrabComponent* GripComponent = Cast<UGrabComponent>(Comp);
			float VectorLengthSquared = UKismetMathLibrary::VSizeSquared(UKismetMathLibrary::Subtract_VectorVector(GripComponent->K2_GetComponentLocation(), MotionControllerComponent->K2_GetComponentLocation()));
			if(VectorLengthSquared <= 9999999.0)
			{
				LocalNearestGripComponent = GripComponent;
			}
		}
	}
	//}
	
	return LocalNearestGripComponent;
}

FHitResult AVRCharacter::GetComponentInSight(USphereComponent* SphereComponentRef)
{
	TArray<AActor*> ActorsToIgnore; 
	FHitResult OutHit;
	
	const FVector A = UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::GetForwardVector(SphereComponentRef->K2_GetComponentRotation()), 500);
	const FVector End = UKismetMathLibrary::Add_VectorVector(A, SphereComponentRef->K2_GetComponentLocation());
	
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), SphereComponentRef->K2_GetComponentLocation(), End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, true, FLinearColor::Yellow, FLinearColor::White, 5);

	return OutHit;
	// return nullptr;
}

