// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaneSim.h"


// Sets default values
APlaneSim::APlaneSim()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	_planes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainPlane"));
	RootComponent = _planes;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> planeAsset(TEXT("/Game/Blueprints/TestStaticMesh.TestStaticMesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> flapsSpoilersAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (planeAsset.Succeeded())
	{
		_planes->SetStaticMesh(planeAsset.Object);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, (TEXT("I'M BROKE")));
	}

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->AttachToComponent(_planes, FAttachmentTransformRules::KeepRelativeTransform);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FlightThirdPersonCamera"));
	check(CameraComponent != nullptr);

	CameraComponent->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepRelativeTransform);

	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->TargetArmLength = 300.0f;

	SpringArmComp->SetRelativeLocation(FVector(-960.0, -0.000856, 583.492249));
	CameraComponent->SetRelativeLocation(FVector(-2300.0, 0.0, 30.0));


	_rotationYaw = GetActorRotation().Yaw;
	_rotationRoll = GetActorRotation().Roll;
	_rotationPitch = GetActorRotation().Pitch;

	_planes->SetVisibility(false, false);

	_planes->SetSimulatePhysics(true);
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

	CalculateLiftCoefficient();
	
	
	
	ApplyRotations(DeltaTime);
	
	_dragDirection = CalculateDragDirection();
	
	
	CalculateDrag();
	CalculateFinalLift();
	
	ApplyFinalLift();
	ApplyFinalDrag();

	_deltaTime = DeltaTime;

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
	PlayerInputComponent->BindAxis("Flaps", this, &APlaneSim::Flaps);
	PlayerInputComponent->BindAxis("Spoilers", this, &APlaneSim::Spoilers);
}

inline FVector APlaneSim::CalculateLiftDirection()
{
	auto forward = normalizedForwardVector;

	auto right = UKismetMathLibrary::GetRightVector(GetActorRotation());

	right.Normalize();

	DrawDebugLine(GetWorld(), GetActorLocation(), (GetActorLocation() + (forward * 1000)) , FColor::Purple, false, -1, 0, 20);
	DrawDebugLine(GetWorld(), GetActorLocation(), (GetActorLocation() + (right * 1000)) , FColor::Blue, false, -1, 0, 20);


	if (_isStalling && _isSkidding)
	{
		if (!_skidType)
		{
			//_rotationRoll += 2.0f;
			//_rotationYaw += 2.0f;
			_planes->AddTorque(FVector(1, 0, 1), NAME_None, true);
		
			
		}
		else
		{
			//_rotationRoll -= 2.0f;
			//_rotationYaw -= 2.0f;
			_planes->AddTorque(FVector(-1, 0, -1), NAME_None, true);
		}

		//_finalSpeed = (_finalSpeed / 2.0);
		

		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Purple, FString::Printf(TEXT("You spin me right round, baby, right round")));
		
		
	}

	return UKismetMathLibrary::Cross_VectorVector(forward, right);
}

inline FVector APlaneSim::CalculateDragDirection()
{
	auto vVector = _planes->GetComponentVelocity();

	if (vVector.Normalize())
	{
		vVector = vVector * -1.0f;
	}
	return vVector;
}

inline void APlaneSim::CalculateDrag()
{
	_drag = (_cDMin + ((_liftCoefficient * _liftCoefficient) / ((_aspectRatio * _oswaltEffect) * PI)));
}

