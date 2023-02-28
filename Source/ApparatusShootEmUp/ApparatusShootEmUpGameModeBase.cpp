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
