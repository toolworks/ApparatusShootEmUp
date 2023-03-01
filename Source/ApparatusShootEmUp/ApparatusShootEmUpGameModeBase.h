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
	 * The maximum lifetime of a projectile.
	 */
	UPROPERTY(EditAnywhere, Category = Projectiles)
	float ProjectileLifetime = 10;

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
	SteadyTick(float DeltaTime) override
	{
		SCOPE_CYCLE_COUNTER(STAT_ShootEmUpMain);
		const auto Mechanism = GetMechanism();
		const auto EnemySpawner = AEnemySpawner::GetInstance();
		const auto PlayerSubject = GetSingletonSubject<FPlayerTrait>(Mechanism);
		const auto HitEffectCurve = EnemyHitEmissionByTimeout.GetRichCurveConst();

		int32 EnemiesCount = 0;
		if (AEnemySpawner::GetInstance())
		{
			EnemiesCount = AEnemySpawner::GetInstance()->GetEnemiesNum();
		}

		// General after-spawn initialization.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpAfterSpawn);
			static TArray<float> ResetData({0, 0, 1, 1});
			Mechanism->Operate<FUnsafeChain>(
			[=](FUnsafeSubjectHandle Subject, const FJustSpawned&, const FRendering& Rendering, const FBubbleSphere BubbleSphere)
			{
				ResetData[MaterialUvScaleDataIndex] = BubbleSphere.Radius / 200;
				Rendering.Owner->SetCustomData(Rendering.InstanceId, ResetData);
				Subject.RemoveTrait<FJustSpawned>();
				Subject.SetTrait(FAppearing{});
				Subject.SetTrait(FMove{});
			});
		}

		// Appearance animation.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpAppearance);
			const auto Curve = AppearanceDissolutionByTime.GetRichCurveConst();
			const auto EndTime = Curve->GetLastKey().Time;
			Mechanism->Operate<FUnsafeChain>(
			[=](FUnsafeSubjectHandle Subject, FAppearing& Appearing, const FRendering& Rendering)
			{
				const auto Value = Curve->Eval(Appearing.Time);
				Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialDissolveDataIndex, Value);
				Appearing.Time += DeltaTime;
				if (Appearing.Time >= EndTime)
				{
					Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialDissolveDataIndex, 0);
					Subject.RemoveTrait<FAppearing>();
				}
			});
		}

		// Shooting intervals.
		{
			Mechanism->Operate<FUnsafeChain>(FFilter::Make<FShoots>(),
			[&](FShoots& Shoots)
			{
				if (Shoots.Timeout > 0)
				{
					Shoots.Timeout -= DeltaTime;
				}
			});
		}

		// Shooting.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpShooting);
			const auto AdditionalStreamsCount = AdditionalShootingStreamByEnemiesCount.Evaluate(EnemiesCount);
			static const auto Filter = FFilter::Make<FShoot, FShoots, FLocated>().Exclude<FDying>();
			Mechanism->Operate<FUnsafeChain>(Filter,
			[=](FShoots& Shoots, const FShoot& Shoot, const FLocated& Located)
			{
				if (Shoots.Timeout <= 0)
				{
					const auto Projectile = Mechanism->SpawnSubject(Shoot.ProjectileRecord);
					Projectile.SetTrait(FDirected{Shoot.Direction});
					Projectile.SetTrait(FLocated{Located});
					Projectile.SetTrait(FMove{Shoot.Direction});
					auto DirectionA = Shoot.Direction;
					auto DirectionB = Shoot.Direction;
					const auto RotatorA = FQuat(FVector::UpVector, FMath::DegreesToRadians(+AdditionalShootingStreamAngle));
					const auto RotatorB = FQuat(FVector::UpVector, FMath::DegreesToRadians(-AdditionalShootingStreamAngle));
					for (int i = 0; i < AdditionalStreamsCount; ++i)
					{
						DirectionA = RotatorA.RotateVector(DirectionA);
						DirectionB = RotatorB.RotateVector(DirectionB);
						const auto ProjectileA = Mechanism->SpawnSubject(Shoot.ProjectileRecord);
						ProjectileA.SetTrait(FDirected{DirectionA});
						ProjectileA.SetTrait(FLocated{Located});
						ProjectileA.SetTrait(FMove{DirectionA});
						const auto ProjectileB = Mechanism->SpawnSubject(Shoot.ProjectileRecord);
						ProjectileB.SetTrait(FDirected{DirectionB});
						ProjectileB.SetTrait(FLocated{Located});
						ProjectileB.SetTrait(FMove{DirectionB});
					}
					Shoots.Timeout = Shoots.Interval;
				}
			});
		}

		// Non-enemy movement.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpMovement);
			static const auto Filter = FFilter::Make<FMove, FLocated>().Exclude<FProjectile, FDying, FEnemy>();
			Mechanism->OperateConcurrently(Filter,
			[=](const FMove& Move, FLocated& Located, const FSpeed* Speed)
			{
				if (Speed != nullptr)
				{
					Located.Location += Move.Velocity * Speed->Value * DeltaTime;
				}
				else
				{
					Located.Location += Move.Velocity * DeltaTime;
				}
			}, ThreadsCount, 32);
		}

		// Projectile movement and collision detection, enemy death start.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpProjectilesLogic);
			std::atomic<int32> SafeScore{Score};
			Mechanism->OperateConcurrently(
			[&](FSolidSubjectHandle ProjectileHandle,
				FLocated&           Located,
				FProjectile&        Projectile,
				const FMove&        Move,
				const FSpeed&       Speed,
				const FDamage&      Damage)
			{
				if (Projectile.Time > ProjectileLifetime)
				{
					ProjectileHandle.DespawnDeferred();
					return;
				}
				Projectile.Time += DeltaTime;
				// We use sub-stepping for projectiles cause they move
				// really fast.
				for (int i = 0; i < ProjectileSubSteps; ++i)
				{
					Located.Location += Move.Velocity * Speed.Value * DeltaTime / ProjectileSubSteps;
					static const auto EnemyFilter = FFilter::Make<FEnemy, FHealth>().Exclude(DyingFlag);
					const auto Overlappers = ABubbleCage::GetOverlapping(Located.Location, Projectile.Radius, EnemyFilter);
					for (const auto& Overlapper : Overlappers)
					{
						const auto& BubbleSphere = Overlapper.GetTraitRef<FBubbleSphere, EParadigm::Unsafe>();
						const auto Health = Overlapper.GetTraitPtr<FHealth, EParadigm::Unsafe>();
						ProjectileHandle->DespawnDeferred();
						Health->Value -= Damage.Value;
						if (Health->Value <= 0)
						{
							Overlapper.SetFlag(DyingFlag);
							Overlapper.RemoveTraitDeferred<FHealth>();
							Overlapper.RemoveTraitDeferred<FBubbleSphere>();
							Overlapper.SetTraitDeferred(FDying{});
							auto& Enemy = Overlapper.GetTraitRef<FEnemy, EParadigm::Unsafe>();
							SafeScore.fetch_add(Enemy.Score, std::memory_order_relaxed);
							Enemy.Score = 0;
						}
						FHit Hit;
						Hit.Time = 0;
						Hit.Velocity = Move.Velocity * (Speed.Value / BubbleSphere.Radius);
						Overlapper.SetTraitDeferred(Hit);
						i = ProjectileSubSteps; // Projectile no longer active.
						// One projectile can hit multiple targets at the same time,
						// so we don't break the overlappers loop here.
					}
				}
			}, ThreadsCount, 32);
			Score = SafeScore.load(std::memory_order_relaxed);
		}
		
		// The process of being hit.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpBeHit);
			const auto EndTime = HitEffectCurve->GetLastKey().Time;
			Mechanism->Operate<FUnsafeChain>(
			[=](FSubjectHandle    CharacterHandle,
				const FRendering& Rendering,
				FLocated&         Located,
				FHit&             Hit,
				FSpeed&           Speed)
			{
				const auto Value = HitEffectCurve->Eval(Hit.Time);
				Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialHitDataIndex, Value);
				// Attack visual should be cancelled:
				Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialAttackDataIndex, 0.0f);
				Located.Location += Hit.Velocity * Value;

				Hit.Time += DeltaTime;
				if (Hit.Time > EndTime)
				{
					CharacterHandle.RemoveTrait<FHit>();
				}
				// Cancel the attack...
				CharacterHandle.RemoveTrait<FAttacking>();
			});
		}

		// The process of a character dying.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpDying);
			const auto Curve = DissolutionByTime.GetRichCurveConst();
			Mechanism->Operate<FUnsafeChain>(
			[&](FSubjectHandle    CharacterHandle,
				const FRendering& Rendering,
				FDying&           Dying)
			{
				const auto Value = Curve->Eval(Dying.Time);
				const auto Duration = Curve->GetLastKey().Time;
				Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialDissolveDataIndex, Value);
				if (Dying.Time < Duration)
				{
					Dying.Time += DeltaTime;
				}
				else
				{
					// Perform the actual destruction now...
					CharacterHandle.Despawn();
				}
			});
		}

		// Decouple
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpDecouple);
			ABubbleCage::Evaluate();
		}

		// Player location sync.
		{
			Mechanism->Operate([=](ISolidSubjective* Subjective, const FPlayerTrait& Player, const FLocated& Located)
			{
				const auto Actor = CastChecked<APlayerPawn>(Subjective->GetActor());
				Actor->SetActorLocation(Located.Location);
			});
		}

		// Enemies reaching the player and enemy statistics.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpEnemiesReaching);
			TArray<std::atomic<int32>> SafeEnemyCountsByKind;
			SafeEnemyCountsByKind.AddDefaulted(EnemySpawner->EnemyCountsByKind.Num());
			for (int32 i = 0; i < EnemySpawner->EnemyCountsByKind.Num(); ++i)
			{
				SafeEnemyCountsByKind[i].store(0, std::memory_order_relaxed);
			}
			if (PlayerSubject && PlayerSubject.HasTrait<FBubbleSphere>())
			{
				const auto RoamingToTargetCurve = RoamingToTargetInfluenceByEnemiesCount.GetRichCurveConst();
				const auto AggressionRadius = AggressionRadiusByEnemiesCount.GetRichCurveConst()->Eval(EnemiesCount);
				const auto AggressionRadiusSquared = AggressionRadius * AggressionRadius;
				static constexpr float ReachTolerance = 1;
				const auto Target = PlayerSubject.GetTraitRef<EParadigm::Unsafe, FLocated>().GetLocation();
				const auto TargetRadius = PlayerSubject.GetTraitRef<EParadigm::Unsafe, FBubbleSphere>().Radius;
				static const auto Filter = FFilter::Make<FEnemy, FRendering>().Exclude<FHit, FDying>();
				Mechanism->OperateConcurrently(Filter,
				[&](FSolidSubjectHandle  EnemySubject,
					const FRendering&    Rendering,
					const FSpeed&        Speed,
					FMove&               Move,
					FEnemy&              Enemy,
					FLocated&            Located,
					const FBubbleSphere& BubbleSphere,
					FAttacks*            Attacks)
				{
					SafeEnemyCountsByKind[Enemy.KindId].fetch_add(1, std::memory_order_relaxed);
					auto Delta = Target - Located.GetLocation();
					auto Distance = Delta.SizeSquared();
					if (Distance < AggressionRadiusSquared)
					{
						Distance = FMath::Sqrt(Distance);
						if ((Attacks != nullptr) && (Distance - (BubbleSphere.Radius + TargetRadius) < Attacks->Range))
						{
							// We're able to attack now (in theory).
							if ((!EnemySubject.HasTrait<FAttacking>()) )
							{
								EnemySubject.SetTraitDeferred(FAttacking{});
							}
						}
						else
						{
							// Try to reach the player...
							if (Distance > SMALL_NUMBER)
							{
								Move.Velocity = Delta / Distance;
							}
						}
					}
					else
					{
						// Roam freely, but with tendency towards the target.
						if (Enemy.RoamingTimeout < DeltaTime)
						{
							Distance = FMath::Sqrt(Distance);
							Enemy.RoamingTimeout = FMath::RandRange(2.0f, 15.0f);
							Move.Velocity = FVector(AEnemySpawner::VRand2D(), 0.0f) + Delta * (RoamingToTargetCurve->Eval(EnemiesCount) / Distance);
							if (!Move.Velocity.Normalize())
							{
								Move.Velocity = FVector::ForwardVector;
							}
						}
						else
						{
							Enemy.RoamingTimeout -= DeltaTime;
						}
					}

					// Perform enemy movement right here for performance reasons:
					Located.Location += Move.Velocity * Speed.Value * DeltaTime;
				}, ThreadsCount, 32);
			}
			else
			{
				// No player available, so just roam...
				static const auto Filter = FFilter::Make<FEnemy, FRendering>().Exclude<FHit, FDying>();
				Mechanism->OperateConcurrently(Filter,
				[&](FSolidSubjectHandle  EnemySubject,
					const FRendering&    Rendering,
					const FSpeed&        Speed,
					FMove&               Move,
					FEnemy&              Enemy,
					FLocated&            Located,
					const FBubbleSphere& BubbleSphere,
					FAttacks*            Attacks)
				{
					SafeEnemyCountsByKind[Enemy.KindId].fetch_add(1, std::memory_order_relaxed);
					// Roam freely.
					if (Enemy.RoamingTimeout < DeltaTime)
					{
						Enemy.RoamingTimeout = FMath::RandRange(2.0f, 10.0f);
						Move.Velocity = FVector(AEnemySpawner::VRand2D(), 0.0f);
					}
					else
					{
						Enemy.RoamingTimeout -= DeltaTime;
					}
					// Perform enemy movement right here for performance reasons:
					Located.Location += Move.Velocity * Speed.Value * DeltaTime;
				}, ThreadsCount, 32);
			}
			for (int32 i = 0; i < EnemySpawner->EnemyCountsByKind.Num(); ++i)
			{
				EnemySpawner->EnemyCountsByKind[i] = SafeEnemyCountsByKind[i].load(std::memory_order_relaxed);
			}
		}

		// Player health regeneration.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpHealthRegeneration);
			if (PlayerSubject && PlayerSubject.HasTrait<FHealth>())
			{
				PlayerHealthRegenTimeout -= DeltaTime;
				if (PlayerHealthRegenTimeout <= 0)
				{
					auto& Health = PlayerSubject.GetTraitRef<EParadigm::Unsafe, FHealth>();
					Health.Value += PlayerHealthRegenPerSecond;
					if (Health.Value > Health.Maximum)
					{
						Health.Value = Health.Maximum;
					}
					PlayerHealthRegenTimeout = 1.0f;
				}
			}
		}

		// Enemies attacking the player.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpMeleeAttack);
			if (PlayerSubject && PlayerSubject.HasTrait<FBubbleSphere>())
			{
				const auto Target = PlayerSubject.GetTrait<FLocated>().GetLocation();
				const auto TargetRadius = PlayerSubject.GetTrait<FBubbleSphere>().Radius;
				static const auto Filter = FFilter::Make<FEnemy, FAttacks, FAttacking, FRendering>().Exclude<FHit, FDying>();
				Mechanism->Operate<FUnsafeChain>(
				[&](FUnsafeSubjectHandle Subject,
					const FLocated&      Located,
					const FBubbleSphere& BubbleSphere,
					const FRendering&    Rendering,
					const FAttacks&      Attacks,
					FAttacking&          Attacking)
				{
					const auto AnimationCurve = Attacks.Animation.GetRichCurveConst();
					const auto Value = AnimationCurve->Eval(Attacking.Time) * Attacks.Range;
					Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialAttackDataIndex, Value);
					if (Attacking.Time >= Attacks.FrameTime)
					{
						if (!Attacking.bExecuted)
						{
							auto Distance = FVector::Distance(Target, Located);

							if (Distance < BubbleSphere.Radius + Attacks.Range + TargetRadius)
							{
								// The player is in range so should receive the damage...
								const auto Health = PlayerSubject.GetTraitPtr<EParadigm::Unsafe, FHealth>();
								if (Health != nullptr)
								{
									Health->Value -= Attacks.Damage;
									if (Health->Value <= 0)
									{
										PlayerSubject.RemoveTrait<FHealth>();
										PlayerSubject.RemoveTrait<FBubbleSphere>();
										PlayerSubject.SetTrait(FDying{});
									}
									auto& Hit = PlayerSubject.ObtainTraitRef<EParadigm::Unsafe, FHit>();
									Hit.Time = 0;
								}
							}
							Attacking.bExecuted = true;
						}
						else if (Attacking.Time > AnimationCurve->GetLastKey().Time)
						{
							Subject.RemoveTrait<FAttacking>();
							return;
						}
					}
					Attacking.Time += DeltaTime;
				});
			}
		}

		// Gravity.
		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpGravity);
			static const auto Filter = FFilter::Make<FLocated, FBubbleSphere>();
			Mechanism->OperateConcurrently(Filter,
			[&](FSolidSubjectHandle Subject, FLocated& Located, const FBubbleSphere& BubbleSphere)
			{
				if (Located.Location.Z > BubbleSphere.Radius)
				{
					Located.Location.Z -= Gravity * DeltaTime;
				}
				if (Located.Location.Z < BubbleSphere.Radius)
				{
					Located.Location.Z = BubbleSphere.Radius;
				}
			}, ThreadsCount, 32);
		}

		{
			SCOPE_CYCLE_COUNTER(STAT_ShootEmUpUpdateRenderState);
			EnemySpawner->UpdateRenderState();
		}
	}

 public:

	AApparatusShootEmUpGameModeBase()
	{
		DefaultPawnClass = APlayerPawn::StaticClass();
	}
};
