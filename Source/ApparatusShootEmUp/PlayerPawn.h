#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"

#include "SubjectiveActorComponent.h"

#include "PlayerPawn.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UCharacterMovementComponent;

UCLASS()
class APPARATUSSHOOTEMUP_API APlayerPawn : public APawn
{
	GENERATED_BODY()

  private:

	/**
	 * The subjective of the player pawn.
	 */
	UPROPERTY(VisibleDefaultsOnly, Category = Data)
	USubjectiveActorComponent* Subjective = nullptr;

	/**
	 * The camera of the game.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess))
	UCameraComponent* CameraComponent = nullptr;

	/**
	 * Accumulated movement direction.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, Meta = (AllowPrivateAccess))
	FVector2D MovementDirection;

	/**
	 * Accumulated shooting direction.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attacking, Meta = (AllowPrivateAccess))
	FVector2D ShootingDirection;

	/**
	 * The amount of moving the camera in relation to the current enemy count.
	 */
	UPROPERTY(EditAnywhere, Category = Camera, Meta = (AllowPrivateAccess))
	FRuntimeFloatCurve ZoomScaleByEnemiesCount;

	FVector OriginalCameraRelativeLocation;

	/**
	 * The base movement speed of the pawn.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float Speed = 100;

	/**
	 * The main projectile prefab.
	 */
	UPROPERTY(EditAnywhere, Category = Attacking)
	FSubjectRecord ProjectileRecord;

  protected:

	void ApplyMovement();

	void ApplyShooting();
	
#pragma region APawn Interface
	void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
#pragma endregion APawn Interface

#pragma region Action Handling

	void MoveRight(float Value);

	void MoveUp(float Value);

	void ShootRight(float Value);

	void ShootUp(float Value);

#pragma endregion Action Handling

#pragma region Actor Interface

	void BeginPlay() override;

	void Tick(float DeltaSeconds) override;

#pragma endregion Actor Interface

public:

	APlayerPawn();
};
