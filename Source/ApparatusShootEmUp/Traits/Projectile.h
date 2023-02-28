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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Radius = 25;
};
