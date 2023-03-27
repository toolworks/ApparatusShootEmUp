#pragma once
 
#include "CoreMinimal.h"
#include "Hit.generated.h"
 
/**
 * The state of being hit by a projectile.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FHit
{
	GENERATED_BODY()
 
  public:

	/**
	 * The elapsed time being hit.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Time = 0.0f;

	/**
	 * The velocity to knock back with.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Velocity = FVector::ZeroVector;
};
