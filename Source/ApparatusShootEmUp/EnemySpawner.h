#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector2D.h"
#include "Curves/CurveFloat.h"

#include "Machine.h"
#include "Apparatist/Singleton.h"
#include "TraitRendererComponent.h"

#include "Located.h"
#include "Rotated.h"
#include "BubbleSphere.h"
#include "BubbleCage.h"
#include "BubbleCageComponent.h"

#include "Traits/PlayerTrait.h"
#include "Traits/Enemy.h"
#include "Traits/RenderBatches.h"
#include "Traits/JustSpawned.h"

#include "EnemySpawner.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UCharacterMovementComponent;

/**
 * The enemy configuration entry.
 */
USTRUCT(BlueprintType)
struct APPARATUSSHOOTEMUP_API FEnemyConfig
{
    GENERATED_BODY()
 
  public:

	/**
	 * The chance of spawning the enemy.
	 */
	UPROPERTY(EditAnywhere, Category = Enemy)
	float SpawnWeight = 1;

	/**
	 * The actual adjusted spawn weight,
	 * resolved at the start of the 
	 */
	float ActualSpawnWeight;

	/**
	 * The cost of spawning this enemy.
	 */
	UPROPERTY(EditAnywhere, Category = Enemy)
	int32 Cost = 1;

	/**
	 * The main enemy composition prefab.
	 */
	UPROPERTY(EditAnywhere, Category = Enemy)
	FSubjectRecord EnemyRecord;
};


