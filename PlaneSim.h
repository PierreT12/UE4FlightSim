// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Rotator.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/Rotator.h"

#include "GameFramework/Pawn.h"
#include "PlaneSim.generated.h"

#define normalizedForwardVector UKismetMathLibrary::GetForwardVector(GetActorRotation())

UCLASS()
class FLIGHT_SIM_API APlaneSim : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlaneSim();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Functions
private:

	FVector CalcLiftDirection();
	FVector CalcDragDirection();

	void CalcDrag();

	void CalcLiftCoefficient();

	void CalcFinalLift();

	void ThrottleControl();

	void ApplyRotations(float deltaTime);


	void ApplyFinalLift();

	void ApplyFinalDrag();



	//Variables
private:

	

	//Throttle Variables
	float _throttle{ 0.0f };
	bool _throttleUp{ false };
	bool _throttleDown{ false };
	const float _throttleIncrease{ 1.0f };
	const float _maxThrust{ 335.0f };
	bool _thrustSaftey;

	

	FVector _forwardNormalized;
	FVector _rightNormalized;
	

	FVector _relativeVelocity;

	float _aspectRatio{ 77.800003f };

	float _liftCoefficient{ 0.0f };

	const float _surfaceArea{ 166.0f };

	float _finalLift{ 0.0f };

	float _drag{ 0.0f };

	const float _cDMin{ 0.027f };

	float _oswaltEffect{ 0.81678f };

	float _multiplier{ 1.0f };

	float _baseAirDensity{ 1.2754f };


	FVector _dragDirection;
	FVector _liftDirection;

	float _inducedDrag{ 0.0f };


	float _rotationPitch{ 0.0f };
	float _rotationYaw{ 0.0f };


	float _rate{ 0.0f };
	
	
	float _finalSpeed{ 0.0f };

	float _airControl{ 0.0f };

	float _flapsDegrees{ 0.0f };

	float _spoilerDegrees{ 0.0f };


	public:

		UFUNCTION()
			void RotatePitch(float Value);

		UFUNCTION()
			void RotateYaw(float Value);

		UFUNCTION()
			void RotateRoll(float Value);

		UFUNCTION()
			void ThrottleUp();

		UFUNCTION()
			void ThrottleDown();


		UFUNCTION()
			void ThrottleUpStop();

		UFUNCTION()
			void ThrottleDownStop();



		UPROPERTY(VisibleAnywhere)
			UCameraComponent* CameraComponent;
	
		UPROPERTY(VisibleAnywhere)
			USpringArmComponent* SpringArmComp;

		UPROPERTY(VisibleAnywhere)
			UStaticMeshComponent* _plane;


		UPROPERTY(BlueprintReadOnly)
			float _angleofAttack{ 0.0f };

		UPROPERTY(BlueprintReadOnly)
			float _rotationRoll{ 0.0f };
};
