#include "PlayerPawn.h"

#include "Components/InputComponent.h"
#include "Located.h"
#include "BubbleSphere.h"
#include "Traits/PlayerTrait.h"

#include "Traits/Move.h"


APlayerPawn::APlayerPawn()
{
	auto Root = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	Subjective = CreateDefaultSubobject<USubjectiveActorComponent>("Subjective");
	Subjective->SetTrait(FLocated{});
	Subjective->SetTrait(FPlayerTrait{});
	Subjective->ObtainTrait<FBubbleSphere>();
}

void
APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	FLocated Located(GetActorLocation());
	Subjective->SetTrait(Located);
}

void
APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp",    this, &APlayerPawn::MoveUp);
}

void
APlayerPawn::ApplyMovement()
{
	auto Move = Subjective->ObtainTrait<FMove>();
	Move.Velocity = GetActorForwardVector() * MovementDirection.Y + GetActorRightVector() * MovementDirection.X;
	Move.Velocity *= Speed;
	if (Move.Velocity.Size() <= 0.01f)
	{
		Subjective->RemoveTrait<FMove>();
	}
	else
	{
		Subjective->SetTrait(Move);
	}
}

void 
APlayerPawn::MoveRight(float Value)
{
	MovementDirection.X = Value;

	ApplyMovement();
}

void
APlayerPawn::MoveUp(float Value)
{
	MovementDirection.Y = Value;

	ApplyMovement();
}
