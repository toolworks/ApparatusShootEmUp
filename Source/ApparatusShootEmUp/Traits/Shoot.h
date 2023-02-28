#pragma once
 
#include "CoreMinimal.h"
#include "Math/Vector.h"

#include "Shoot.generated.h"


/**
 * The attack command for the player.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FShoot
{
	GENERATED_BODY()
 
  public:
 
	/**
	 * The direction to launch to.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Direction = FVector::ZeroVector;

	/**
	 * The projectile template to spawn.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSubjectRecord ProjectileRecord;
};
