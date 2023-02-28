#pragma once
 
#include "CoreMinimal.h"
#include "Attacking.generated.h"
 
/**
 * The state of making an attack.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FAttacking
{
	GENERATED_BODY()
 
  public:

	/**
	 * The current attack time.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Time = 0.0f;

	/**
	 * Was the attack actually performed.
	 */
	mutable bool bExecuted = false;
};
