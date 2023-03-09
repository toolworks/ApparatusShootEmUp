#include "ApparatusShootEmUpGameModeBase.h"


DEFINE_STAT(STAT_ShootEmUpMain);
DEFINE_STAT(STAT_ShootEmUpAfterSpawn);
DEFINE_STAT(STAT_ShootEmUpAppearance);
DEFINE_STAT(STAT_ShootEmUpShooting);
DEFINE_STAT(STAT_ShootEmUpMovement);
DEFINE_STAT(STAT_ShootEmUpProjectilesLogic);
DEFINE_STAT(STAT_ShootEmUpBeHit);
DEFINE_STAT(STAT_ShootEmUpDying);
DEFINE_STAT(STAT_ShootEmUpDecouple);
DEFINE_STAT(STAT_ShootEmUpEnemiesReaching);
DEFINE_STAT(STAT_ShootEmUpHealthRegeneration);
DEFINE_STAT(STAT_ShootEmUpMeleeAttack);
DEFINE_STAT(STAT_ShootEmUpGravity);
DEFINE_STAT(STAT_ShootEmUpUpdateRenderState);

AApparatusShootEmUpGameModeBase* AApparatusShootEmUpGameModeBase::Instance = nullptr;

void
AApparatusShootEmUpGameModeBase::Tick(float DeltaTime)
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

	// Appearance animation.
	{
		SCOPE_CYCLE_COUNTER(STAT_ShootEmUpAppearance);
		const auto Curve = AppearanceDissolutionByTime.GetRichCurveConst();
		const auto EndTime = Curve->GetLastKey().Time;
		Mechanism->OperateConcurrently(
		[=](FSolidSubjectHandle Subject, FAppearing& Appearing, const FRendering& Rendering)
		{
			const auto Value = Curve->Eval(Appearing.Time);
			Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialDissolveDataIndex, Value);
			Appearing.Time += DeltaTime;
			if (Appearing.Time >= EndTime)
			{
				Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialDissolveDataIndex, 0);
				Subject.RemoveTraitDeferred<FAppearing>();
			}
		}, ThreadsCount, BaseBatchSize);
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
		const float RadiusRatio = 0.5f;
		static const auto Filter = FFilter::Make<FShoot, FShoots, FLocated, FBubbleSphere>().Exclude<FDying>();
		Mechanism->Operate<FUnsafeChain>(Filter,
		[=](FUnsafeSubjectHandle PlayerHandle, FShoots& Shoots, const FShoot& Shoot, const FLocated& Located, const FBubbleSphere& BubbleSphere)
		{
			if (Shoot.Direction.IsZero()) return;
			PlayerHandle.SetTrait(FDirected{Shoot.Direction});
			if (Shoots.Timeout <= 0)
			{
				const auto Projectile = Mechanism->SpawnSubject(Shoot.ProjectileRecord);
				Projectile.SetTrait(FDirected{Shoot.Direction});
				Projectile.SetTrait(FLocated{Located.Location + Shoot.Direction * BubbleSphere.Radius * RadiusRatio});
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
					ProjectileA.SetTrait(FLocated{Located.Location + DirectionA * BubbleSphere.Radius * RadiusRatio});
					ProjectileA.SetTrait(FMove{DirectionA});
					const auto ProjectileB = Mechanism->SpawnSubject(Shoot.ProjectileRecord);
					ProjectileB.SetTrait(FDirected{DirectionB});
					ProjectileB.SetTrait(FLocated{Located.Location + DirectionB * BubbleSphere.Radius * RadiusRatio});
					ProjectileB.SetTrait(FMove{DirectionB});
				}
				Shoots.Timeout = Shoots.Interval;
			}
		});
	}

	// Non-enemy movement.
	{
		SCOPE_CYCLE_COUNTER(STAT_ShootEmUpMovement);
		const auto Applicator = Mechanism->CreateDeferredsApplicator();
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
		}, ThreadsCount, BaseBatchSize);
	}

	// Projectile movement and collision detection, enemy death start.
	{
		SCOPE_CYCLE_COUNTER(STAT_ShootEmUpProjectilesLogic);
		const auto Applicator = Mechanism->CreateDeferredsApplicator();
		const auto Curve = ProjectileOpacityByTime.GetRichCurveConst();
		const auto Lifetime = Curve->GetLastKey().Time;
		std::atomic<int32> SafeScore{Score};
		Mechanism->OperateConcurrently(
		[&](FSolidSubjectHandle ProjectileHandle,
			FLocated&           Located,
			FProjectile&        Projectile,
			FRendering&         Rendering,
			const FMove&        Move,
			const FSpeed&       Speed,
			const FDamage&      Damage)
		{
			if (Projectile.Time > Lifetime)
			{
				ProjectileHandle.DespawnDeferred();
				return;
			}
			const auto Opacity = Curve->Eval(Projectile.Time);
			Projectile.Time += DeltaTime;
			Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, ProjectileMaterialOpacityDataIndex, Opacity);
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
					Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, ProjectileMaterialOpacityDataIndex, 0);
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
		}, ThreadsCount, BaseBatchSize);
		Score = SafeScore.load(std::memory_order_relaxed);
	}
	
	// The process of being hit.
	{
		SCOPE_CYCLE_COUNTER(STAT_ShootEmUpBeHit);
		const auto EndTime = HitEffectCurve->GetLastKey().Time;
		const auto Applicator = Mechanism->CreateDeferredsApplicator();
		Mechanism->OperateConcurrently(
		[=](FSolidSubjectHandle CharacterHandle,
			const FRendering&   Rendering,
			FLocated&           Located,
			FHit&               Hit,
			FSpeed&             Speed)
		{
			const auto Value = HitEffectCurve->Eval(Hit.Time);
			Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialHitDataIndex, Value);
			// Attack visual should be cancelled:
			Rendering.Owner->SetCustomDataValue(Rendering.InstanceId, MaterialAttackDataIndex, 0.0f);
			Located.Location += Hit.Velocity * Value;

			Hit.Time += DeltaTime;
			if (Hit.Time > EndTime)
			{
				CharacterHandle.RemoveTraitDeferred<FHit>();
			}
			// Cancel the attack...
			CharacterHandle.RemoveTraitDeferred<FAttacking>();
		}, ThreadsCount, BaseBatchSize);
	}

	// The process of a character dying.
	{
		SCOPE_CYCLE_COUNTER(STAT_ShootEmUpDying);
		const auto Curve = DissolutionByTime.GetRichCurveConst();
		const auto Applicator = Mechanism->CreateDeferredsApplicator();
		Mechanism->OperateConcurrently(
		[&](FSolidSubjectHandle CharacterHandle,
			const FRendering&   Rendering,
			FDying&             Dying)
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
				CharacterHandle.DespawnDeferred();
			}
		}, ThreadsCount, BaseBatchSize);
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
		const auto Applicator = Mechanism->CreateDeferredsApplicator();
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
				FDirected&           Directed,
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
						Enemy.RoamingTimeout = FMath::RandRange(2.0f, 15.0f) * (BubbleSphere.Radius / 500);
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
				Directed.Direction = FMath::Lerp(Directed.Direction, Move.Velocity, DeltaTime * 4);
				// Perform enemy movement right here for performance reasons:
				Located.Location += Move.Velocity * Speed.Value * DeltaTime;
			}, ThreadsCount, BaseBatchSize);
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
				FDirected&           Directed,
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
				Directed.Direction = FMath::Lerp(Directed.Direction, Move.Velocity, DeltaTime * 4);
				// Perform enemy movement right here for performance reasons:
				Located.Location += Move.Velocity * Speed.Value * DeltaTime;
			}, ThreadsCount, BaseBatchSize);
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
		const auto Applicator = Mechanism->CreateDeferredsApplicator();
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
		}, ThreadsCount, BaseBatchSize);
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_ShootEmUpUpdateRenderState);
		EnemySpawner->UpdateRenderState([&]()
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
		});
	}
}