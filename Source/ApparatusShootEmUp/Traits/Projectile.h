#pragma once
 
#include "CoreMinimal.h"
#include "Projectile.generated.h"
 
/**
 * The main projectile trait.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FProjectile
{
	GENERATED_BODY()
 
  public:

	/**
	 * The projectile's radius.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Radius = 25;

	/**
	 * The current living time of the projectile.
	 */
	float Time = 0;
};
