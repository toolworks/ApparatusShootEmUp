#include "EnemySpawner.h"


AEnemySpawner* AEnemySpawner::Instance = nullptr;

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = true;
}
