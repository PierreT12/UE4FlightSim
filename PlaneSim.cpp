// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaneSim.h"

// Sets default values
APlaneSim::APlaneSim()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	_plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
	RootComponent = _plane;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> planeAsset(TEXT("/Game/Blueprints/TestStaticMesh.TestStaticMesh"));

	if (planeAsset.Succeeded())
	{
		_plane->SetStaticMesh(planeAsset.Object);
	}

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	//SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->AttachToComponent(_plane, FAttachmentTransformRules::KeepRelativeTransform);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FlightThirdPersonCamera"));
	check(CameraComponent != nullptr);

	CameraComponent->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepRelativeTransform);

	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->TargetArmLength = 300.0f;

	SpringArmComp->SetRelativeLocation(FVector(-960.0, -0.000856, 583.492249));
	CameraComponent->SetRelativeLocation(FVector(-2300.0, 0.0, 30.0));

	_plane->SetSimulatePhysics(true);

}

// Called when the game starts or when spawned
void APlaneSim::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlaneSim::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ThrottleControl();

	ApplyRotations(DeltaTime);

	//_liftDirection = CalcLiftDirection();

	_dragDirection = CalcDragDirection();

	CalcLiftCoefficient();
	CalcDrag();
	CalcFinalLift();

	ApplyFinalLift();

	ApplyFinalDrag();

}

// Called to bind functionality to input
void APlaneSim::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("RotatePitch", this, &APlaneSim::RotatePitch);
	PlayerInputComponent->BindAxis("RotateYaw", this, &APlaneSim::RotateYaw);
	PlayerInputComponent->BindAxis("RotateRoll", this, &APlaneSim::RotateRoll);
	PlayerInputComponent->BindAxis("Turn", this, &APlaneSim::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APlaneSim::AddControllerPitchInput);
	PlayerInputComponent->BindAction("ThrottleUp", IE_Pressed, this, &APlaneSim::ThrottleUp);
	PlayerInputComponent->BindAction("ThrottleUp", IE_Released, this, &APlaneSim::ThrottleUpStop);
	PlayerInputComponent->BindAction("ThrottleDown", IE_Pressed, this, &APlaneSim::ThrottleDown);
	PlayerInputComponent->BindAction("ThrottleDown", IE_Released, this, &APlaneSim::ThrottleDownStop);
}

FVector APlaneSim::CalcLiftDirection()
{
	auto forward = normalizedForwardVector;

	auto right = UKismetMathLibrary::GetRightVector(GetActorRotation());

	right.Normalize();

	DrawDebugLine(GetWorld(), GetActorLocation(), (GetActorLocation() + (forward * 1000)) , FColor::Purple, false, -1, 0, 20);
	DrawDebugLine(GetWorld(), GetActorLocation(), (GetActorLocation() + (right * 1000)) , FColor::Blue, false, -1, 0, 20);



	return UKismetMathLibrary::Cross_VectorVector(forward, right);
}

FVector APlaneSim::CalcDragDirection()
{
	auto vVector = _plane->GetComponentVelocity();

	if (vVector.Normalize())
	{
		vVector = vVector * -1.0f;
	}
	//GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Cyan, FString::Printf(TEXT("My Drag Direction is: %s"), *vVector.ToString()));

	return vVector;
}

inline void APlaneSim::CalcDrag()
{
	_drag = (_cDMin + ((_liftCoefficient * _liftCoefficient) / ((_aspectRatio * _oswaltEffect) * PI)));
}

void APlaneSim::CalcLiftCoefficient()
{
	_liftCoefficient = ((_aspectRatio / (_aspectRatio + 2)) * UKismetMathLibrary::DegreesToRadians(_angleofAttack) * (2 * PI));

	//GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("My Nu Aoa is: %f"), nuAoA));
}

void APlaneSim::CalcFinalLift()
{
	_finalLift = (((_finalSpeed * _finalSpeed) * _baseAirDensity) / 2) * _multiplier * _liftCoefficient * _surfaceArea;

	GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("My Final Lift is: %f"), _finalLift));
}

void APlaneSim::ThrottleControl()
{
	if (_throttleUp)
	{
		_throttle = FMath::Clamp((_throttle + _throttleIncrease), 0.0f, 50.0f);
	}
	else if (_throttleDown)
	{
		_throttle = FMath::Clamp((_throttle - _throttleIncrease), 0.0f, 50.0f);
	}

	_finalSpeed = ((_throttle / 100) * _maxThrust);

	

	auto forward = normalizedForwardVector;

	_plane->AddForce((forward * _finalSpeed) * 25.0f, NAME_None, true);


	_plane->SetPhysicsLinearVelocity(_plane->GetPhysicsLinearVelocity().GetClampedToSize(0, 5000.0f));
}

void APlaneSim::ApplyRotations(float deltaTime)
{
	auto relaviveQuat = _plane->GetRelativeRotation().Quaternion();

	auto targetQuat = FQuat(FRotator(_rotationPitch, _rotationYaw, _rotationRoll));

	auto newRotation = FMath::QInterpTo(relaviveQuat, targetQuat, deltaTime, 2.0f);

	_plane->SetRelativeRotation(newRotation);


}

void APlaneSim::ApplyFinalLift()
{
	GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("My Weight is: %f"), _plane->GetMass() * -980.f));

	auto vector = CalcLiftDirection();
	_plane->AddForce(vector * _finalLift);

	//GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, FString::Printf(TEXT("\n My lift direction is: %s"), *vector.ToString()));

}

void APlaneSim::ApplyFinalDrag()
{
	_plane->AddForce((_drag * CalcDragDirection()));
}

void APlaneSim::RotatePitch(float Value)
{
	_rotationPitch = _angleofAttack += (Value * 0.25);

	GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Purple, FString::Printf(TEXT("My Angle of attack is: %f"), _angleofAttack));
}

void APlaneSim::RotateYaw(float Value)
{
	_rotationYaw += (Value * 0.75);
}

void APlaneSim::RotateRoll(float Value)
{
	_rotationRoll += (Value * 1.2);
}

void APlaneSim::ThrottleUp()
{
	_throttleUp = true;
}

void APlaneSim::ThrottleDown()
{
	_throttleDown = true;
}

void APlaneSim::ThrottleUpStop()
{
	_throttleUp = false;
}

void APlaneSim::ThrottleDownStop()
{
	_throttleDown = false;
}

