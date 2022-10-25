#pragma once
#include "../global/vars.h"

void inject_sgame(bool replace);

long GetRandomControl();
void SeedRandomControl(long seed);
long GetRandomDraw();
void SeedRandomDraw(long seed);
long GameStats(long level_num, long type);
void SortOutAdventureSave(long world);
long Level2World(long level);
long World2Level(long world);
long LevelStats(long level);
void GetValidLevelsList(REQUEST_INFO* req);
void GetSavedGamesList(REQUEST_INFO* req);
void DisplayCredits();
long LevelCompleteSequence();
long S_FrontEndCheck(SAVEGAME_INFO* pData, long nBytes);
long S_LoadGame(LPVOID data, long size, long slot);

#define mGetAngle	( (ulong(__cdecl*)(long, long, long, long)) 0x00483860)
#define S_SaveGame	( (void(__cdecl*)(void*, long, long)) 0x00484580 )
