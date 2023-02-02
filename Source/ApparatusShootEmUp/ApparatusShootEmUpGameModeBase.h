// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MechanicalGameModeBase.h"

#include "Located.h"
#include "BubbleCage.h"
#include "Traits/Move.h"
#include "Traits/PlayerTrait.h"
#include "Traits/Enemy.h"
#include "PlayerPawn.h"

#include "ApparatusShootEmUpGameModeBase.generated.h"

/**
 * The main mechanical game mode.
 */
UCLASS()
class APPARATUSSHOOTEMUP_API AApparatusShootEmUpGameModeBase
  : public AMechanicalGameModeBase
{
	GENERATED_BODY()

 protected:

	template < typename T, typename ...Ts >
	FSubjectHandle
	GetSingleton() const
	{
		const auto Filter = FFilter::Make<T, Ts...>();
		const auto Chain = GetMechanism()->Enchain(Filter);
	
		auto Cursor = Chain->Iterate(0, 1);
		if (!Cursor.Provide())
		{
			checkNoEntry();
			return FSubjectHandle::Invalid;
		}

		auto SubjectHandle = Cursor.GetSubject();
		verifyf(!Cursor.Provide(), TEXT("Two singleton objects detected!"));
		return SubjectHandle;
	}

	void
	SteadyTick(float DeltaTime) override
	{
		auto PlayerSubject = GetSingleton<FPlayerTrait>();
		// Movement.
		{
			GetMechanism()->Operate(FFilter::Make<FMove, FLocated>(), [=](const FMove& Move, FLocated& Located)
			{
				Located.Location += Move.Velocity * DeltaTime;
			});
		}

		// Decouple
		{
			ABubbleCage::Evaluate();
		}

		// Location sync.
		{
			GetMechanism()->Operate([=](ISolidSubjective* Subjective, const FPlayerTrait& Player, const FLocated& Located)
			{
				const auto Actor = CastChecked<APlayerPawn>(Subjective->GetActor());
				Actor->SetActorLocation(Located.Location);
			});
		}

		// Enemies reaching player.
		{
			const auto Target = PlayerSubject.GetTrait<FLocated>().GetLocation();
			GetMechanism()->Operate<FUnsafeChain>([&](FUnsafeSubjectHandle Subject, const FEnemy& Enemy, const FLocated& Located)
			{
				const auto Delta = Target - Located.GetLocation();
				const auto Direction = Delta.GetSafeNormal();
				if (!Delta.IsZero())
				{
					auto& Move = Subject.ObtainTraitRef<FMove>();
					Move.Velocity = Direction * 100;
				}
			});
		}
	}

 public:

	AApparatusShootEmUpGameModeBase()
	{
		DefaultPawnClass = APlayerPawn::StaticClass();
	}
};