UCLASS()
class APPARATUSSHOOTEMUP_API AEnemySpawner
  : public AActor
{
	GENERATED_BODY()

  private:

	// Needs access to enemy count to decrease it on death.
	friend class AApparatusShootEmUpGameModeBase;

	/**
	 * The current enemy budget to spend.
	 */
	float EnemyBudget = 0;

	/**
	 * The total number of enemies currently in the world.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Statistics, Meta = (AllowPrivateAccess))
	int32 EnemiesCount = 0;

	/**
	 * The singleton instance of the spawner.
	 * 
	 * Set during BeginPlay.
	 */
	static AEnemySpawner* Instance;

	/**
	 * The index of the current batch being rendered.
	 */
	int32 CurrentBatchIndex = 0;

  public:

	/**
	 * The maximum size of a rendering batch before starting to defer it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Optimization)
	int32 BatchSizeMax = 10000;

	/**
	 * The enemy spawning radius in relation to the current count.
	 */
	UPROPERTY(EditAnywhere, Category = Spawning)
	FRuntimeFloatCurve SpawnRadiusByCount;

	/**
	 * All of the available enemy configurations.
	 */
	UPROPERTY(EditAnywhere, Category = Enemies)
	TArray<FEnemyConfig> EnemyConfigs;

	/**
	 * The amount of budget raise per second by the current number of enemies.
	 */
	UPROPERTY(EditAnywhere, Category = Spawning)
	FRuntimeFloatCurve BudgetGain;

	/**
	 * The total spawning weight consisting of every enemy weight.
	 */
	UPROPERTY(VisibleAnywhere, Category = Spawning)
	float TotalWeight = 0.0f;

	/**
	 * The original total spawning weight consisting of every enemy weight.
	 */
	UPROPERTY(VisibleAnywhere, Category = Spawning)
	float OriginalTotalWeight = 0.0f;

	/**
	 * All of the enemy counts grouped by the kind.
	 */
	TArray<int32> EnemyCountsByKind;

	uint32 SpawnId = 0;

	TArray<UScriptStruct*> BatchTypes;

  protected:

	void
	BeginPlay() override
	{
		Super::BeginPlay();

		EnemyCountsByKind.Reset();
		OriginalTotalWeight = 0;
		for (auto& EnemyConfig : EnemyConfigs)
		{
			OriginalTotalWeight += EnemyConfig.SpawnWeight;
			EnemyCountsByKind.Add(0);
		}

		BatchTypes.Add(FRenderBatchA::StaticStruct());
		BatchTypes.Add(FRenderBatchB::StaticStruct());
		BatchTypes.Add(FRenderBatchC::StaticStruct());
		BatchTypes.Add(FRenderBatchD::StaticStruct());

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

	void
	Tick(float DeltaTime) override
	{
		Super::Tick(DeltaTime);

		const auto Mechanism = UMachine::ObtainMechanism(GetWorld());
		const auto Curve = BudgetGain.GetRichCurveConst();

		EnemiesCount = 0;
		for (auto Count : EnemyCountsByKind)
		{
			EnemiesCount += Count;
		}

		TotalWeight = 0;
		for (int32 i = 0; i < EnemyConfigs.Num(); ++i)
		{
			auto& EnemyConfig = EnemyConfigs[i];
			const auto NeededWeight = EnemyConfig.SpawnWeight / OriginalTotalWeight;
			const auto FactualWeight = (float) EnemyCountsByKind[i] / (float) EnemiesCount;
			EnemyConfig.ActualSpawnWeight = FMath::Max(NeededWeight - FactualWeight, 0.0001f);
			TotalWeight += EnemyConfig.ActualSpawnWeight;
		}

		EnemyBudget += Curve->Eval(EnemiesCount) * DeltaTime;

		const auto PlayerSubject = GetSingletonSubject<FPlayerTrait>(UMachine::ObtainMechanism(GetWorld()));
		auto PlayerLocation2D = FVector2D(0, 0);
		if (LIKELY(PlayerSubject))
		{
			PlayerLocation2D = FVector2D(PlayerSubject.GetTrait<FLocated>().Location);
		}
		const auto BubbleCage = ABubbleCage::GetInstance();
		if (BubbleCage == nullptr) return;
		const auto BubbleCageComponent = BubbleCage->GetComponent();

		// We don't want too many spawning during a frame, cause they may actually lag:
		int32 SpawnPointIterationsMax = 128;

		while (EnemyBudget > 0)
		{
			const auto SpawnRadius = SpawnRadiusByCount.GetRichCurveConst()->Eval(EnemiesCount);

			// Point to the enemy config to spawn:
			float WeightCursor = FMath::RandRange(0.0f, TotalWeight);
			for (int32 j = 0; j < EnemyConfigs.Num(); ++j)
			{
				const auto& Config = EnemyConfigs[j];
				if (WeightCursor <= Config.ActualSpawnWeight)
				{
					const auto& Sphere = Config.EnemyRecord.GetTraitRef<FBubbleSphere>();

					// Make sure the spawn point is within a valid range:
					FVector SpawnPoint2D;
					do
					{
						SpawnPoint2D = FVector(PlayerLocation2D + VRand2D() * (SpawnRadius + Sphere.Radius), Sphere.Radius);
						if (--SpawnPointIterationsMax == 0)
						{
							// Too many tries to find the point, so just exit for now:
							return;
						}
					} while (!BubbleCageComponent->IsInside(SpawnPoint2D));

					const auto Enemy = Mechanism->SpawnSubject(Config.EnemyRecord);
					Enemy.GetTraitRef<EParadigm::Unsafe, FEnemy>().KindId = j;
					Enemy.SetTrait(FLocated(SpawnPoint2D));
					Enemy.SetTrait(FJustSpawned());
					// Random rotation for variation sake:
					Enemy.SetTrait(FRotated(FRotator(FMath::FRandRange(-180.f, 180.f),
													 FMath::FRandRange(-180.f, 180.f),
													 FMath::FRandRange(-180.f, 180.f)).Quaternion()));
					Enemy.ObtainTrait(BatchTypes[(SpawnId++) % BatchTypes.Num()]);
					EnemyBudget -= Config.Cost;
					break; // The enemy was spawned successfully.
				}
				WeightCursor -= Config.ActualSpawnWeight;
			}
		}
	}

  public:

	/**
	 * Get the current number of enemies.
	 */
	FORCEINLINE auto
	GetEnemiesNum() const
	{
		return EnemiesCount;
	}

	/**
	 * Get the current instance of the spawner.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static AEnemySpawner*
	GetInstance()
	{
		return Instance;
	}

	static FVector2D
	VRand2D()
	{
		FVector2D Result;
		float Length;

		do
		{
			Result.X = FMath::FRand() * 2.0f - 1.0f;
			Result.Y = FMath::FRand() * 2.0f - 1.0f;
			Length = Result.SizeSquared();
		}
		while ((Length > 1.0f) || (Length < KINDA_SMALL_NUMBER));

		return Result * (1.0f / FMath::Sqrt(Length));
	}

	void UpdateRenderState(std::function<void(void)> UpdatedCallback)
	{
		for (int32 BatchedSoFar = 0, TotalHandled = 0; BatchedSoFar < BatchSizeMax && TotalHandled < BatchTypes.Num(); CurrentBatchIndex += 1, TotalHandled += 1)
		{
			if (CurrentBatchIndex >= BatchTypes.Num())
			{
				CurrentBatchIndex = 0;
			}
			const auto BatchType = BatchTypes[CurrentBatchIndex];
			const auto Renderer = UTraitRendererComponent::GetInstance(BatchType);
			Renderer->Update();
			UpdatedCallback();
			Renderer->UpdateRenderState();
			BatchedSoFar += Renderer->GetNumRenderInstances();
		}
	}

	AEnemySpawner();
};