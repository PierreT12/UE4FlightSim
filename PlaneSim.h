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
#include "Components/StaticMeshComponent.h"


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

	/// <summary>
	/// Gets the Lift Direction
	/// </summary>
	/// <returns> Lift Direction as a FVector </returns>
	FVector CalculateLiftDirection();

	/// <summary>
	/// Gets the Drag Direction
	/// </summary>
	/// <returns> Returns the Drag Direciton as an FVector </returns>
	FVector CalculateDragDirection();

	/// <summary>
	/// Calcuates the Drag
	/// </summary>
	void CalculateDrag();

	/// <summary>
	/// Calculates the Lift Coefficient
	/// </summary>
	void CalculateLiftCoefficient();

	/// <summary>
	/// Calculates the Final Lift based off of the previous 
	/// </summary>
	void CalculateFinalLift();

	/// <summary>
	/// Calculates the Plane's throttle based on what it's set to
	/// </summary>
	void ThrottleControl();

	/// <summary>
	/// Applies rotation to the plane
	/// </summary>
	/// <param name="deltaTime"> The current delta time for this tick</param>
	void ApplyRotations(float deltaTime);


	/// <summary>
	/// Does final Calculations and Applies Lift
	/// </summary>
	void ApplyFinalLift();

	/// <summary>
	/// Does final Calculations and Applies Drag
	/// </summary>
	void ApplyFinalDrag();



	//Variables
private:

	

	float _throttle{ 0.0f };
	bool _throttleUp{ false };
	bool _throttleDown{ false };
	const float _throttleIncrease{ 1.0f };
	const float _maxThrust{ 335.0f };
	bool _thrustSaftey;

	
	FVector _forwardNormalized;
	FVector _rightNormalized;
	FVector _relativeVelocity;

	const float _aspectRatio{ 77.800003f };


	float _internalAngleofAttack{ 0.0f };

	

	float _surfaceArea{ 166.0f };
	float _drag{ 0.0f };
	const float _cDMin{ 0.027f };
	const float _oswaltEffect{ 0.81678f };
	float _multiplier{ 1.0f };

	


	FVector _dragDirection;
	FVector _liftDirection;

	float _inducedDrag{ 0.0f };
	float _rotationPitch{ 0.0f };
	float _rate{ 0.0f };
	float _airControl{ 0.0f };

	

	float _deltaTime{ 0.0f };
	float _pressureAltitude{ 0.0f };
	float _stallChangeSpeed{ 220.f};


	float _isStalling{ false };
	float _stalllift{ 0.0f };


	float _rateofDescent{ 0.0f };
	float _lastGroundDistanceTick{ 0.0f };
	float _timeTillImpact{ 0.0f };
	bool _addTouqueOnce{ false };


	public:

		UFUNCTION()
			/// <summary>
			/// Controller Input for Pitch
			/// </summary>
			/// <param name="Value"> 0 - 1 of the controllers input</param>
			void RotatePitch(float Value);

		UFUNCTION()
			/// <summary>
			/// Controller Input for Yaw
			/// </summary>
			/// <param name="Value"> 0 - 1 of the controllers input</param>
			void RotateYaw(float Value);

		UFUNCTION()
			/// <summary>
			/// Controller Input for Roll
			/// </summary>
			/// <param name="Value"> 0 - 1 of the controllers input</param>
			void RotateRoll(float Value);

		UFUNCTION()
			void ThrottleUp();

		UFUNCTION()
			void ThrottleDown();


		UFUNCTION()
			void ThrottleUpStop();

		UFUNCTION()
			void ThrottleDownStop();

		UFUNCTION()
			/// <summary>
			/// Controller Input for Flaps
			/// </summary>
			/// <param name="Value"> 0 - 1 of the controllers input</param>
			void Flaps(float Value);

		UFUNCTION()
			/// <summary>
			/// Controller Input for Spoilers
			/// </summary>
			/// <param name="Value"> 0 - 1 of the controllers input</param>
			void Spoilers(float Value);


		UFUNCTION(BlueprintCallable)
			/// <summary>
			/// Finds if the plane is about to collide with the ground
			/// </summary>
			/// <returns> True or false depending on if will or not, and the rest is handled in blueprints</returns>
			bool GroundProximity();

		UFUNCTION(BlueprintCallable)
			/// <summary>
			/// Finds if the plane is about to collide with another object (not a plane)
			/// </summary>
			/// <returns> A true or false depending if it is, and the rest is handled in Blueprints</returns>
			bool ForwardProximity();

		UFUNCTION(BlueprintCallable)
			/// <summary>
			/// Does the TCAS Calulations
			/// </summary>
			/// <returns> A true of false for if the plane is in the airspace of another, the rest is handled in blueprints due to UI</returns>
			bool TCAS();

		UFUNCTION(BlueprintCallable)
			/// <summary>
			/// Finds the air desnisty givent the altitude
			/// </summary>
			/// <param name="altitude">The current altitude of the plane</param>
			void CalcuateAirDensity(float altitude);


		UPROPERTY(VisibleAnywhere)
			UCameraComponent* CameraComponent;
	
		UPROPERTY(VisibleAnywhere)
			USpringArmComponent* SpringArmComp;

		UPROPERTY(BlueprintReadOnly)
			UStaticMeshComponent* _planes;


		UPROPERTY(BlueprintReadOnly)
			float _angleofAttack{ 0.0f };

		UPROPERTY(BlueprintReadOnly)
			float _rotationRoll{ 0.0f };

		UPROPERTY(BlueprintReadOnly)
			float _rotationYaw{ 0.0f };


		UPROPERTY(BlueprintReadOnly)
			float _finalSpeed{ 0.0f };


		UPROPERTY(BlueprintReadOnly)
			float _finalLift{ 0.0f };


		UPROPERTY(BlueprintReadWrite)
			float _liftCoefficient{ 0.0f };

		UPROPERTY(BlueprintReadWrite)
			float _minFlapsDegreeAngle{ 0.0f };

		UPROPERTY(BlueprintReadWrite)
			float _maxFlapsDegreeAngle{ 0.0f };

		UPROPERTY(BlueprintReadWrite)
			float _minSpoilersDegreeAngle{ 0.0f };

		UPROPERTY(BlueprintReadWrite)
			float _maxSpoilersDegreeAngle{ 0.0f };

		UPROPERTY(BlueprintReadWrite)
			float _flapSpoilerMultiplier{ 10.0f };

		UPROPERTY(BlueprintReadWrite)
			bool _isSkidding{false};

		UPROPERTY(BlueprintReadWrite)
			bool _skidType{ false };


		UPROPERTY(BlueprintReadWrite)
			float _rollMultiplier{ 150.0f };

		UPROPERTY(BlueprintReadWrite)
			float _pitchYawMultiplier{ 25.0f };

		UPROPERTY(BlueprintReadWrite)
			float _airDensity{ 0.0f };


		UPROPERTY(BlueprintReadWrite)
		float _flapsDegrees{ 0.0f };

		UPROPERTY(BlueprintReadWrite)
		float _spoilersDegrees{ 0.0f };


		UPROPERTY(BlueprintReadOnly)
			float _groundDistance{ 0.0f };

		UPROPERTY(BlueprintReadOnly)
			bool _inContact{ false };


};
