#pragma once
 
#include "CoreMinimal.h"
#include "Speed.generated.h"
 
/**
 * The movement speed factor.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FSpeed
{
	GENERATED_BODY()
 
  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Value = 50;
};
