#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector2D.h"
#include "Curves/CurveFloat.h"
#include "Located.h"
#include "BubbleSphere.h"

#include "Traits/Enemy.h"

#include "EnemySpawner.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UCharacterMovementComponent;

/**
 * The enemy configuration entry.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FEnemyConfig
{
    GENERATED_BODY()
 
  public:

	UPROPERTY(EditAnywhere, Category = Enemy)
	float Scale = 1;

	UPROPERTY(EditAnywhere, Category = Enemy)
	float Speed = 100;
};


UCLASS()
class APPARATUSSHOOTEMUP_API AEnemySpawner
  : public AActor
{
	GENERATED_BODY()

	float Time = 0;

	float EnemyBudget = 0;

  public:

	UPROPERTY(EditAnywhere, Category = Enemies)
	float BaseEnemyRadius = 100;

	UPROPERTY(EditAnywhere, Category = Spawning)
	float SpawnRadius = 10000;

	/**
	 * The amount of budget raise per second.
	 */
	UPROPERTY(EditAnywhere, Category = Spawning)
	FRuntimeFloatCurve BudgetGain;

	FVector2D
	VRand2D()
	{
		FVector2D Result;
		float Length;

		do
		{
			Result.X = FMath::FRand() * 2.0f - 1.0f;
			Result.Y = FMath::FRand() * 2.0f - 1.0f;
			Length = Result.SizeSquared();
		}
		while ((Length > 1.0f) || (Length < KINDA_SMALL_NUMBER));

		return Result * (1.0f / FMath::Sqrt(Length));
	}

	void
	Tick(float DeltaTime) override
	{
		Super::Tick(DeltaTime);

		const auto Mechanism = UMachine::ObtainMechanism(GetWorld());
		const auto Curve = BudgetGain.GetRichCurveConst();

		EnemyBudget += Curve->Eval(Time) * DeltaTime;

		for (int32 i = 0; i < (int)EnemyBudget; ++i)
		{
			const float EnemyRadius = BaseEnemyRadius;
			const auto SpawnPoint2D = FVector(VRand2D() * SpawnRadius, EnemyRadius);

			Mechanism->SpawnSubject(FLocated(SpawnPoint2D), FBubbleSphere(EnemyRadius), FEnemy{});

			EnemyBudget -= 1;
		}

		Time += DeltaTime;
	}

	AEnemySpawner();
};