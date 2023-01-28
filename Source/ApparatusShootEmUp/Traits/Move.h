#pragma once
 
#include "CoreMinimal.h"
#include "Math/Vector.h"

#include "Move.generated.h"
 
/**
 * The move command.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FMove
{
	GENERATED_BODY()
 
  public:
 
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Velocity = FVector::ZeroVector;
};
