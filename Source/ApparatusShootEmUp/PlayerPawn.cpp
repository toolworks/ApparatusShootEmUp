#include "PlayerPawn.h"

#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Located.h"
#include "BubbleSphere.h"
#include "Traits/PlayerTrait.h"

#include "Traits/Move.h"
#include "Traits/Shoot.h"
#include "EnemySpawner.h"


APlayerPawn::APlayerPawn()
{
	auto Root = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	Subjective = CreateDefaultSubobject<USubjectiveActorComponent>("Subjective");
	Subjective->SetTrait(FLocated{});
	Subjective->SetTrait(FPlayerTrait{});
	Subjective->ObtainTrait<FBubbleSphere>();
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	SetRootComponent(Root);
	PrimaryActorTick.bCanEverTick = true;
}

void
APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	FLocated Located(GetActorLocation());
	Subjective->SetTrait(Located);
	OriginalCameraRelativeLocation = CameraComponent->GetRelativeLocation();
}

void
APlayerPawn::Tick(float DeltaSeconds)
{
	const auto Spawner = AEnemySpawner::GetInstance();
	if (Spawner == nullptr) return;
	const auto EnemiesCount = Spawner->GetEnemiesNum();
	const auto RelativeLocation = ZoomScaleByEnemiesCount.GetRichCurveConst()->Eval(EnemiesCount) * OriginalCameraRelativeLocation;
	CameraComponent->SetRelativeLocation(FMath::Lerp(CameraComponent->GetRelativeLocation(), RelativeLocation, DeltaSeconds));
}

void
APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis("MoveRight",  this, &APlayerPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp",     this, &APlayerPawn::MoveUp);
	PlayerInputComponent->BindAxis("ShootRight", this, &APlayerPawn::ShootRight);
	PlayerInputComponent->BindAxis("ShootUp",    this, &APlayerPawn::ShootUp);
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
APlayerPawn::ApplyShooting()
{
	auto Direction = GetActorForwardVector() * ShootingDirection.Y + GetActorRightVector() * ShootingDirection.X;
	if (Direction.Normalize())
	{
		auto Shoot = Subjective->ObtainTrait<FShoot>();
		Shoot.Direction = Direction;
		Shoot.ProjectileRecord = ProjectileRecord;
		Subjective->SetTrait(Shoot);
	}
	else
	{
		Subjective->RemoveTrait<FShoot>();
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

void 
APlayerPawn::ShootRight(float Value)
{
	ShootingDirection.X = Value;

	ApplyShooting();
}

void
APlayerPawn::ShootUp(float Value)
{
	ShootingDirection.Y = Value;

	ApplyShooting();
}
