#include "EnemySpawner.h"


AEnemySpawner* AEnemySpawner::Instance = nullptr;

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void
AEnemySpawner::Tick(float DeltaTime)
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

void
AEnemySpawner::UpdateRenderState(std::function<void(void)> UpdatedCallback)
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