void APlaneSim::CalculateLiftCoefficient()
{
	//Stall Version 2, This is a bit more real world accurate
	if (_angleofAttack >= 15 && _angleofAttack <= 360)
	{
		_liftCoefficient = ((_aspectRatio / (_aspectRatio + 2)) * UKismetMathLibrary::DegreesToRadians(_internalAngleofAttack) * (2 * PI));

		//Only resets the Physics Linear Velocity Once a stall occurs
		if (!_isStalling)
		{
			_planes->SetPhysicsLinearVelocity(_planes->GetPhysicsLinearVelocity() * 0.005);
		}
		
		//Plane still likes to fly when trying to emulate a spin so this kind of takes that into account
		if (_angleofAttack >= 40)
		{
			_liftCoefficient = _liftCoefficient * -0.005;
		}
		else if (_angleofAttack >= 25)
		{
			_liftCoefficient = _liftCoefficient * -0.0005;
			
		}
		else
		{
			_liftCoefficient = _liftCoefficient * 0.0005;
		}


		_isStalling = true;
		_planes->SetPhysicsLinearVelocity(_planes->GetPhysicsLinearVelocity().GetClampedToSize(0, 5000.0f));

		//_finalSpeed = _finalSpeed / 4.0f;
		
	}
	else
	{
		//if no stalling occurs we just do the normal lift calcuations
		_liftCoefficient = ((_aspectRatio / (_aspectRatio + 2)) * UKismetMathLibrary::DegreesToRadians(_internalAngleofAttack) * (2 * PI));

		_isStalling = false;
	}

}

inline void APlaneSim::CalculateFinalLift()
{
	_finalLift = (((_finalSpeed * _finalSpeed) * _airDensity) / 2) * _multiplier * _liftCoefficient * _surfaceArea;
}

void APlaneSim::ThrottleControl()
{
	if (_throttleUp)
	{
		_throttle = FMath::Clamp((_throttle + _throttleIncrease), 0.0f, 100.0f);
	}
	else if (_throttleDown)
	{
		_throttle = FMath::Clamp((_throttle - _throttleIncrease), 0.0f, 100.0f);
	}

	_finalSpeed = ((_throttle / 100) * _maxThrust);

	if (_isStalling)
	{
		_planes->AddForce(((normalizedForwardVector) * _finalSpeed) * 10.0f, NAME_None, true);
	}
	else
	{
		_planes->AddForce((normalizedForwardVector * _finalSpeed) * 25.0f, NAME_None, true);
	}

	//This might be causing some issues
	_planes->SetPhysicsLinearVelocity(_planes->GetPhysicsLinearVelocity().GetClampedToSize(0, 7000.0f));
}

void APlaneSim::CalcuateAirDensity(float altitude)
{
	_airDensity = UKismetMathLibrary::Lerp(1.23f, 0.72f, UKismetMathLibrary::NormalizeToRange(altitude, 0.0f, 16000.f));
}

inline void APlaneSim::ApplyRotations(float deltaTime)
{
	auto newRotation = FMath::QInterpTo(_planes->GetRelativeRotation().Quaternion(), FQuat(FRotator(_rotationPitch, _rotationYaw, _rotationRoll)), deltaTime, 2.0f);

	_planes->SetRelativeRotation(newRotation);
}

inline void APlaneSim::ApplyFinalLift()
{
	_planes->AddForce(CalculateLiftDirection() * _finalLift);
}

inline void APlaneSim::ApplyFinalDrag()
{
	_planes->AddForce((_drag * CalculateDragDirection()));
}

void APlaneSim::RotatePitch(float Value)
{
	_rotationPitch = _angleofAttack += (_deltaTime * Value * _pitchYawMultiplier);

	_internalAngleofAttack = (_angleofAttack * 2);


	if (_angleofAttack >= 360 || _angleofAttack <= -360)
	{
		_internalAngleofAttack = _rotationPitch = _angleofAttack = 0.0f;
	}
}

void APlaneSim::RotateYaw(float Value)
{
	_rotationYaw += (_deltaTime * Value * _pitchYawMultiplier);

	if (_rotationYaw >= 360 || _rotationYaw <= -360)
	{
		_rotationYaw = 0.0f;
	}

	AddControllerYawInput(_deltaTime * Value * _pitchYawMultiplier);
}

