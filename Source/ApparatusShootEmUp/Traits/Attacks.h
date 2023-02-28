#pragma once
 
#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"

#include "Attacks.generated.h"
 
/**
 * The ability to do a melee attack.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FAttacks
{
	GENERATED_BODY()
 
  public:

	/**
	 * The animation curve for the attack.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRuntimeFloatCurve Animation;

	/**
	 * The attack range.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Range = 50.0f;

	/**
	 * The damage dealt by an attack.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Damage = 10;

	/**
	 * The exact attack frame time.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FrameTime = 0.5f;
};
