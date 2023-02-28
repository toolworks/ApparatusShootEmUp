#pragma once
 
#include "CoreMinimal.h"
#include "Enemy.generated.h"
 
/**
 * The main enemy trait.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FEnemy
{
	GENERATED_BODY()
 
  public:

	/**
	 * The identifier of the enemy kind.
	 * This is set automatically.
	 */
	UPROPERTY(VisibleAnywhere);
	int32 KindId = 0;

	/**
	 * The amount of score to add for killing this enemy.
	 */
	UPROPERTY(EditAnywhere);
	int32 Score = 100;

	/**
	 * The time left since choosing the next roaming direction
	 */
	UPROPERTY(VisibleAnywhere);
	float RoamingTimeout = 0.0f;
};
