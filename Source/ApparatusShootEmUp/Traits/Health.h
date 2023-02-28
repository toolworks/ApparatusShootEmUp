#pragma once
 
#include "CoreMinimal.h"
#include "Health.generated.h"
 
/**
 * The health level of the character.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FHealth
{
	GENERATED_BODY()
 
  public:

	/**
	 * The current health value.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Value = 100;

	/**
	 * The maximum health value.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Maximum = 100;
};
