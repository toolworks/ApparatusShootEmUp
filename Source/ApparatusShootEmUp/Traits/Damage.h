#pragma once
 
#include "CoreMinimal.h"
#include "Damage.generated.h"
 
/**
 * The damage being dealt.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FDamage
{
	GENERATED_BODY()
 
  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Value = 10;
};
