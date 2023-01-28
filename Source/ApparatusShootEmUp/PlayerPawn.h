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

	UPROPERTY(VisibleDefaultsOnly, Category = Data)
	USubjectiveActorComponent* Subjective;

	/**
	 * Accumulated movement direction.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, Meta = (AllowPrivateAccess = "true"))
	FVector2D MovementDirection;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float Speed = 100;

protected:

	void ApplyMovement();

	void MoveRight(float Value);

	void MoveUp(float Value);
	
#pragma region APawn Interface
	void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
#pragma endregion APawn Interface

	void BeginPlay() override;

public:

	APlayerPawn();
};
