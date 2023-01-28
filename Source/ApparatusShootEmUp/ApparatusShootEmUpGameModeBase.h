// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MechanicalGameModeBase.h"

#include "Located.h"
#include "Traits/Move.h"
#include "Traits/PlayerTrait.h"
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

	void
	SteadyTick(float DeltaTime) override
	{
		// Movement.
		{
			GetMechanism()->Operate(FFilter::Make<FMove, FLocated>(), [=](const FMove& Move, FLocated& Located)
			{
				Located.Location += Move.Velocity * DeltaTime;
			});
		}

		// Location sync.
		{
			GetMechanism()->Operate([=](ISolidSubjective* Subjective, const FPlayerTrait& Player, const FLocated& Located)
			{
				Subjective->GetActor()->SetActorLocation(Located.Location);
			});
		}
	}

 public:

	AApparatusShootEmUpGameModeBase()
	{
		DefaultPawnClass = APlayerPawn::StaticClass();
	}
};
