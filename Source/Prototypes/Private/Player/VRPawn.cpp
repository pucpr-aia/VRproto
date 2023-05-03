#include "Player/VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AVRPawn::AVRPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	RootComponent = SceneComponent;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(RootComponent);

	MotionControllerComponent_R = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_R"));
	MotionControllerComponent_R->SetupAttachment(RootComponent);
	MotionControllerComponent_L = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_L"));
	MotionControllerComponent_L->SetupAttachment(RootComponent);

	WidgetInteractionComponent_L = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction_L"));
	WidgetInteractionComponent_L->SetupAttachment(MotionControllerComponent_L);
	WidgetInteractionComponent_R = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction_R"));
	WidgetInteractionComponent_R->SetupAttachment(MotionControllerComponent_R);

	RightMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("R"));
	LeftMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("L"));
	RightMesh->SetupAttachment(MotionControllerComponent_R);
	LeftMesh->SetupAttachment(MotionControllerComponent_L);
	
	RightHandSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Right Hand Sphere"));
	LeftHandSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Left Hand Sphere"));

	RightHandSphere->SetupAttachment(MotionControllerComponent_R);
	LeftHandSphere->SetupAttachment(MotionControllerComponent_L);

	RightHandSphere->SetSphereRadius(15.f);
	LeftHandSphere->SetSphereRadius(15.f);

	RightHandSphere->ComponentTags.Add(FName("Hand"));
	LeftHandSphere->ComponentTags.Add(FName("Hand"));
}

// Called when the game starts or when spawned
void AVRPawn::BeginPlay()
{
	Super::BeginPlay();
	
    // if(UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
    // {
    //     UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
    // }
}

// Called every frame
void AVRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, this, &AVRPawn::GripLeft);
	PlayerInputComponent->BindAction("GrabLeft", IE_Released, this, &AVRPawn::GripReleaseLeft);
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, this, &AVRPawn::GripRight);
	PlayerInputComponent->BindAction("GrabRight", IE_Released, this, &AVRPawn::GripReleaseRight);
}

void AVRPawn::IncrementScore() { Score++; }

void AVRPawn::DecrementScore(){ Score--; }

int32 AVRPawn::GetScore() const { return Score; }

void AVRPawn::GripLeft()
{
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
			
		//UE_LOG(LogMathVR, Log, TEXT("Nearest Grip Component not null"));
	}
}

void AVRPawn::GripRight()
{
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
			
		//UE_LOG(LogMathVR, Log, TEXT("Nearest Grip Component not null"));
	}
}

void AVRPawn::GripReleaseLeft()
{
	UE_LOG(LogTemp, Warning, TEXT("GripLeft"));
	// UE_LOG(LogMathVR, Log, TEXT("GripReleaseLeft() called"));
	if(HeldComponentLeft)
	{
		HeldComponentLeft->TryRelease();
		if(HeldComponentLeft->TryRelease())
		{
			HeldComponentLeft = nullptr;
		}
	}
}

void AVRPawn::GripReleaseRight()
{
	// UE_LOG(LogMathVR, Log, TEXT("GripReleaseRight() called"));
	if(HeldComponentRight)
	{
		HeldComponentRight->TryRelease();
		if(HeldComponentRight->TryRelease())
		{
			HeldComponentRight = nullptr;
		}
	}
}

UGrabComponent* AVRPawn::GetNearestGripComponent(UMotionControllerComponent* MotionControllerComponent,
	USphereComponent* Sphere)
{
	FHitResult OutHit;
	TArray<AActor*> ActorsToIgnore;
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