void APlaneSim::RotateRoll(float Value)
{
	_rotationRoll += (_deltaTime * Value * _rollMultiplier);

	if (_rotationRoll >= 360 || _rotationRoll <= -360)
	{
		_rotationRoll = 0.0f;
	}
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

void APlaneSim::Flaps(float Value)
{
	_flapsDegrees += (_deltaTime * Value * _flapSpoilerMultiplier);

	_flapsDegrees = FMath::Clamp<float>(_flapsDegrees, _minFlapsDegreeAngle, _maxFlapsDegreeAngle);

	_surfaceArea = UKismetMathLibrary::Lerp(166.0f, 200.0f, UKismetMathLibrary::NormalizeToRange(_flapsDegrees, _minFlapsDegreeAngle, _maxFlapsDegreeAngle));
}

void APlaneSim::Spoilers(float Value)
{
	_spoilersDegrees += (_deltaTime * Value * _flapSpoilerMultiplier);
	_spoilersDegrees = FMath::Clamp<float>(_spoilersDegrees, 0.0f, _maxSpoilersDegreeAngle);

	_multiplier = UKismetMathLibrary::Lerp(1.0f, 0.1f, UKismetMathLibrary::NormalizeToRange(_spoilersDegrees, 0.0f, _maxSpoilersDegreeAngle));
}

bool APlaneSim::GroundProximity()
{
	FHitResult hitResult;

	auto actorLocation = GetActorLocation();

	auto endLocation = FVector(actorLocation.X, actorLocation.Y, (actorLocation.Z + -1000000.0));

	auto hit = GetWorld()->LineTraceSingleByChannel(hitResult, actorLocation, endLocation, ECC_Visibility);

	if (hit)
	{
		_groundDistance = hitResult.Distance;

		if (_groundDistance <= 10000.0 && _groundDistance > 100.0)
		{
			_rateofDescent = (_planes->GetPhysicsLinearVelocity().Size() * 5);

			auto diff = _groundDistance - _lastGroundDistanceTick;



			if ((diff < 0.f) && (_rateofDescent >= 1000.0) && ((_angleofAttack < 3.0) != (_angleofAttack > 12.0f)))
			{
				if (_groundDistance <= 5000.0)
				{
					_lastGroundDistanceTick = _groundDistance;
					return true;
				}
				else
				{
					_lastGroundDistanceTick = _groundDistance;
					return false;
				}
			}
			else
			{
				_lastGroundDistanceTick = _groundDistance;
			}

		}
		else
		{
			_lastGroundDistanceTick = _groundDistance;
		}

		_lastGroundDistanceTick = _groundDistance;

	}


	return false;
}

bool APlaneSim::ForwardProximity()
{
	FHitResult hitResult;

	auto actorLocation = GetActorLocation();

	auto endLocation = actorLocation +  ((normalizedForwardVector) * 5000.0);

	auto hit = GetWorld()->SweepSingleByChannel(hitResult, actorLocation, endLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(250.f));

	if (hit)
	{
		if (hitResult.GetComponent()->GetCollisionObjectType() != ECollisionChannel::ECC_EngineTraceChannel1)
		{

			if (_groundDistance > 300.f)
			{
				_timeTillImpact = hitResult.Distance / (_planes->GetPhysicsLinearVelocity().Size());

				if ((_timeTillImpact < 1.5) && (_timeTillImpact > 0.8))
				{
					return true;
				}
			}
		}
	
	}

	return false;
}

bool APlaneSim::TCAS()
{
	FHitResult hitResult;

	auto actorLocation = GetActorLocation();
	auto endLocation = actorLocation + ((normalizedForwardVector) * 30000.0);

	auto hit = GetWorld()->SweepSingleByChannel(hitResult, actorLocation, endLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(2000.0f));

	if (hit)
	{
		if (hitResult.GetComponent()->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel1)
		{

			auto distance = hitResult.Distance;

			if (distance < 30000.0f && distance > 15000.0f)
			{
				_inContact = true;
			}

			if (distance <= 15000.0f && distance > 0.0f)
			{
				auto origin = hitResult.GetComponent()->GetComponentLocation();


				auto difference = actorLocation - origin;

				if (difference.Z < 0.0f)
				{
					return true;
				}
				else
				{
					return false;
				}

			}
			else if (_inContact)
			{
				return false;
			}

		}
	}
	return false;
}

