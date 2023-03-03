// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Curves/IntegralCurve.h"

#include "MechanicalGameModeBase.h"
#include "Apparatist/Singleton.h"
#include "Scaled.h"
#include "Directed.h"
#include "Rendering.h"
#include "TraitRendererComponent.h"

#include "Located.h"
#include "BubbleCage.h"
#include "Traits/Move.h"
#include "Traits/Shoots.h"
#include "Traits/Shoot.h"
#include "Traits/PlayerTrait.h"
#include "Traits/Enemy.h"
#include "Traits/JustSpawned.h"
#include "Traits/Appearing.h"
#include "Traits/Projectile.h"
#include "Traits/Speed.h"
#include "Traits/Health.h"
#include "Traits/Damage.h"
#include "Traits/Hit.h"
#include "Traits/Dying.h"
#include "Traits/Attacks.h"
#include "Traits/Attacking.h"
#include "PlayerPawn.h"
#include "EnemySpawner.h"

#include "ApparatusShootEmUpGameModeBase.generated.h"


DECLARE_STATS_GROUP(TEXT("ApparatusShootEmUp"), STATGROUP_ApparatusShootEmUp, STATCAT_Advanced);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Main Mechanics"),
						  STAT_ShootEmUpMain,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("After Spawn"),
						  STAT_ShootEmUpAfterSpawn,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Appearance"),
						  STAT_ShootEmUpAppearance,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Shooting"),
						  STAT_ShootEmUpShooting,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Movement"),
						  STAT_ShootEmUpMovement,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Projectile Logic"),
						  STAT_ShootEmUpProjectilesLogic,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Be Hit Logic"),
						  STAT_ShootEmUpBeHit,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Dying Logic"),
						  STAT_ShootEmUpDying,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Decouple Spheres"),
						  STAT_ShootEmUpDecouple,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Enemies Reaching"),
						  STAT_ShootEmUpEnemiesReaching,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Health Regeneration"),
						  STAT_ShootEmUpHealthRegeneration,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Melee Attack"),
						  STAT_ShootEmUpMeleeAttack,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Gravity Logic"),
						  STAT_ShootEmUpGravity,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Render State Update"),
						  STAT_ShootEmUpUpdateRenderState,
						  STATGROUP_ApparatusShootEmUp,
						  APPARATUSSHOOTEMUP_API);

/**
 * The main mechanical game mode.
 */
UCLASS()
class APPARATUSSHOOTEMUP_API AApparatusShootEmUpGameModeBase
  : public AMechanicalGameModeBase
{
	GENERATED_BODY()

  public:

	/**
	 * The dying flag for the character.
	 */
	static constexpr EFlagmarkBit DyingFlag = EFlagmarkBit::D;

	/**
	 * The index of the attack custom data parameter of the enemies.
	 */
	static constexpr int32 MaterialAttackDataIndex = 0;

	/**
	 * The index of the hit custom data parameter of the enemies.
	 */
	static constexpr int32 MaterialHitDataIndex = 1;

	/**
	 * The index of the dissolve custom data parameter of the enemies.
	 */
	static constexpr int32 MaterialDissolveDataIndex = 2;

	/**
	 * The index of the general scaling data of the main UV channel.
	 */
	static constexpr int32 MaterialUvScaleDataIndex = 3;

	/**
	 * The index of the opacity data for the projectiles.
	 */
	static constexpr int32 ProjectileMaterialOpacityDataIndex = 0;

	/**
	 * The number of threads for concurrent operations.
	 */
	UPROPERTY(EditAnywhere, Category = Performance)
	int32 ThreadsCount = 4;

	/**
	 * The amount of dissolution by time.
	 */
	UPROPERTY(EditAnywhere, Category = Effects)
	FRuntimeFloatCurve DissolutionByTime;

	/**
	 * The amount of dissolution by time when the character is appearing.
	 */
	UPROPERTY(EditAnywhere, Category = Effects)
	FRuntimeFloatCurve AppearanceDissolutionByTime;

	/**
	 * The amount of emission when the enemy is being hit.
	 */
	UPROPERTY(EditAnywhere, Category = Effects)
	FRuntimeFloatCurve EnemyHitEmissionByTimeout;

	/**
	 * The gravity speed for the bubble spheres.
	 */
	UPROPERTY(EditAnywhere, Category = Physics)
	float Gravity = 200;

	/**
	 * The projectile opacity of time.
	 */
	UPROPERTY(EditAnywhere, Category = Projectiles)
	FRuntimeFloatCurve ProjectileOpacityByTime;

	/**
	 * The number of sub-steps for the projectile shooting model.
	 * 
	 * We need sub-stepping here, cause the bullets move really fast.
	 */
	UPROPERTY(EditAnywhere, Category = Projectiles)
	float ProjectileSubSteps = 4;

	/**
	 * The relative angle by which additional bullet streams should be launched.
	 */
	UPROPERTY(EditAnywhere, Category = Projectiles)
	float AdditionalShootingStreamAngle = 10;

	/**
	 * The number of additional bullet lines by
	 * the current number of enemies.
	 */
	UPROPERTY(EditAnywhere, Category = Projectiles)
	FIntegralCurve AdditionalShootingStreamByEnemiesCount;

	/**
	 * The radius of enemy aggression in relation
	 * to the current enemy count.
	 */
	UPROPERTY(EditAnywhere, Category = Enemies)
	FRuntimeFloatCurve AggressionRadiusByEnemiesCount;

	/**
	 * The roaming influence towards the player in relation
	 * to the current enemy count.
	 */
	UPROPERTY(EditAnywhere, Category = Enemies)
	FRuntimeFloatCurve RoamingToTargetInfluenceByEnemiesCount;

	/**
	 * The number of health points restored per seconds.
	 */
	UPROPERTY(EditAnywhere, Category = Player)
	int32 PlayerHealthRegenPerSecond = 5;

	/**
	 * The current player score.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	int32 Score = 0;

	/**
	 * Get the current instance of the game mode.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static AApparatusShootEmUpGameModeBase*
	GetInstance()
	{
		return Instance;
	}

  protected:

	static AApparatusShootEmUpGameModeBase* Instance;

	float PlayerHealthRegenTimeout = 1.0f;

	void
	BeginPlay() override
	{
		Super::BeginPlay();

		Instance = this;
	}

	void
	EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		if (Instance == this)
		{
			Instance = nullptr;
		}

		Super::EndPlay(EndPlayReason);
	}

#pragma warning(disable: 4883) // Very large method.
	void
	Tick(float DeltaTime) override;

 public:

	AApparatusShootEmUpGameModeBase()
	{
		DefaultPawnClass = APlayerPawn::StaticClass();
	}
};
