#pragma once
 
#include "CoreMinimal.h"
#include "Appearing.generated.h"
 
/**
 * The state of appearing in game.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FAppearing
{
	GENERATED_BODY()
 
  public:

	/**
	 * The current time of appearing.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Time = 0.0f;
};
