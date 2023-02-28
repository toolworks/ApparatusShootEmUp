#pragma once
 
#include "CoreMinimal.h"
#include "Dying.generated.h"
 
/**
 * The state of dying.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FDying
{
	GENERATED_BODY()
 
  public:

	/**
	 * The current time of the dying process in seconds.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Time = 0.0f;
};
