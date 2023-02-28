#pragma once
 
#include "CoreMinimal.h"

#include "Shoots.generated.h"


/**
 * The shooting ability.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FShoots
{
	GENERATED_BODY()
 
  public:

	/**
	 * The minimal interval between shots in seconds.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Interval = 0.25f;

	/**
	 * The shot's timeout to the next shot possibility.
	 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	float Timeout = 0.0f;
};
