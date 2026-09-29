#include "iGraphics.h"
#include "pujita_level3.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>
#include <cstring>
#include <time.h>
#include <stdlib.h>

#pragma comment(lib, "winmm.lib")

void playLevel2CatchSound();
void playLevel3SpellSound();
void playWinSound();     // NEW
void playCoinSound();    // NEW
void drawVictoryBox(const char* title, const char* line1, const char* line2,
	const char* hint1, const char* hint2);   // NEW
void drawGameOverBox(const char* title, const char* line1, const char* line2,
	const char* hint1, const char* hint2);   // NEW
void initCoinSound();    // NEW
void playGameOverSound();   // NEW
#pragma comment(lib, "winmm.lib")

#define SCREEN_WIDTH 1365
#define SCREEN_HEIGHT 768

// ======================================================
// SCREEN
// ======================================================

// 0 = Main Menu
// 1 = How To Play
// 2 = Credits
// 3 = Story (Level 1 backstory)
// 4 = Level 1
// 5 = Level 2
// 6 = Level 3 Final Trial
// 7 = Level 2 Backstory
// 8 = Level Select
// 9 = Level 3 Backstory
// 10 = Name Input (before Level 1)
// 11 = Highscore Board
int screen = 0;

// =====================================================
// LEVEL 2 BACKSTORY
// =====================================================

int level2StoryPage = 1;

// ======================================================
// FUNCTION DECLARATIONS
// ======================================================

void initLevel2();
void drawLevel2();
void updateLevel2();
void level2Keyboard(unsigned char key);
void initLevel3();
void drawLevel3();
void updateLevel3();
void level3Keyboard(unsigned char key);
void level3SpecialKeyboard(int key);
void level3Mouse(int button, int state, int x, int y);
void level3MouseMove(int x, int y);
void level3MousePassiveMove(int x, int y);

// =====================================================
// LEVEL 3 IMPLEMENTATION
// =====================================================

// =====================================================
// ECHOES OF ARCANA - LEVEL 3 FINAL TRIAL
// =====================================================

#define L3_SCREEN_WIDTH 1365
#define L3_SCREEN_HEIGHT 768
#define L3_MAX_SPIDERS 4
#define L3_MAX_SMALL_ENEMIES 4
#define L3_MAX_PLAYER_SPELLS 30
#define L3_MAX_ENEMY_SPELLS 40

struct L3Spider
{
	int x;
	int y;
	int hp;
	bool active;
	int attackCooldown;
};

struct L3SmallEnemy
{
	int x;
	int y;
	int hp;
	bool active;
	int attackCooldown;
	int shootCooldown;
};

struct L3Spell
{
	int x;
	int y;
	int dx;
	int dy;
	int speed;
	bool active;
};

struct L3EnemySpell
{
	int x;
	int y;
	int dx;
	int dy;
	int speed;
	int damage;
	bool active;
};

// =====================================================
// PLAYER
// =====================================================

int level3PlayerX = 620;
int level3PlayerY = 100;
int level3PlayerWidth = 90;
int level3PlayerHeight = 110;
int level3PlayerSpeed = 8;
int level3PlayerDirection = 0;
// 0 = down, 1 = up, 2 = left, 3 = right

int level3PlayerHP = 100;
int level3PlayerMaxHP = 100;

int level3Mana = 100;
int level3MaxMana = 100;

// =====================================================
// GAME STATE
// =====================================================

int level3Phase = 1;
bool level3GameOver = false;
bool level3Won = false;

int level3PlayerDamageCooldown = 0;
int level3DashCooldown = 0;
int level3DashInvulnerability = 0;
int level3TransitionTimer = 0;
int level3FrameCounter = 0;

// Tracks the furthest phase reached in this attempt, so R can
// retry from the current phase instead of always going back to
// Phase 1.
int level3CheckpointPhase = 1;

// =====================================================
// ENEMIES
// =====================================================

L3Spider spiders[L3_MAX_SPIDERS];
L3SmallEnemy smallEnemies[L3_MAX_SMALL_ENEMIES];

int level3GuardianX = 1000;
int level3GuardianY = 500;
int level3GuardianHP = 100;
int level3GuardianMaxHP = 100;
bool level3GuardianActive = false;
int level3GuardianPunchCooldown = 0;
int level3GuardianMagicCooldown = 0;

L3Spell playerSpells[L3_MAX_PLAYER_SPELLS];
L3EnemySpell enemySpells[L3_MAX_ENEMY_SPELLS];

// =====================================================
// HELPER FUNCTIONS
// =====================================================

bool l3RectCollision(int x1, int y1, int w1, int h1,
	int x2, int y2, int w2, int h2)
{
	if (x1 + w1 < x2) return false;
	if (x2 + w2 < x1) return false;
	if (y1 + h1 < y2) return false;
	if (y2 + h2 < y1) return false;
	return true;
}

int l3Distance(int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	return (int)sqrt((double)(dx * dx + dy * dy));
}

void l3ClearSpells()
{
	int i;

	for (i = 0; i < L3_MAX_PLAYER_SPELLS; i++)
		playerSpells[i].active = false;

	for (i = 0; i < L3_MAX_ENEMY_SPELLS; i++)
		enemySpells[i].active = false;
}

void l3SpawnPlayerSpell()
{
	int i;

	if (level3Mana < 10)
		return;

	for (i = 0; i < L3_MAX_PLAYER_SPELLS; i++)
	{
		if (!playerSpells[i].active)
		{
			playerSpells[i].x = level3PlayerX + level3PlayerWidth / 2 - 12;
			playerSpells[i].y = level3PlayerY + level3PlayerHeight / 2 - 12;
			playerSpells[i].dx = 0;
			playerSpells[i].dy = 0;
			playerSpells[i].speed = 18;

			if (level3PlayerDirection == 0)
				playerSpells[i].dy = -1;
			else if (level3PlayerDirection == 1)
				playerSpells[i].dy = 1;
			else if (level3PlayerDirection == 2)
				playerSpells[i].dx = -1;
			else
				playerSpells[i].dx = 1;

			playerSpells[i].active = true;
			level3Mana -= 10;

			playLevel3SpellSound();

			return;
		}
	}
}

void l3SpawnEnemySpell(int x, int y, int damage, int speed, bool spread)
{
	int i;
	int dx = level3PlayerX - x;
	int dy = level3PlayerY - y;
	double length;

	if (dx == 0 && dy == 0)
		dx = -1;

	length = sqrt((double)(dx * dx + dy * dy));
	if (length == 0) length = 1;

	for (i = 0; i < L3_MAX_ENEMY_SPELLS; i++)
	{
		if (!enemySpells[i].active)
		{
			enemySpells[i].x = x;
			enemySpells[i].y = y;
			enemySpells[i].dx = (int)(dx / length * 10.0);
			enemySpells[i].dy = (int)(dy / length * 10.0);

			if (enemySpells[i].dx == 0 && dx != 0)
				enemySpells[i].dx = (dx > 0) ? 1 : -1;
			if (enemySpells[i].dy == 0 && dy != 0)
				enemySpells[i].dy = (dy > 0) ? 1 : -1;

			if (spread)
			{
				if (i % 3 == 1)
					enemySpells[i].dx += (enemySpells[i].dy >= 0 ? 3 : -3);
				else if (i % 3 == 2)
					enemySpells[i].dx += (enemySpells[i].dy >= 0 ? -3 : 3);
			}

			enemySpells[i].speed = speed;
			enemySpells[i].damage = damage;
			enemySpells[i].active = true;
			return;
		}
	}
}

void l3DamagePlayer(int damage)
{
	if (level3GameOver || level3Won)
		return;

	if (level3DashInvulnerability > 0)
		return;

	if (level3PlayerDamageCooldown > 0)
		return;

	level3PlayerHP -= damage;
	level3PlayerDamageCooldown = 25;

	if (level3PlayerHP <= 0)
	{
		level3PlayerHP = 0;
		level3GameOver = true;

		playGameOverSound();
	}
}

// =====================================================
// INITIALIZE LEVEL 3
// =====================================================

void initLevel3()
{
	int i;

	mciSendString("stop winSound", NULL, 0, NULL);

	level3PlayerX = 620;
	level3PlayerY = 100;
	level3PlayerDirection = 1;

	level3PlayerHP = 100;
	level3Mana = 100;

	level3Phase = 1;
	level3CheckpointPhase = 1;
	level3GameOver = false;
	level3Won = false;

	level3PlayerDamageCooldown = 0;
	level3DashCooldown = 0;
	level3DashInvulnerability = 0;
	level3TransitionTimer = 0;
	level3FrameCounter = 0;

	// Four spiders: 30 HP each
	spiders[0].x = 150;  spiders[0].y = 560;
	spiders[1].x = 450;  spiders[1].y = 600;
	spiders[2].x = 900;  spiders[2].y = 570;
	spiders[3].x = 1120; spiders[3].y = 250;

	for (i = 0; i < L3_MAX_SPIDERS; i++)
	{
		spiders[i].hp = 30;
		spiders[i].active = true;
		spiders[i].attackCooldown = 0;
	}

	// Four small enemies: 40 HP each
	smallEnemies[0].x = 180;  smallEnemies[0].y = 300;
	smallEnemies[1].x = 500;  smallEnemies[1].y = 520;
	smallEnemies[2].x = 850;  smallEnemies[2].y = 300;
	smallEnemies[3].x = 1150; smallEnemies[3].y = 560;

	for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
	{
		smallEnemies[i].hp = 40;
		smallEnemies[i].active = false;
		smallEnemies[i].attackCooldown = 0;
		smallEnemies[i].shootCooldown = 60;
	}

	level3GuardianX = 1050;
	level3GuardianY = 500;
	level3GuardianHP = 100;
	level3GuardianActive = false;
	level3GuardianPunchCooldown = 0;
	level3GuardianMagicCooldown = 0;

	l3ClearSpells();
}

// =====================================================
// PHASE START
// =====================================================

void l3StartPhase2()
{
	int i;

	level3Phase = 2;
	level3CheckpointPhase = 2;
	level3TransitionTimer = 70;

	for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
	{
		smallEnemies[i].active = true;
		smallEnemies[i].hp = 40;
		smallEnemies[i].attackCooldown = 0;
		smallEnemies[i].shootCooldown = 90 + i * 20;
	}
}

void l3StartPhase3()
{
	level3Phase = 3;
	level3CheckpointPhase = 3;
	level3TransitionTimer = 90;
	level3GuardianActive = false;
	level3GuardianHP = 100;
	level3GuardianPunchCooldown = 30;
	level3GuardianMagicCooldown = 50;
}

// =====================================================
// RETRY FROM CHECKPOINT (used by R during/after a run,
// instead of a full initLevel3() reset back to Phase 1)
// =====================================================

void l3RetryFromCheckpoint()
{
	int i;

	// Stop the win sound if it is still playing
	mciSendString("stop winSound", NULL, 0, NULL);

	level3PlayerX = 620;
	level3PlayerY = 100;
	level3PlayerDirection = 1;

	level3PlayerHP = 100;
	level3Mana = 100;

	level3GameOver = false;
	level3Won = false;

	level3PlayerDamageCooldown = 0;
	level3DashCooldown = 0;
	level3DashInvulnerability = 0;
	level3TransitionTimer = 0;
	level3FrameCounter = 0;

	l3ClearSpells();

	level3Phase = level3CheckpointPhase;

	if (level3Phase == 1)
	{
		spiders[0].x = 150;  spiders[0].y = 560;
		spiders[1].x = 450;  spiders[1].y = 600;
		spiders[2].x = 900;  spiders[2].y = 570;
		spiders[3].x = 1120; spiders[3].y = 250;

		for (i = 0; i < L3_MAX_SPIDERS; i++)
		{
			spiders[i].hp = 30;
			spiders[i].active = true;
			spiders[i].attackCooldown = 0;
		}

		for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
			smallEnemies[i].active = false;

		level3GuardianActive = false;
	}
	else if (level3Phase == 2)
	{
		for (i = 0; i < L3_MAX_SPIDERS; i++)
			spiders[i].active = false;

		smallEnemies[0].x = 180;  smallEnemies[0].y = 300;
		smallEnemies[1].x = 500;  smallEnemies[1].y = 520;
		smallEnemies[2].x = 850;  smallEnemies[2].y = 300;
		smallEnemies[3].x = 1150; smallEnemies[3].y = 560;

		for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
		{
			smallEnemies[i].hp = 40;
			smallEnemies[i].active = true;
			smallEnemies[i].attackCooldown = 0;
			smallEnemies[i].shootCooldown = 90 + i * 20;
		}

		level3GuardianActive = false;
	}
	else // phase 3
	{
		for (i = 0; i < L3_MAX_SPIDERS; i++)
			spiders[i].active = false;

		for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
			smallEnemies[i].active = false;

		level3GuardianX = 1050;
		level3GuardianY = 500;
		level3GuardianHP = 100;
		level3GuardianActive = true;
		level3GuardianPunchCooldown = 30;
		level3GuardianMagicCooldown = 50;
	}
}

// =====================================================
// PLAYER MOVEMENT + DASH
// =====================================================

void l3MovePlayer(int dx, int dy)
{
	if (level3GameOver || level3Won || level3TransitionTimer > 0)
		return;

	level3PlayerX += dx;
	level3PlayerY += dy;

	if (level3PlayerX < 20) level3PlayerX = 20;
	if (level3PlayerY < 30) level3PlayerY = 30;

	if (level3PlayerX > L3_SCREEN_WIDTH - level3PlayerWidth - 20)
		level3PlayerX = L3_SCREEN_WIDTH - level3PlayerWidth - 20;

	if (level3PlayerY > L3_SCREEN_HEIGHT - level3PlayerHeight - 30)
		level3PlayerY = L3_SCREEN_HEIGHT - level3PlayerHeight - 30;
}

void l3Dash()
{
	int dx = 0;
	int dy = 0;

	if (level3DashCooldown > 0 || level3GameOver || level3Won)
		return;

	if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000)
		dy += 1;
	if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
		dy -= 1;
	if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
		dx -= 1;
	if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
		dx += 1;

	if (dx == 0 && dy == 0)
	{
		if (level3PlayerDirection == 0) dy = -1;
		else if (level3PlayerDirection == 1) dy = 1;
		else if (level3PlayerDirection == 2) dx = -1;
		else dx = 1;
	}

	level3PlayerX += dx * 120;
	level3PlayerY += dy * 120;

	if (level3PlayerX < 20) level3PlayerX = 20;
	if (level3PlayerY < 30) level3PlayerY = 30;
	if (level3PlayerX > L3_SCREEN_WIDTH - level3PlayerWidth - 20)
		level3PlayerX = L3_SCREEN_WIDTH - level3PlayerWidth - 20;
	if (level3PlayerY > L3_SCREEN_HEIGHT - level3PlayerHeight - 30)
		level3PlayerY = L3_SCREEN_HEIGHT - level3PlayerHeight - 30;

	level3DashCooldown = 35;
	level3DashInvulnerability = 10;
}

// =====================================================
// UPDATE SPELLS
// =====================================================

void l3UpdatePlayerSpells()
{
	int i, j;

	for (i = 0; i < L3_MAX_PLAYER_SPELLS; i++)
	{
		if (!playerSpells[i].active)
			continue;

		playerSpells[i].x += playerSpells[i].dx * playerSpells[i].speed;
		playerSpells[i].y += playerSpells[i].dy * playerSpells[i].speed;

		if (playerSpells[i].x < -40 || playerSpells[i].x > L3_SCREEN_WIDTH + 40 ||
			playerSpells[i].y < -40 || playerSpells[i].y > L3_SCREEN_HEIGHT + 40)
		{
			playerSpells[i].active = false;
			continue;
		}

		if (level3Phase == 1)
		{
			for (j = 0; j < L3_MAX_SPIDERS; j++)
			{
				if (spiders[j].active &&
					l3RectCollision(playerSpells[i].x, playerSpells[i].y, 30, 30,
					spiders[j].x, spiders[j].y, 80, 70))
				{
					spiders[j].hp -= 10;
					playerSpells[i].active = false;

					if (spiders[j].hp <= 0)
					{
						spiders[j].hp = 0;
						spiders[j].active = false;
					}
					break;
				}
			}
		}
		else if (level3Phase == 2)
		{
			for (j = 0; j < L3_MAX_SMALL_ENEMIES; j++)
			{
				if (smallEnemies[j].active &&
					l3RectCollision(playerSpells[i].x, playerSpells[i].y, 30, 30,
					smallEnemies[j].x, smallEnemies[j].y, 70, 90))
				{
					smallEnemies[j].hp -= 10;
					playerSpells[i].active = false;

					if (smallEnemies[j].hp <= 0)
					{
						smallEnemies[j].hp = 0;
						smallEnemies[j].active = false;
					}
					break;
				}
			}
		}
		else if (level3Phase == 3 && level3GuardianActive)
		{
			if (l3RectCollision(playerSpells[i].x, playerSpells[i].y, 30, 30,
				level3GuardianX, level3GuardianY, 150, 170))
			{
				level3GuardianHP -= 10;
				playerSpells[i].active = false;

				if (level3GuardianHP <= 0)
				{
					level3GuardianHP = 0;
					level3GuardianActive = false;
					level3Won = true;

					playWinSound();      // NEW
				}
			}
		}
	}
}

void l3UpdateEnemySpells()
{
	int i;

	for (i = 0; i < L3_MAX_ENEMY_SPELLS; i++)
	{
		if (!enemySpells[i].active)
			continue;

		enemySpells[i].x += enemySpells[i].dx * enemySpells[i].speed / 10;
		enemySpells[i].y += enemySpells[i].dy * enemySpells[i].speed / 10;

		if (enemySpells[i].x < -50 || enemySpells[i].x > L3_SCREEN_WIDTH + 50 ||
			enemySpells[i].y < -50 || enemySpells[i].y > L3_SCREEN_HEIGHT + 50)
		{
			enemySpells[i].active = false;
			continue;
		}

		if (l3RectCollision(enemySpells[i].x, enemySpells[i].y, 30, 30,
			level3PlayerX, level3PlayerY,
			level3PlayerWidth, level3PlayerHeight))
		{
			l3DamagePlayer(enemySpells[i].damage);
			enemySpells[i].active = false;
		}
	}
}

// =====================================================
// UPDATE SPIDERS
// =====================================================

void l3UpdateSpiders()
{
	int i;

	for (i = 0; i < L3_MAX_SPIDERS; i++)
	{
		int dx, dy;

		if (!spiders[i].active)
			continue;

		dx = level3PlayerX - spiders[i].x;
		dy = level3PlayerY - spiders[i].y;

		if (dx > 2) spiders[i].x += 1;
		else if (dx < -2) spiders[i].x -= 1;

		if (dy > 2) spiders[i].y += 1;
		else if (dy < -2) spiders[i].y -= 1;

		if (spiders[i].attackCooldown > 0)
			spiders[i].attackCooldown--;

		if (l3RectCollision(level3PlayerX, level3PlayerY,
			level3PlayerWidth, level3PlayerHeight,
			spiders[i].x, spiders[i].y, 80, 70))
		{
			if (spiders[i].attackCooldown <= 0)
			{
				l3DamagePlayer(10);
				spiders[i].attackCooldown = 35;
			}
		}
	}
}

// =====================================================
// UPDATE SMALL ENEMIES
// =====================================================

void l3UpdateSmallEnemies()
{
	int i;

	for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
	{
		int dx, dy;

		if (!smallEnemies[i].active)
			continue;

		dx = level3PlayerX - smallEnemies[i].x;
		dy = level3PlayerY - smallEnemies[i].y;

		if (dx > 3) smallEnemies[i].x += 2;
		else if (dx < -3) smallEnemies[i].x -= 2;

		if (dy > 3) smallEnemies[i].y += 2;
		else if (dy < -3) smallEnemies[i].y -= 2;

		if (smallEnemies[i].attackCooldown > 0)
			smallEnemies[i].attackCooldown--;

		if (smallEnemies[i].shootCooldown > 0)
			smallEnemies[i].shootCooldown--;

		if (l3RectCollision(level3PlayerX, level3PlayerY,
			level3PlayerWidth, level3PlayerHeight,
			smallEnemies[i].x, smallEnemies[i].y, 70, 90))
		{
			if (smallEnemies[i].attackCooldown <= 0)
			{
				l3DamagePlayer(10);
				smallEnemies[i].attackCooldown = 30;
			}
		}

		if (smallEnemies[i].shootCooldown <= 0)
		{
			if (l3Distance(smallEnemies[i].x, smallEnemies[i].y,
				level3PlayerX, level3PlayerY) < 650)
			{
				// Slower, easier Phase 2 projectiles: reduced speed
				// (was 9) and a longer cooldown between shots (was
				// 90 + rand()%50).
				l3SpawnEnemySpell(smallEnemies[i].x + 35,
					smallEnemies[i].y + 45,
					10, 6, false);
			}

			smallEnemies[i].shootCooldown = 150 + rand() % 70;
		}
	}
}

// =====================================================
// UPDATE GUARDIAN
// =====================================================

void l3UpdateGuardian()
{
	int dx, dy;
	int speed;
	int distanceToPlayer;

	if (!level3GuardianActive)
		return;

	if (level3GuardianHP > 60)
		speed = 2;
	else if (level3GuardianHP > 30)
		speed = 3;
	else
		speed = 5;

	dx = level3PlayerX - level3GuardianX;
	dy = level3PlayerY - level3GuardianY;
	distanceToPlayer = l3Distance(level3GuardianX, level3GuardianY,
		level3PlayerX, level3PlayerY);

	if (dx > 5) level3GuardianX += speed;
	else if (dx < -5) level3GuardianX -= speed;

	if (dy > 5) level3GuardianY += speed;
	else if (dy < -5) level3GuardianY -= speed;

	if (level3GuardianX < 30) level3GuardianX = 30;
	if (level3GuardianY < 40) level3GuardianY = 40;
	if (level3GuardianX > L3_SCREEN_WIDTH - 180)
		level3GuardianX = L3_SCREEN_WIDTH - 180;
	if (level3GuardianY > L3_SCREEN_HEIGHT - 220)
		level3GuardianY = L3_SCREEN_HEIGHT - 220;

	if (level3GuardianPunchCooldown > 0)
		level3GuardianPunchCooldown--;

	if (level3GuardianMagicCooldown > 0)
		level3GuardianMagicCooldown--;

	// Punch: 15 damage
	if (distanceToPlayer < 150 && level3GuardianPunchCooldown <= 0)
	{
		l3DamagePlayer(15);

		if (level3GuardianHP > 60)
			level3GuardianPunchCooldown = 45;
		else if (level3GuardianHP > 30)
			level3GuardianPunchCooldown = 32;
		else
			level3GuardianPunchCooldown = 22;
	}

	// Ranged magic: 10 damage
	if (level3GuardianMagicCooldown <= 0)
	{
		if (level3GuardianHP > 60)
		{
			l3SpawnEnemySpell(level3GuardianX + 75,
				level3GuardianY + 80,
				10, 10, false);
			level3GuardianMagicCooldown = 100;
		}
		else if (level3GuardianHP > 30)
		{
			l3SpawnEnemySpell(level3GuardianX + 75,
				level3GuardianY + 80,
				10, 11, false);
			level3GuardianMagicCooldown = 60;
		}
		else
		{
			// Rage Mode: multiple magic orbs
			l3SpawnEnemySpell(level3GuardianX + 75,
				level3GuardianY + 80,
				10, 12, true);
			l3SpawnEnemySpell(level3GuardianX + 75,
				level3GuardianY + 80,
				10, 12, true);
			l3SpawnEnemySpell(level3GuardianX + 75,
				level3GuardianY + 80,
				10, 12, true);
			level3GuardianMagicCooldown = 38;
		}
	}
}

// =====================================================
// UPDATE LEVEL 3
// =====================================================

void updateLevel3()
{
	int i;
	bool anyActive;

	// R poll: reliably works from any Level 3 state (mid-game,
	// game over, or victory), same reasoning as SPACE/movement below.
	// Retries from the checkpoint (the phase currently reached),
	// not always a full reset back to Phase 1.
	static bool rPolledPressed = false;

	if (GetAsyncKeyState('R') & 0x8000)
	{
		if (!rPolledPressed)
		{
			rPolledPressed = true;
			l3RetryFromCheckpoint();
			return;
		}
	}
	else
	{
		rPolledPressed = false;
	}

	// M poll: return to Level Select. Polled the same way as R,
	// since the GLUT keyboard callback is not reliable in this
	// project (see comment on SPACE below).
	static bool mPolledPressed = false;

	if (GetAsyncKeyState('M') & 0x8000)
	{
		if (!mPolledPressed)
		{
			mPolledPressed = true;
			screen = 8;
			return;
		}
	}
	else
	{
		mPolledPressed = false;
	}

	if (level3GameOver || level3Won)
		return;

	level3FrameCounter++;

	if (level3PlayerDamageCooldown > 0)
		level3PlayerDamageCooldown--;

	if (level3DashCooldown > 0)
		level3DashCooldown--;

	if (level3DashInvulnerability > 0)
		level3DashInvulnerability--;

	// Gradual mana regeneration
	if (level3FrameCounter % 6 == 0 && level3Mana < level3MaxMana)
		level3Mana++;

	// Continuous WASD / arrow movement
	if (level3TransitionTimer <= 0)
	{
		if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000)
		{
			l3MovePlayer(0, level3PlayerSpeed);
			level3PlayerDirection = 1;
		}

		if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			l3MovePlayer(0, -level3PlayerSpeed);
			level3PlayerDirection = 0;
		}

		if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			l3MovePlayer(-level3PlayerSpeed, 0);
			level3PlayerDirection = 2;
		}

		if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			l3MovePlayer(level3PlayerSpeed, 0);
			level3PlayerDirection = 3;
		}

		// SHIFT = Dash
		if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && level3DashCooldown <= 0)
		{
			l3Dash();
		}

		// SPACE = Cast Spell
		// (polled directly with GetAsyncKeyState, same as movement,
		// instead of relying only on the GLUT keyboard callback,
		// which does not always fire reliably in this project)
		static bool spacePolledPressed = false;

		if ((GetAsyncKeyState(VK_SPACE) & 0x8000))
		{
			if (!spacePolledPressed)
			{
				spacePolledPressed = true;
				l3SpawnPlayerSpell();
			}
		}
		else
		{
			spacePolledPressed = false;
		}
	}

	// Dark transition between phases
	if (level3TransitionTimer > 0)
	{
		level3TransitionTimer--;

		if (level3Phase == 3 && level3TransitionTimer == 0)
			level3GuardianActive = true;

		return;
	}

	l3UpdatePlayerSpells();
	l3UpdateEnemySpells();

	if (level3Phase == 1)
	{
		l3UpdateSpiders();

		anyActive = false;
		for (i = 0; i < L3_MAX_SPIDERS; i++)
		{
			if (spiders[i].active)
			{
				anyActive = true;
				break;
			}
		}

		if (!anyActive)
			l3StartPhase2();
	}
	else if (level3Phase == 2)
	{
		l3UpdateSmallEnemies();

		anyActive = false;
		for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
		{
			if (smallEnemies[i].active)
			{
				anyActive = true;
				break;
			}
		}

		if (!anyActive)
			l3StartPhase3();
	}
	else if (level3Phase == 3)
	{
		l3UpdateGuardian();
	}
}

// =====================================================
// DRAW HP BAR
// =====================================================

void l3DrawBars()
{
	int hpWidth = level3PlayerHP * 3;
	int manaWidth = level3Mana * 3;

	iSetColor(40, 0, 0);
	iFilledRectangle(25, 720, 300, 20);

	iSetColor(0, 220, 0);
	iFilledRectangle(25, 720, hpWidth, 20);

	iSetColor(255, 255, 255);
	iRectangle(25, 720, 300, 20);

	iSetColor(20, 20, 80);
	iFilledRectangle(25, 685, 300, 20);

	iSetColor(50, 120, 255);
	iFilledRectangle(25, 685, manaWidth, 20);

	iSetColor(255, 255, 255);
	iRectangle(25, 685, 300, 20);

	iText(335, 722, "HP", GLUT_BITMAP_HELVETICA_18);
	iText(335, 687, "MANA", GLUT_BITMAP_HELVETICA_18);

	char hpText[30];
	char manaText[30];

	sprintf_s(hpText, "HP: %d / %d", level3PlayerHP, level3PlayerMaxHP);
	sprintf_s(manaText, "MANA: %d / %d", level3Mana, level3MaxMana);

	iText(390, 722, hpText, GLUT_BITMAP_HELVETICA_18);
	iText(390, 687, manaText, GLUT_BITMAP_HELVETICA_18);
}

// =====================================================
// DRAW LEVEL 3
// =====================================================

void drawLevel3()
{
	int i;

	iShowBMP(0, 0, "level3_assets\\arena.bmp");

	// Spiders
	for (i = 0; i < L3_MAX_SPIDERS; i++)
	{
		if (spiders[i].active)
		{
			iShowBMP2(spiders[i].x, spiders[i].y,
				"level3_assets\\spider.bmp", 0);

			iSetColor(80, 0, 0);
			iFilledRectangle(spiders[i].x, spiders[i].y + 75, 80, 8);

			iSetColor(255, 50, 50);
			iFilledRectangle(spiders[i].x, spiders[i].y + 75,
				(spiders[i].hp * 80) / 30, 8);
		}
	}

	// Small enemies
	for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
	{
		if (smallEnemies[i].active)
		{
			iShowBMP2(smallEnemies[i].x, smallEnemies[i].y,
				"level3_assets\\small_enemy.bmp", 0);

			iSetColor(80, 0, 0);
			iFilledRectangle(smallEnemies[i].x, smallEnemies[i].y + 92, 70, 8);

			iSetColor(255, 80, 80);
			iFilledRectangle(smallEnemies[i].x, smallEnemies[i].y + 92,
				(smallEnemies[i].hp * 70) / 40, 8);
		}
	}

	// Guardian
	if (level3GuardianActive)
	{
		iShowBMP2(level3GuardianX, level3GuardianY,
			"level3_assets\\guardian.bmp", 0);

		// Boss HP bar
		iSetColor(50, 0, 0);
		iFilledRectangle(500, 675, 365, 22);

		iSetColor(220, 0, 0);
		iFilledRectangle(500, 675,
			(level3GuardianHP * 365) / level3GuardianMaxHP,
			22);

		iSetColor(255, 255, 255);
		iRectangle(500, 675, 365, 22);

		iText(620, 705, "ARCANE GUARDIAN", GLUT_BITMAP_HELVETICA_18);

		if (level3GuardianHP <= 30)
		{
			iSetColor(255, 70, 70);
			iText(905, 680, "RAGE MODE!", GLUT_BITMAP_HELVETICA_18);
		}
	}

	// Player
	if (level3PlayerDirection == 1)
		iShowBMP2(level3PlayerX, level3PlayerY,
		"level3_assets\\player_up.bmp", 0);
	else if (level3PlayerDirection == 2)
		iShowBMP2(level3PlayerX, level3PlayerY,
		"level3_assets\\player_left.bmp", 0);
	else if (level3PlayerDirection == 3)
		iShowBMP2(level3PlayerX, level3PlayerY,
		"level3_assets\\player_right.bmp", 0);
	else
		iShowBMP2(level3PlayerX, level3PlayerY,
		"level3_assets\\player_down.bmp", 0);

	// Player spells
	for (i = 0; i < L3_MAX_PLAYER_SPELLS; i++)
	{
		if (playerSpells[i].active)
			iShowBMP2(playerSpells[i].x, playerSpells[i].y,
			"level3_assets\\spell.bmp", 0);
	}

	// Enemy spells
	for (i = 0; i < L3_MAX_ENEMY_SPELLS; i++)
	{
		if (enemySpells[i].active)
			iShowBMP2(enemySpells[i].x, enemySpells[i].y,
			"level3_assets\\guardian_spell.bmp", 0);
	}

	// HUD
	glDisable(GL_TEXTURE_2D);
	l3DrawBars();

	iSetColor(255, 255, 255);

	if (level3Phase == 1)
		iText(1080, 735, "PHASE 1: SPIDERS", GLUT_BITMAP_HELVETICA_18);
	else if (level3Phase == 2)
		iText(1030, 735, "PHASE 2: DARK MINIONS", GLUT_BITMAP_HELVETICA_18);
	else
		iText(1050, 735, "PHASE 3: GUARDIAN", GLUT_BITMAP_HELVETICA_18);

	iSetColor(220, 220, 220);
	iText(20, 15,
		"WASD / ARROWS = Move   SPACE = Magic   SHIFT = Dash   R = Retry   M = Level Select",
		GLUT_BITMAP_HELVETICA_12);

	if (level3DashCooldown == 0)
		iText(20, 45, "DASH READY", GLUT_BITMAP_HELVETICA_12);
	else
		iText(20, 45, "DASH COOLING DOWN", GLUT_BITMAP_HELVETICA_12);

	// Phase transition overlay
	if (level3TransitionTimer > 0)
	{
		iSetColor(0, 0, 0);
		iFilledRectangle(0, 0, L3_SCREEN_WIDTH, L3_SCREEN_HEIGHT);

		iSetColor(220, 180, 255);

		if (level3Phase == 2)
			iText(540, 400, "PHASE 2", GLUT_BITMAP_TIMES_ROMAN_24);
		else
			iText(520, 400, "THE ARCANE GUARDIAN", GLUT_BITMAP_TIMES_ROMAN_24);

		iSetColor(255, 255, 255);
		iText(500, 350, "Prepare yourself...", GLUT_BITMAP_HELVETICA_18);
	}

	// Game Over (same box style as Level 1 and Level 2)
	if (level3GameOver)
	{
		char phaseInfo[60];

		sprintf_s(phaseInfo, "You fell in Phase %d of 3", level3Phase);

		drawGameOverBox(
			"GAME OVER",
			"The Final Trial defeated you.",
			phaseInfo,
			"Press R to Retry (from this phase)",
			"Press M for Level Select");
	}

	// Victory (same box design as Level 1)
	if (level3Won)
	{
		drawVictoryBox(
			"LEVEL 3 COMPLETE!",
			"The Arcane Guardian has fallen.",
			"You completed the Final Trial!",
			"Press R to Play Again",
			"Press M for Level Select");
	}
}

// =====================================================
// KEYBOARD
// =====================================================

void level3Keyboard(unsigned char key)
{
	if (level3GameOver)
	{
		if (key == 'r' || key == 'R')
			l3RetryFromCheckpoint();
		return;
	}

	if (level3Won)
	{
		if (key == 'r' || key == 'R')
			l3RetryFromCheckpoint();
		return;
	}

	// Movement (WASD) and SPACE (spell cast) are handled by
	// GetAsyncKeyState polling inside updateLevel3() instead,
	// since that is more reliable than this GLUT keyboard event
	// callback in this project. R and M are also polled inside
	// updateLevel3() now so they work reliably in every state
	// (mid-game, game over, victory); this handler is kept only
	// as a harmless fallback.
	if (key == 'r' || key == 'R')
	{
		l3RetryFromCheckpoint();
	}
}

// =====================================================
// SPECIAL KEYBOARD - ARROW KEYS
// =====================================================

void level3SpecialKeyboard(int key)
{
	if (level3GameOver || level3Won)
		return;

	if (key == GLUT_KEY_UP)
	{
		level3PlayerDirection = 1;
		l3MovePlayer(0, level3PlayerSpeed);
	}
	else if (key == GLUT_KEY_DOWN)
	{
		level3PlayerDirection = 0;
		l3MovePlayer(0, -level3PlayerSpeed);
	}
	else if (key == GLUT_KEY_LEFT)
	{
		level3PlayerDirection = 2;
		l3MovePlayer(-level3PlayerSpeed, 0);
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		level3PlayerDirection = 3;
		l3MovePlayer(level3PlayerSpeed, 0);
	}
}

// =====================================================
// MOUSE
// =====================================================

void level3Mouse(int button, int state, int x, int y)
{
	// Level 3 currently uses keyboard controls.
	// Mouse function is kept so the main project can call it safely.
}

void level3MouseMove(int x, int y)
{
}

void level3MousePassiveMove(int x, int y)
{
}




// ======================================================
// STORY SYSTEM
// ======================================================

int storyPage = 1;
bool storyTimerStarted = false;

void nextStory()
{
	if (storyPage < 4)
		storyPage++;
}


// ======================================================
// MAIN MENU
// ======================================================

bool startHover = false;
bool howHover = false;
bool creditsHover = false;
bool exitHover = false;
bool backHover = false;

int startX1 = 400, startX2 = 830;
int startY1 = 440, startY2 = 510;

int howX1 = 400, howX2 = 830;
int howY1 = 260, howY2 = 330;

int creditsX1 = 400, creditsX2 = 830;
int creditsY1 = 170, creditsY2 = 240;

int exitX1 = 400, exitX2 = 830;
int exitY1 = 80, exitY2 = 150;


// ======================================================
// LEVEL SELECT
// ======================================================

bool level1Unlocked = true;      // Level 1 is always unlocked
bool level2Unlocked = false;     // Unlocked when Level 1 is completed
bool level3Unlocked = false;     // Unlocked when Level 2 is completed

// Click / hover regions for the 3 cards on the level_select.jpg background.
// Tune these numbers to match exactly where your card art sits on the image.
// NOTE: iGraphics Y coordinates grow UPWARD from the bottom of the screen.
// Level 2 and Level 3 were shifted left (to the left) so the locked
// overlay covers the card art properly. Lower the X numbers to move
// further left, raise them to move right.
int lvl1X1 = 120, lvl1Y1 = 110, lvl1X2 = 430, lvl1Y2 = 580;
int lvl2X1 = 520, lvl2Y1 = 110, lvl2X2 = 850, lvl2Y2 = 580;
int lvl3X1 = 950, lvl3Y1 = 110, lvl3X2 = 1280, lvl3Y2 = 580;

bool lvl1Hover = false;
bool lvl2Hover = false;
bool lvl3Hover = false;


// ======================================================
// HIGHSCORE SYSTEM (Level 1)
// ======================================================

#define MAX_HIGHSCORES 5

struct HighscoreEntry
{
	char name[20];
	int score;
};

HighscoreEntry highscores[MAX_HIGHSCORES];
int highscoreCount = 0;

// Name typed by the player before Level 1 starts
char playerNameInput[20] = "";
int playerNameLength = 0;

// Main menu "HIGHSCORE" button
bool highscoreHover = false;

int highscoreX1 = 400, highscoreX2 = 830;
int highscoreY1 = 350, highscoreY2 = 420;


// ======================================================
// TEXT CENTER
// ======================================================

int textPixelWidth(void* font, const char* str)
{
	int w = 0;

	for (const char* c = str; *c; c++)
	{
		w += glutBitmapWidth(font, *c);
	}

	return w;
}

void iTextCentered(int centerX, int y, const char* str, void* font)
{
	int w = textPixelWidth(font, str);

	iText(centerX - w / 2, y, (char*)str, font);
}


// ======================================================
// BUTTON
// ======================================================

void drawButton(int x1, int y1, int x2, int y2, char text[])
{
	iSetColor(20, 10, 35);
	iFilledRectangle(x1, y1, x2 - x1, y2 - y1);

	iSetColor(180, 120, 255);
	iRectangle(x1, y1, x2 - x1, y2 - y1);

	iSetColor(255, 255, 255);

	int textWidth =
		glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24,
		(unsigned char*)text);

	iText(
		x1 + ((x2 - x1) - textWidth) / 2,
		y1 + 23,
		text,
		GLUT_BITMAP_TIMES_ROMAN_24
		);
}


void drawHoverButton(int x1, int y1, int x2, int y2, char text[])
{
	iSetColor(220, 170, 255);
	iRectangle(
		x1 - 3,
		y1 - 3,
		x2 - x1 + 6,
		y2 - y1 + 6
		);

	iSetColor(65, 30, 100);

	iFilledRectangle(
		x1,
		y1,
		x2 - x1,
		y2 - y1
		);

	iSetColor(255, 220, 255);

	iRectangle(
		x1,
		y1,
		x2 - x1,
		y2 - y1
		);

	iSetColor(255, 255, 255);

	int textWidth =
		glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24,
		(unsigned char*)text);

	iText(
		x1 + ((x2 - x1) - textWidth) / 2,
		y1 + 23,
		text,
		GLUT_BITMAP_TIMES_ROMAN_24
		);
}


// ======================================================
// LEVEL 1 MAZE
// ======================================================

const int ROWS = 10;
const int COLS = 30;

int maze[ROWS][COLS] =
{
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

	{ 1, 0, 0, 2, 0, 1, 0, 0, 0, 1, 0, 2, 0, 0, 1, 0, 0, 2, 0, 1, 0, 0, 2, 0, 1, 0, 0, 2, 0, 1 },

	{ 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1 },

	{ 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1 },

	{ 0, 0, 1, 0, 1, 1, 1, 3, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 3, 0, 4 },

	{ 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },

	{ 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1 },

	{ 1, 0, 0, 2, 0, 0, 1, 0, 1, 0, 0, 2, 0, 0, 1, 0, 0, 2, 0, 1, 0, 0, 2, 0, 1, 0, 0, 2, 0, 1 },

	{ 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1 },

	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

int initialMaze[ROWS][COLS];

int cellSize = 60;
int yOffset = 80;

int score = 0;
int lives = 3;

bool isGameOver = false;
bool isLevelCompleted = false;

float winAnimTimer = 0.0f;

const int START_ROW = 4;
const int START_COL = 0;

int playerRow = START_ROW;
int playerCol = START_COL;

int playerDirection = 0;
// 0 = Down
// 1 = Up
// 2 = Left
// 3 = Right

float cameraX = 0;


// ======================================================
// LEVEL 1 ASSETS
// ======================================================

char coinImg[] = "assets\\coin.bmp";
char bookImg[] = "assets\\book.bmp";
char gateImg[] = "assets\\gate.bmp";

char playerDown[] = "assets\\P_down.bmp";
char playerUp[] = "assets\\P_up.bmp";
char playerLeft[] = "assets\\P_left.bmp";
char playerRight[] = "assets\\P_right.bmp";


// ======================================================
// ENEMIES
// ======================================================

const int NUM_ENEMIES = 3;

struct Enemy
{
	int row;
	int col;
	int dir;
	int minCol;
	int maxCol;
};

Enemy enemies[NUM_ENEMIES] =
{
	{ 1, 6, 1, 5, 10 },
	{ 7, 12, 1, 9, 15 },
	{ 1, 16, 1, 15, 20 }
};


// ======================================================
// AUDIO
// ======================================================
void playLevel2CatchSound()
{
	mciSendString(
		"close catchSound",
		NULL,
		0,
		NULL
		);

	mciSendString(
		"open \"level2_assets\\catch.wav\" type waveaudio alias catchSound",
		NULL,
		0,
		NULL
		);

	mciSendString(
		"play catchSound from 0",
		NULL,
		0,
		NULL
		);
}

void playLevel3SpellSound()
{
	mciSendString(
		"close spellSound",
		NULL,
		0,
		NULL
		);

	mciSendString(
		"open \"level3_assets\\spellsound.wav\" type waveaudio alias spellSound",
		NULL,
		0,
		NULL
		);

	mciSendString(
		"play spellSound from 0",
		NULL,
		0,
		NULL
		);
}

// Explicit close -> open -> play pattern (same as the catch/spell
// sounds above), used for the Level 1 / 2 / 3 win sound so a
// stale/left-open MCI alias from a previous play can't silently
// block it from playing again on retry.
void playWinSound()
{
	mciSendString(
		"close winSound",
		NULL,
		0,
		NULL
		);

	mciSendString(
		"open \"assets\\win.mp3\" type mpegvideo alias winSound",
		NULL,
		0,
		NULL
		);

	mciSendString(
		"play winSound from 0",
		NULL,
		0,
		NULL
		);
}

// Level 1 coin pickup sound (assets\coin.mp3).
// The file is opened ONCE at startup (initCoinSound) and each pickup
// just rewinds and plays it. Opening/closing an mp3 on every pickup
// is slow and can silently fail on some Windows setups.
// Several open methods are tried in order, so it still works if the
// mp3 needs a different MCI device type.
bool coinSoundReady = false;
bool coinSoundTried = false;

void initCoinSound()
{
	coinSoundTried = true;

	mciSendString("close coinSound", NULL, 0, NULL);

	if (mciSendString(
		"open \"assets\\coin.wav\" type waveaudio alias coinSound",
		NULL, 0, NULL) == 0)
	{
		coinSoundReady = true;
	}
	else if (mciSendString(
		"open \"assets\\coin.mp3\" type mpegvideo alias coinSound",
		NULL, 0, NULL) == 0)
	{
		coinSoundReady = true;
	}
	else if (mciSendString(
		"open \"assets\\coin.mp3\" alias coinSound",
		NULL, 0, NULL) == 0)
	{
		coinSoundReady = true;
	}
	else if (mciSendString(
		"open \"assets\\coin.mp3\" type waveaudio alias coinSound",
		NULL, 0, NULL) == 0)
	{
		coinSoundReady = true;
	}
	else
	{
		coinSoundReady = false;
	}
}

// Loaded lazily on the first coin pickup (NOT at startup), so nothing
// audio related runs before the game window exists.
void playCoinSound()
{
	if (!coinSoundTried)
		initCoinSound();

	if (!coinSoundReady)
		return;

	mciSendString("seek coinSound to start", NULL, 0, NULL);
	mciSendString("play coinSound", NULL, 0, NULL);
}

// Game over sound: tries assets\gameover.wav first (waveaudio, same
// as catch/spell sounds), then falls back to assets\gameover.mp3.
void playGameOverSound()
{
	mciSendString("close gameoverSound", NULL, 0, NULL);

	if (mciSendString(
		"open \"assets\\gameover.wav\" type waveaudio alias gameoverSound",
		NULL, 0, NULL) != 0)
	{
		if (mciSendString(
			"open \"assets\\gameover.mp3\" type mpegvideo alias gameoverSound",
			NULL, 0, NULL) != 0)
		{
			mciSendString(
				"open \"assets\\gameover.mp3\" alias gameoverSound",
				NULL, 0, NULL);
		}
	}

	mciSendString("play gameoverSound from 0", NULL, 0, NULL);
}

void playBGM()
{
	PlaySound(
		TEXT("assets\\bgm.wav"),
		NULL,
		SND_FILENAME | SND_ASYNC | SND_LOOP
		);
}

void stopBGM()
{
	PlaySound(NULL, NULL, 0);
}


// ======================================================
// HIGHSCORE FILE I/O (text file: highscore.txt)
// One line per entry -> "name score"
// ======================================================

void loadHighscores()
{
	FILE* fp;
	errno_t err;

	highscoreCount = 0;

	err = fopen_s(&fp, "highscore.txt", "r");

	if (err != 0 || fp == NULL)
		return;

	while (
		highscoreCount < MAX_HIGHSCORES &&
		fscanf_s(fp, "%19s %d",
		highscores[highscoreCount].name,
		(unsigned)sizeof(highscores[highscoreCount].name),
		&highscores[highscoreCount].score) == 2
		)
	{
		highscoreCount++;
	}

	fclose(fp);
}

void saveHighscores()
{
	FILE* fp;
	errno_t err;
	int i;

	err = fopen_s(&fp, "highscore.txt", "w");

	if (err != 0 || fp == NULL)
		return;

	for (i = 0; i < highscoreCount; i++)
	{
		fprintf(fp, "%s %d\n",
			highscores[i].name,
			highscores[i].score);
	}

	fclose(fp);
}

// Sorts highscores[] descending by score (simple bubble sort,
// list is always tiny: max 5 entries)
void sortHighscores()
{
	int i, j;
	HighscoreEntry temp;

	for (i = 0; i < highscoreCount - 1; i++)
	{
		for (j = 0; j < highscoreCount - 1 - i; j++)
		{
			if (highscores[j].score < highscores[j + 1].score)
			{
				temp = highscores[j];
				highscores[j] = highscores[j + 1];
				highscores[j + 1] = temp;
			}
		}
	}
}

// Called when Level 1 is completed. Adds (name, newScore) to the
// top-5 list only if it actually qualifies, then re-sorts and
// re-saves the file.
void addHighscore(const char name[], int newScore)
{
	int i;
	int minIndex;

	if (highscoreCount < MAX_HIGHSCORES)
	{
		strcpy_s(highscores[highscoreCount].name, name);
		highscores[highscoreCount].score = newScore;
		highscoreCount++;
	}
	else
	{
		minIndex = 0;

		for (i = 1; i < MAX_HIGHSCORES; i++)
		{
			if (highscores[i].score < highscores[minIndex].score)
				minIndex = i;
		}

		if (newScore <= highscores[minIndex].score)
			return; // doesn't make the top 5

		strcpy_s(highscores[minIndex].name, name);
		highscores[minIndex].score = newScore;
	}

	sortHighscores();
	saveHighscores();
}


// ======================================================
// RESET LEVEL 1
// ======================================================

void resetGame()
{
	score = 0;
	lives = 3;

	playerRow = START_ROW;
	playerCol = START_COL;

	cameraX = 0;

	playerDirection = 0;

	isGameOver = false;
	isLevelCompleted = false;

	for (int r = 0; r < ROWS; r++)
	{
		for (int c = 0; c < COLS; c++)
		{
			maze[r][c] = initialMaze[r][c];
		}
	}

	enemies[0] = { 1, 6, 1, 5, 10 };
	enemies[1] = { 7, 12, 1, 9, 15 };
	enemies[2] = { 1, 16, 1, 15, 20 };

	mciSendString("stop gameoverSound", NULL, 0, NULL);
	mciSendString("stop winSound", NULL, 0, NULL);

	playBGM();
}


// ======================================================
// STORY DRAW
// ======================================================

void drawStory()
{
	if (storyPage == 1)
	{
		iShowBMP(0, 0, "assets\\story4.bmp");
	}
	else if (storyPage == 2)
	{
		iShowBMP(0, 0, "assets\\story1.bmp");
	}
	else if (storyPage == 3)
	{
		iShowBMP(0, 0, "assets\\story2.bmp");
	}
	else if (storyPage == 4)
	{
		iShowBMP(0, 0, "assets\\story3.bmp");

		glDisable(GL_TEXTURE_2D);

		iSetColor(255, 255, 255);

		iTextCentered(
			SCREEN_WIDTH / 2,
			50,
			"PRESS SPACE TO START GAME",
			GLUT_BITMAP_TIMES_ROMAN_24
			);
	}
}


// ======================================================
// LOCKED CARD OVERLAY
// A much darker semi-transparent overlay so the locked
// card is clearly dimmed, while the level select art is
// still very faintly visible behind it.
// ======================================================

void drawLockedOverlay(int x, int y, int width, int height)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0.0f, 0.0f, 0.0f, 0.92f);   // was 0.65f -> darker

	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x + width, y);
	glVertex2i(x + width, y + height);
	glVertex2i(x, y + height);
	glEnd();

	glDisable(GL_BLEND);

	// Reset to fully opaque white so the iGraphics text/shape
	// calls that follow (which rely on iSetColor) aren't left
	// tinted by the alpha color set above.
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}


// ======================================================
// LEVEL SELECT DRAW
// ======================================================

void drawLevelSelect()
{
	iShowBMP(0, 0, "assets\\level_select_bg.bmp");

	glDisable(GL_TEXTURE_2D);

	// ---------------- LEVEL 1 (always unlocked) ----------------
	if (lvl1Hover)
	{
		iSetColor(255, 255, 255);
		iRectangle(
			lvl1X1 - 5,
			lvl1Y1 - 5,
			(lvl1X2 - lvl1X1) + 10,
			(lvl1Y2 - lvl1Y1) + 10
			);
	}

	// ---------------- LEVEL 2 ----------------
	if (!level2Unlocked)
	{
		drawLockedOverlay(
			lvl2X1,
			lvl2Y1,
			lvl2X2 - lvl2X1,
			lvl2Y2 - lvl2Y1
			);

		iSetColor(255, 255, 255);
		iTextCentered(
			(lvl2X1 + lvl2X2) / 2,
			(lvl2Y1 + lvl2Y2) / 2 + 10,
			"LOCKED",
			GLUT_BITMAP_TIMES_ROMAN_24
			);

		iTextCentered(
			(lvl2X1 + lvl2X2) / 2,
			(lvl2Y1 + lvl2Y2) / 2 - 25,
			"Complete Level 1 first",
			GLUT_BITMAP_HELVETICA_18
			);
	}
	else if (lvl2Hover)
	{
		iSetColor(255, 255, 255);
		iRectangle(
			lvl2X1 - 5,
			lvl2Y1 - 5,
			(lvl2X2 - lvl2X1) + 10,
			(lvl2Y2 - lvl2Y1) + 10
			);
	}

	// ---------------- LEVEL 3 ----------------
	if (!level3Unlocked)
	{
		drawLockedOverlay(
			lvl3X1,
			lvl3Y1,
			lvl3X2 - lvl3X1,
			lvl3Y2 - lvl3Y1
			);

		iSetColor(255, 255, 255);
		iTextCentered(
			(lvl3X1 + lvl3X2) / 2,
			(lvl3Y1 + lvl3Y2) / 2 + 10,
			"LOCKED",
			GLUT_BITMAP_TIMES_ROMAN_24
			);

		iTextCentered(
			(lvl3X1 + lvl3X2) / 2,
			(lvl3Y1 + lvl3Y2) / 2 - 25,
			"Complete Level 2 first",
			GLUT_BITMAP_HELVETICA_18
			);
	}
	else if (lvl3Hover)
	{
		iSetColor(255, 255, 255);
		iRectangle(
			lvl3X1 - 5,
			lvl3Y1 - 5,
			(lvl3X2 - lvl3X1) + 10,
			(lvl3Y2 - lvl3Y1) + 10
			);
	}
}


// ======================================================
// NAME INPUT DRAW (Screen 10 - before Level 1)
// ======================================================

void drawNameInput()
{
	iSetColor(15, 10, 30);
	iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// ---------------- WELCOME BANNER ----------------
	// GLUT bitmap fonts are fixed-size (TIMES_ROMAN_24 is the
	// biggest one available in iGraphics), so a soft glow
	// (the same text drawn several times, slightly offset, in a
	// dim color, then the bright text on top) is used to make
	// this read like a big decorated title instead of flat text.
	{
		char welcomeText[] = "WELCOME TO WRAITHMOOR ACADEMY OF WITCHCRAFT AND WIZARDRY";
		int bannerY = 640;

		// Glow layers (dim purple, offset in a small ring around the text)
		iSetColor(90, 40, 140);

		iTextCentered(SCREEN_WIDTH / 2 - 2, bannerY - 2, welcomeText, GLUT_BITMAP_TIMES_ROMAN_24);
		iTextCentered(SCREEN_WIDTH / 2 + 2, bannerY - 2, welcomeText, GLUT_BITMAP_TIMES_ROMAN_24);
		iTextCentered(SCREEN_WIDTH / 2 - 2, bannerY + 2, welcomeText, GLUT_BITMAP_TIMES_ROMAN_24);
		iTextCentered(SCREEN_WIDTH / 2 + 2, bannerY + 2, welcomeText, GLUT_BITMAP_TIMES_ROMAN_24);

		// Bright main text on top of the glow
		iSetColor(230, 200, 255);
		iTextCentered(SCREEN_WIDTH / 2, bannerY, welcomeText, GLUT_BITMAP_TIMES_ROMAN_24);

		// Decorative underline
		iSetColor(180, 120, 255);
		iFilledRectangle(SCREEN_WIDTH / 2 - 260, bannerY - 25, 520, 2);
	}

	iSetColor(255, 215, 0);
	iTextCentered(
		SCREEN_WIDTH / 2,
		470,
		"ENTER YOUR NAME",
		GLUT_BITMAP_TIMES_ROMAN_24
		);

	// Input box
	iSetColor(65, 30, 100);
	iFilledRectangle(
		SCREEN_WIDTH / 2 - 200,
		400,
		400,
		50
		);

	iSetColor(180, 120, 255);
	iRectangle(
		SCREEN_WIDTH / 2 - 200,
		400,
		400,
		50
		);

	iSetColor(255, 255, 255);
	iTextCentered(
		SCREEN_WIDTH / 2,
		418,
		playerNameInput,
		GLUT_BITMAP_TIMES_ROMAN_24
		);

	iSetColor(200, 200, 200);
	iTextCentered(
		SCREEN_WIDTH / 2,
		350,
		"Type your name (max 19 letters)",
		GLUT_BITMAP_HELVETICA_18
		);

	iSetColor(150, 255, 150);
	iTextCentered(
		SCREEN_WIDTH / 2,
		310,
		"Press ENTER to continue    |    BACKSPACE to erase",
		GLUT_BITMAP_HELVETICA_18
		);
}


// ======================================================
// HIGHSCORE BOARD DRAW (Screen 11 - from Main Menu)
// ======================================================

void drawHighscoreBoard()
{
	int i;
	char line[60];
	int startY = 500;

	iSetColor(15, 10, 30);
	iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	iSetColor(255, 215, 0);
	iTextCentered(
		SCREEN_WIDTH / 2,
		600,
		"TOP 5 HIGHSCORES - LEVEL 1",
		GLUT_BITMAP_TIMES_ROMAN_24
		);

	if (highscoreCount == 0)
	{
		iSetColor(200, 200, 200);
		iTextCentered(
			SCREEN_WIDTH / 2,
			500,
			"No highscores yet - be the first!",
			GLUT_BITMAP_HELVETICA_18
			);
	}
	else
	{
		for (i = 0; i < highscoreCount; i++)
		{
			sprintf_s(
				line,
				"%d.  %s  -  %d",
				i + 1,
				highscores[i].name,
				highscores[i].score
				);

			iSetColor(255, 255, 255);
			iTextCentered(
				SCREEN_WIDTH / 2,
				startY - i * 45,
				line,
				GLUT_BITMAP_TIMES_ROMAN_24
				);
		}
	}

	iSetColor(200, 200, 200);
	iTextCentered(
		SCREEN_WIDTH / 2,
		150,
		"Press M or click BACK to return to Main Menu",
		GLUT_BITMAP_HELVETICA_18
		);

	if (backHover)
		drawHoverButton(15, 30, 180, 90, "BACK");
	else
		drawButton(15, 30, 180, 90, "BACK");
}


// ======================================================
// VICTORY BOX (shared by Level 2 and Level 3,
// same design as the Level 1 victory box)
// ======================================================

void drawVictoryBox(const char* title, const char* line1, const char* line2,
	const char* hint1, const char* hint2)
{
	int cx = SCREEN_WIDTH / 2;
	int cy = SCREEN_HEIGHT / 2;

	iSetColor(30, 20, 50);
	iFilledRectangle(cx - 250, cy - 140, 500, 280);

	iSetColor(255, 215, 0);
	iRectangle(cx - 250, cy - 140, 500, 280);

	winAnimTimer += 0.05f;
	int glow = (int)(128 + 127 * sin(winAnimTimer));

	iSetColor(50, 255, glow);
	iTextCentered(cx, cy + 85, title, GLUT_BITMAP_TIMES_ROMAN_24);

	iSetColor(255, 255, 255);
	iTextCentered(cx, cy + 35, line1, GLUT_BITMAP_HELVETICA_18);
	iTextCentered(cx, cy - 5, line2, GLUT_BITMAP_HELVETICA_18);

	iSetColor(100, 255, 180);
	iTextCentered(cx, cy - 55, hint1, GLUT_BITMAP_HELVETICA_18);

	iSetColor(200, 200, 200);
	iTextCentered(cx, cy - 100, hint2, GLUT_BITMAP_HELVETICA_18);
}


// ======================================================
// GAME OVER BOX (shared by Level 1, Level 2 and Level 3)
// Dim overlay + dark red panel with a pulsing red title.
// ======================================================

float gameOverAnimTimer = 0.0f;

void drawGameOverBox(const char* title, const char* line1, const char* line2,
	const char* hint1, const char* hint2)
{
	int cx = SCREEN_WIDTH / 2;
	int cy = SCREEN_HEIGHT / 2;

	// Dim the whole screen
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.78f);

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(SCREEN_WIDTH, 0);
	glVertex2i(SCREEN_WIDTH, SCREEN_HEIGHT);
	glVertex2i(0, SCREEN_HEIGHT);
	glEnd();

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Panel
	iSetColor(38, 8, 16);
	iFilledRectangle(cx - 270, cy - 150, 540, 300);

	// Double red border
	iSetColor(190, 30, 40);
	iRectangle(cx - 270, cy - 150, 540, 300);
	iSetColor(120, 15, 25);
	iRectangle(cx - 264, cy - 144, 528, 288);

	// Pulsing title
	gameOverAnimTimer += 0.06f;
	int pulse = (int)(200 + 55 * sin(gameOverAnimTimer));

	iSetColor(pulse, 40, 40);
	iTextCentered(cx, cy + 90, title, GLUT_BITMAP_TIMES_ROMAN_24);

	// Divider
	iSetColor(190, 30, 40);
	iFilledRectangle(cx - 150, cy + 75, 300, 2);

	iSetColor(255, 255, 255);
	iTextCentered(cx, cy + 35, line1, GLUT_BITMAP_HELVETICA_18);

	iSetColor(255, 200, 120);
	iTextCentered(cx, cy - 5, line2, GLUT_BITMAP_HELVETICA_18);

	iSetColor(255, 140, 140);
	iTextCentered(cx, cy - 60, hint1, GLUT_BITMAP_HELVETICA_18);

	iSetColor(200, 200, 200);
	iTextCentered(cx, cy - 100, hint2, GLUT_BITMAP_HELVETICA_18);
}


// ======================================================
// LEVEL 1 DRAW
// ======================================================

void drawLevel1()
{
	if (isGameOver)
	{
		glDisable(GL_TEXTURE_2D);

		char goScore[40];

		sprintf_s(goScore, "Final Score: %d", score);

		drawGameOverBox(
			"GAME OVER!",
			"The corridors of Wraithmoor claimed you.",
			goScore,
			"Press R to Restart Level 1",
			"Press M for Level Select");

		return;
	}


	// Maze
	for (int r = 0; r < ROWS; r++)
	{
		for (int c = 0; c < COLS; c++)
		{
			int px = (int)(c * cellSize - cameraX);
			int py = (ROWS - 1 - r) * cellSize + yOffset;

			if (px + cellSize >= 0 && px <= SCREEN_WIDTH)
			{
				if (maze[r][c] == 1)
				{
					iSetColor(55, 30, 75);
					iFilledRectangle(px, py, cellSize, cellSize);

					iSetColor(85, 45, 110);

					iFilledRectangle(
						px + 3,
						py + 3,
						26,
						24
						);

					iFilledRectangle(
						px + 31,
						py + 3,
						26,
						24
						);

					iFilledRectangle(
						px + 3,
						py + 30,
						54,
						26
						);

					iSetColor(120, 70, 150);

					iRectangle(
						px,
						py,
						cellSize,
						cellSize
						);
				}
				else
				{
					iSetColor(0, 0, 0);

					iFilledRectangle(
						px,
						py,
						cellSize,
						cellSize
						);

					if (maze[r][c] == 2)
						iShowBMP(px, py, coinImg);

					if (maze[r][c] == 3)
						iShowBMP(px, py, bookImg);

					if (maze[r][c] == 4)
						iShowBMP(px, py, gateImg);
				}
			}
		}
	}


	// Enemies
	for (int i = 0; i < NUM_ENEMIES; i++)
	{
		int ePx =
			(int)(enemies[i].col * cellSize - cameraX);

		int ePy =
			(ROWS - 1 - enemies[i].row) * cellSize + yOffset;

		if (ePx + cellSize >= 0 && ePx <= SCREEN_WIDTH)
		{
			iSetColor(230, 40, 40);

			iFilledCircle(
				ePx + cellSize / 2,
				ePy + cellSize / 2,
				20
				);

			iSetColor(0, 0, 0);

			iFilledCircle(
				ePx + cellSize / 2 - 6,
				ePy + cellSize / 2 + 4,
				4
				);

			iFilledCircle(
				ePx + cellSize / 2 + 6,
				ePy + cellSize / 2 + 4,
				4
				);
		}
	}


	// Player
	int playerPx =
		(int)(playerCol * cellSize - cameraX);

	int playerPy =
		(ROWS - 1 - playerRow) * cellSize + yOffset;

	if (playerPx + cellSize >= 0 && playerPx <= SCREEN_WIDTH)
	{
		if (playerDirection == 1)
			iShowBMP2(playerPx, playerPy, playerUp, 0);

		else if (playerDirection == 2)
			iShowBMP2(playerPx, playerPy, playerLeft, 0);

		else if (playerDirection == 3)
			iShowBMP2(playerPx, playerPy, playerRight, 0);

		else
			iShowBMP2(playerPx, playerPy, playerDown, 0);
	}


	// HUD
	glDisable(GL_TEXTURE_2D);

	char scoreStr[30];
	char livesStr[30];

	sprintf_s(scoreStr, "SCORE: %d", score);
	sprintf_s(livesStr, "LIVES: %d", lives);

	iSetColor(255, 215, 0);

	iText(
		20,
		730,
		scoreStr,
		GLUT_BITMAP_HELVETICA_18
		);

	iSetColor(255, 80, 80);

	iText(
		1180,
		730,
		livesStr,
		GLUT_BITMAP_HELVETICA_18
		);


	// CONTROLS HINT
	iSetColor(180, 180, 180);

	iText(
		20,
		705,
		"R = Restart Level 1    |    N = Unlock Level 2 (after win)    |    M = Level Select",
		GLUT_BITMAP_HELVETICA_12
		);


	// Victory
	if (isLevelCompleted)
	{
		iSetColor(30, 20, 50);

		iFilledRectangle(
			SCREEN_WIDTH / 2 - 200,
			SCREEN_HEIGHT / 2 - 120,
			400,
			240
			);

		iSetColor(255, 215, 0);

		iRectangle(
			SCREEN_WIDTH / 2 - 200,
			SCREEN_HEIGHT / 2 - 120,
			400,
			240
			);

		winAnimTimer += 0.05f;

		int glow =
			(int)(128 + 127 * sin(winAnimTimer));

		iSetColor(50, 255, glow);

		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 + 60,
			"VICTORY! LEVEL CLEAR!",
			GLUT_BITMAP_TIMES_ROMAN_24
			);

		char finalScore[40];

		sprintf_s(
			finalScore,
			"TOTAL SCORE : %d",
			score
			);

		iSetColor(255, 255, 255);

		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 + 10,
			finalScore,
			GLUT_BITMAP_HELVETICA_18
			);

		iSetColor(100, 255, 180);

		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 - 30,
			"Press N for Level 2",
			GLUT_BITMAP_HELVETICA_18
			);

		iSetColor(200, 200, 200);

		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 - 70,
			"Press R to Play Again",
			GLUT_BITMAP_HELVETICA_18
			);
	}
}


// ======================================================
// ======================================================
// LEVEL 2 - POTIONS AND SPELLS CLASS
// ======================================================
// ======================================================


// ======================================================
// IMPORTANT LEVEL 2 GLOBAL VARIABLES
// ======================================================

int level2PlayerX = 650;
int level2PlayerY = 80;

int level2PlayerWidth = 117;
int level2PlayerHeight = 136;

int level2PlayerSpeed = 7;

// 1 = Right
// -1 = Left
int level2PlayerDirection = 1;


int level2Wands = 0;
int level2Books = 0;

int level2Time = 60;

bool level2Started = false;

// FIXED: declared before functions use them
bool level2Won = false;
bool level2Lost = false;
// level3Unlocked is declared earlier, near level1Unlocked / level2Unlocked

int level2TimerCounter = 0;


// ======================================================
// POWER UPS
// ======================================================

bool level2Shield = false;
bool level2Magnet = false;
bool level2Golden = false;
bool level2Slow = false;

int shieldTimer = 0;
int magnetTimer = 0;
int goldenTimer = 0;
int slowTimer = 0;


// ======================================================
// ITEM TYPES
// ======================================================

enum ItemType
{
	WAND,
	ENCHANTED_BOOK,
	MAGIC_CRYSTAL,
	GOLDEN_SPELLBOOK,

	POISON_POTION,
	DARK_ARTIFACT,
	CURSED_BOOK,

	SHIELD,
	MAGIC_MAGNET,
	TIME_CRYSTAL
};


// ======================================================
// FALLING ITEM
// ======================================================

struct FallingItem
{
	int x;
	int y;

	int width;
	int height;

	int speed;

	ItemType type;

	bool active;
};


#define MAX_LEVEL2_ITEMS 20

FallingItem level2Items[MAX_LEVEL2_ITEMS];


// ======================================================
// DIFFICULTY PHASE
// ======================================================

// 0 = Early
// 1 = Middle
// 2 = Chaos

int getLevel2Phase()
{
	if (level2Time > 45)
		return 0;

	if (level2Time > 15)
		return 1;

	return 2;
}


// ======================================================
// ACTIVE ITEM COUNT
// ======================================================

int getLevel2ActiveItemTarget()
{
	if (level2Time > 45)
	{

		return 4;
	}

	else if (level2Time > 15)
	{

		return 8;
	}

	else
	{

		return 14;
	}
}


// ======================================================
// ITEM SPEED
// ======================================================

int getLevel2ItemSpeed()
{
	int phase = getLevel2Phase();

	if (phase == 0)
	{
		// Early
		return 2 + rand() % 2;
		// 2 - 3
	}

	else if (phase == 1)
	{
		// Middle
		return 4 + rand() % 3;
		// 4 - 6
	}

	else
	{
		// MAGICAL CHAOS MODE
		return 8 + rand() % 4;
		// 8 - 11
	}
}


// ======================================================
// RANDOM ITEM
// ======================================================

ItemType getLevel2RandomItem()
{
	int r = rand() % 100;

	int phase = getLevel2Phase();


	// ==================================================
	// EARLY
	// Good items increases
	// ==================================================

	if (phase == 0)
	{
		if (r < 25)
			return WAND;

		if (r < 50)
			return ENCHANTED_BOOK;

		if (r < 56)
			return MAGIC_CRYSTAL;

		if (r < 60)
			return GOLDEN_SPELLBOOK;

		if (r < 82)
			return POISON_POTION;

		if (r < 88)
			return DARK_ARTIFACT;

		if (r < 93)
			return CURSED_BOOK;

		if (r < 96)
			return SHIELD;

		if (r < 98)
			return MAGIC_MAGNET;

		return TIME_CRYSTAL;
	}


	// ==================================================
	// MIDDLE
	// Harmful items increases
	// ==================================================

	else if (phase == 1)
	{
		if (r < 20)
			return WAND;

		if (r < 40)
			return ENCHANTED_BOOK;

		if (r < 45)
			return MAGIC_CRYSTAL;

		if (r < 49)
			return GOLDEN_SPELLBOOK;

		if (r < 65)
			return POISON_POTION;

		if (r < 75)
			return DARK_ARTIFACT;

		if (r < 84)
			return CURSED_BOOK;

		if (r < 90)
			return SHIELD;

		if (r < 96)
			return MAGIC_MAGNET;

		return TIME_CRYSTAL;
	}


	// ==================================================
	// MAGICAL CHAOS MODE
	// Harmful item hightest
	// ==================================================

	else
	{
		if (r < 16)
			return WAND;

		if (r < 32)
			return ENCHANTED_BOOK;

		if (r < 36)
			return MAGIC_CRYSTAL;

		if (r < 39)
			return GOLDEN_SPELLBOOK;

		// Harmful items = 45%
		if (r < 55)
			return POISON_POTION;

		if (r < 70)
			return DARK_ARTIFACT;

		if (r < 84)
			return CURSED_BOOK;

		if (r < 90)
			return SHIELD;

		if (r < 95)
			return MAGIC_MAGNET;

		return TIME_CRYSTAL;
	}
}


// ======================================================
// CREATE ITEM
// ======================================================

void createLevel2Item(int index)
{
	level2Items[index].x =
		30 + rand() % (SCREEN_WIDTH - 100);

	level2Items[index].y =
		SCREEN_HEIGHT + rand() % 350;

	level2Items[index].width = 55;
	level2Items[index].height = 55;

	level2Items[index].type =
		getLevel2RandomItem();

	level2Items[index].speed =
		getLevel2ItemSpeed();

	level2Items[index].active = true;
}


// ======================================================
// UPDATE NUMBER OF ITEMS
// ======================================================

void updateLevel2ItemCount()
{
	int target =
		getLevel2ActiveItemTarget();

	int activeCount = 0;


	for (int i = 0; i < MAX_LEVEL2_ITEMS; i++)
	{
		if (level2Items[i].active)
			activeCount++;
	}


	// new items create according to need
	if (activeCount < target)
	{
		for (int i = 0;
			i < MAX_LEVEL2_ITEMS && activeCount < target;
			i++)
		{
			if (!level2Items[i].active)
			{
				createLevel2Item(i);
				activeCount++;
			}
		}
	}
}


// ======================================================
// UPDATE SPEED WHEN PHASE CHANGES
// ======================================================

void updateLevel2ItemSpeeds()
{
	int currentPhase = getLevel2Phase();

	static int previousPhase = -1;

	if (currentPhase == previousPhase)
		return;

	previousPhase = currentPhase;


	for (int i = 0; i < MAX_LEVEL2_ITEMS; i++)
	{
		if (level2Items[i].active)
		{
			level2Items[i].speed =
				getLevel2ItemSpeed();
		}
	}
}


// ======================================================
// INITIALIZE LEVEL 2
// ======================================================

void initLevel2()
{
	srand((unsigned int)time(NULL));


	level2PlayerX = 650;
	level2PlayerY = 80;

	level2PlayerDirection = 1;


	level2Wands = 0;
	level2Books = 0;

	level2Time = 60;


	level2Started = true;

	// FIXED
	level2Won = false;
	level2Lost = false;
	level3Unlocked = false;


	level2TimerCounter = 0;


	level2Shield = false;
	level2Magnet = false;
	level2Golden = false;
	level2Slow = false;


	shieldTimer = 0;
	magnetTimer = 0;
	goldenTimer = 0;
	slowTimer = 0;


	// stop the win sound if it is still playing from a previous run
	mciSendString("stop winSound", NULL, 0, NULL);


	// all item inactive
	for (int i = 0; i < MAX_LEVEL2_ITEMS; i++)
	{
		level2Items[i].active = false;
	}


	// only 4 items at the start
	for (int i = 0; i < 4; i++)
	{
		createLevel2Item(i);

		// items start close to the top of the screen
		level2Items[i].y =
			SCREEN_HEIGHT + rand() % 250;
		playBGM();
	}
}


// ======================================================
// COLLISION
// ======================================================

bool level2Collision(FallingItem &item)
{
	if (!item.active)
		return false;


	if (
		level2PlayerX < item.x + item.width &&
		level2PlayerX + level2PlayerWidth > item.x &&
		level2PlayerY < item.y + item.height &&
		level2PlayerY + level2PlayerHeight > item.y
		)
	{
		return true;
	}


	return false;
}


// ======================================================
// COLLECT ITEM
// ======================================================

void collectLevel2Item(FallingItem &item)
{
	// Item caught sound
	playLevel2CatchSound();

	item.active = false;


	switch (item.type)
	{
	case WAND:

		if (level2Wands < 10)
			level2Wands++;

		break;


	case ENCHANTED_BOOK:

		if (level2Books < 10)
			level2Books++;

		break;


	case MAGIC_CRYSTAL:

		level2Time += 5;

		break;


	case GOLDEN_SPELLBOOK:

		level2Golden = true;
		goldenTimer = 400;

		level2Time += 3;

		break;


	case POISON_POTION:

		if (!level2Shield)
		{
			level2Time -= 5;

			if (level2Time < 0)
				level2Time = 0;
		}

		break;


	case DARK_ARTIFACT:

		if (!level2Shield)
		{
			level2Slow = true;
			slowTimer = 250;
		}

		break;


	case CURSED_BOOK:

		if (!level2Shield)
		{
			if (level2Books > 0)
				level2Books--;
		}

		break;


	case SHIELD:

		level2Shield = true;
		shieldTimer = 400;

		break;


	case MAGIC_MAGNET:

		level2Magnet = true;
		magnetTimer = 400;

		break;


	case TIME_CRYSTAL:

		level2Time += 10;

		break;
	}


	if (level2Time > 60)
		level2Time = 60;
}
// ======================================================
// UPDATE PLAYER
// ======================================================

void updateLevel2Player()
{
	int speed = level2PlayerSpeed;


	if (level2Slow)
		speed = 3;


	// LEFT
	if (isKeyPressed('a') || isKeyPressed('A'))
	{
		level2PlayerX -= speed;
		level2PlayerDirection = -1;
	}


	// RIGHT
	if (isKeyPressed('d') || isKeyPressed('D'))
	{
		level2PlayerX += speed;
		level2PlayerDirection = 1;
	}


	// Arrow LEFT
	if (isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		level2PlayerX -= speed;
		level2PlayerDirection = -1;
	}


	// Arrow RIGHT
	if (isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		level2PlayerX += speed;
		level2PlayerDirection = 1;
	}


	// Boundary
	if (level2PlayerX < 20)
		level2PlayerX = 20;


	if (
		level2PlayerX >
		SCREEN_WIDTH - level2PlayerWidth - 20
		)
	{
		level2PlayerX =
			SCREEN_WIDTH - level2PlayerWidth - 20;
	}
}


// ======================================================
// UPDATE ITEMS
// ======================================================

void updateLevel2Items()
{
	if (level2Won || level2Lost)
		return;


	for (int i = 0; i < MAX_LEVEL2_ITEMS; i++)
	{
		if (!level2Items[i].active)
			continue;


		level2Items[i].y -=
			level2Items[i].speed;


		// Collision
		if (level2Collision(level2Items[i]))
		{
			collectLevel2Item(level2Items[i]);
			continue;
		}


		// Bottom reached
		if (level2Items[i].y < 30)
		{
			level2Items[i].active = false;
		}
	}


	// create items again according to the target
	updateLevel2ItemCount();
}


// ======================================================
// MAGNET
// ======================================================

void updateLevel2Magnet()
{
	if (!level2Magnet)
		return;


	for (int i = 0; i < MAX_LEVEL2_ITEMS; i++)
	{
		if (!level2Items[i].active)
			continue;


		if (
			level2Items[i].type != WAND &&
			level2Items[i].type != ENCHANTED_BOOK &&
			level2Items[i].type != MAGIC_CRYSTAL &&
			level2Items[i].type != GOLDEN_SPELLBOOK
			)
		{
			continue;
		}


		int centerX =
			level2PlayerX +
			level2PlayerWidth / 2;


		if (level2Items[i].x < centerX - 5)
			level2Items[i].x += 3;

		else if (level2Items[i].x > centerX + 5)
			level2Items[i].x -= 3;
	}
}


// ======================================================
// POWER UP TIMER
// ======================================================

void updateLevel2PowerUps()
{
	if (shieldTimer > 0)
		shieldTimer--;
	else
		level2Shield = false;


	if (magnetTimer > 0)
		magnetTimer--;
	else
		level2Magnet = false;


	if (goldenTimer > 0)
		goldenTimer--;
	else
		level2Golden = false;


	if (slowTimer > 0)
		slowTimer--;
	else
		level2Slow = false;
}


// ======================================================
// TIMER
// ======================================================

void updateLevel2Timer()
{
	level2TimerCounter++;


	// fixedUpdate = 20ms
	// 50 frames = 1 second

	if (level2TimerCounter >= 50)
	{
		level2TimerCounter = 0;


		if (level2Time > 0)
			level2Time--;
	}
}


// ======================================================
// CHECK RESULT
// ======================================================

void checkLevel2Result()
{
	// WIN FIRST
	if (
		level2Wands >= 10 &&
		level2Books >= 10
		)
	{
		if (!level2Won)
		{
			level2Won = true;
			level3Unlocked = true;

			stopBGM();
			playWinSound();
		}
	}


	// LOSE
	if (level2Time <= 0)
	{
		level2Lost = true;

		playGameOverSound();

		return;
	}
}


// ======================================================
// MAIN LEVEL 2 UPDATE
// ======================================================

void updateLevel2()
{
	if (!level2Started)
		return;


	if (level2Won || level2Lost)
		return;


	updateLevel2Player();

	updateLevel2Items();

	updateLevel2Magnet();

	updateLevel2PowerUps();

	updateLevel2ItemSpeeds();

	updateLevel2Timer();

	checkLevel2Result();
}
// ======================================================
// LEVEL 2 BACKSTORY
// ======================================================
void drawLevel2Story()
{
	if (level2StoryPage == 1)
	{
		iShowBMP(0, 0, "level2_assets\\story1.bmp");
	}

	else if (level2StoryPage == 2)
	{
		iShowBMP(0, 0, "level2_assets\\story2.bmp");
	}

	else if (level2StoryPage == 3)
	{
		iShowBMP(0, 0, "level2_assets\\story3.bmp");
	}

	else if (level2StoryPage == 4)
	{
		iShowBMP(0, 0, "level2_assets\\story4.bmp");

		glDisable(GL_TEXTURE_2D);

		iSetColor(255, 255, 255);

		iTextCentered(
			SCREEN_WIDTH / 2,
			40,
			"PRESS SPACE TO START LEVEL 2",
			GLUT_BITMAP_TIMES_ROMAN_24
			);
	}
}

// ======================================================
// LEVEL 3 BACKSTORY
// ======================================================

void drawLevel3Story()
{
	iShowBMP(0, 0, "level3_assets\\story1.bmp");

	glDisable(GL_TEXTURE_2D);

	iSetColor(255, 255, 255);

	iTextCentered(
		SCREEN_WIDTH / 2,
		40,
		"PRESS SPACE TO START LEVEL 3",
		GLUT_BITMAP_TIMES_ROMAN_24
		);
}


// ======================================================
// DRAW LEVEL 2 ITEM
// ======================================================

void drawLevel2Item(FallingItem &item)
{
	if (!item.active)
		return;


	switch (item.type)
	{
	case WAND:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\wand.bmp"
			);

		break;


	case ENCHANTED_BOOK:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\book_stack.bmp"
			);

		break;


	case MAGIC_CRYSTAL:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\blue_crystal.bmp"
			);

		break;


	case GOLDEN_SPELLBOOK:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\gold_spellbook.bmp"
			);

		break;


	case POISON_POTION:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\poison_bottle.bmp"
			);

		break;


	case DARK_ARTIFACT:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\purple_orb.bmp"
			);

		break;


	case CURSED_BOOK:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\skull_book.bmp"
			);

		break;


	case SHIELD:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\shield.bmp"
			);

		break;


	case MAGIC_MAGNET:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\magnet.bmp"
			);

		break;


	case TIME_CRYSTAL:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\hourglass.bmp"
			);

		break;
	}
}


// ======================================================
// LEVEL 2 HUD
// ======================================================

void drawLevel2HUD()
{
	iSetColor(20, 10, 35);

	iFilledRectangle(
		0,
		SCREEN_HEIGHT - 80,
		SCREEN_WIDTH,
		80
		);


	char text[100];

	iSetColor(255, 255, 255);


	// WANDS
	sprintf_s(
		text,
		"Wands: %d / 10",
		level2Wands
		);

	iText(
		30,
		SCREEN_HEIGHT - 45,
		text,
		GLUT_BITMAP_HELVETICA_18
		);


	// BOOKS
	sprintf_s(
		text,
		"Books: %d / 10",
		level2Books
		);

	iText(
		220,
		SCREEN_HEIGHT - 45,
		text,
		GLUT_BITMAP_HELVETICA_18
		);


	// TIME
	if (level2Time <= 15)
		iSetColor(255, 50, 50);
	else
		iSetColor(255, 255, 255);


	sprintf_s(
		text,
		"Time: %d",
		level2Time
		);

	iText(
		410,
		SCREEN_HEIGHT - 45,
		text,
		GLUT_BITMAP_HELVETICA_18
		);


	// CHAOS MODE
	if (level2Time <= 15)
	{
		iSetColor(255, 80, 80);

		iText(
			550,
			SCREEN_HEIGHT - 45,
			"MAGICAL CHAOS MODE!",
			GLUT_BITMAP_HELVETICA_18
			);
	}


	// SHIELD
	if (level2Shield)
	{
		iSetColor(100, 200, 255);

		iText(
			950,
			SCREEN_HEIGHT - 35,
			"SHIELD",
			GLUT_BITMAP_HELVETICA_12
			);
	}


	// MAGNET
	if (level2Magnet)
	{
		iSetColor(255, 220, 50);

		iText(
			1030,
			SCREEN_HEIGHT - 35,
			"MAGNET",
			GLUT_BITMAP_HELVETICA_12
			);
	}


	// SLOW
	if (level2Slow)
	{
		iSetColor(200, 100, 255);

		iText(
			1120,
			SCREEN_HEIGHT - 35,
			"SLOW",
			GLUT_BITMAP_HELVETICA_12
			);
	}


	// CONTROLS HINT
	iSetColor(180, 180, 180);

	iText(
		30,
		SCREEN_HEIGHT - 65,
		"R = Retry Level 2    |    N = Unlock Level 3 (after win)    |    M = Level Select",
		GLUT_BITMAP_HELVETICA_12
		);
}


// ======================================================
// DRAW LEVEL 2
// ======================================================

void drawLevel2()
{
	// Background
	iShowBMP(
		0,
		0,
		"level2_assets\\level2_bg.bmp"
		);


	// Items
	for (int i = 0; i < MAX_LEVEL2_ITEMS; i++)
	{
		drawLevel2Item(level2Items[i]);
	}


	// ==================================================
	// PLAYER
	// ==================================================

	if (level2PlayerDirection == -1)
	{
		iShowBMP2(
			level2PlayerX,
			level2PlayerY,
			"level2_assets\\playerLeft.bmp",
			0
			);
	}
	else
	{
		iShowBMP2(
			level2PlayerX,
			level2PlayerY,
			"level2_assets\\playerRight.bmp",
			0
			);
	}


	// HUD
	glDisable(GL_TEXTURE_2D);

	drawLevel2HUD();


	// ==================================================
	// WIN (same box design as Level 1)
	// ==================================================

	if (level2Won)
	{
		char info[80];

		sprintf_s(
			info,
			"Wands: %d / 10     Books: %d / 10",
			level2Wands,
			level2Books
			);

		drawVictoryBox(
			"LEVEL 2 COMPLETE!",
			"You mastered the Potions and Spells Trial!",
			info,
			"LEVEL 3 UNLOCKED!  Press N to Continue",
			"Press R to Restart Level 2");
	}


	// ==================================================
	// LOSE
	// ==================================================

	if (level2Lost)
	{
		char lostInfo[80];

		sprintf_s(
			lostInfo,
			"Wands: %d / 10     Books: %d / 10",
			level2Wands,
			level2Books
			);

		drawGameOverBox(
			"TRIAL FAILED!",
			"The magical experiment overwhelmed you.",
			lostInfo,
			"Press R to Retry Level 2",
			"Press M for Level Select");
	}
}


// ======================================================
// I DRAW
// ======================================================

void iDraw()
{
	iClear();


	// ==================================================
	// MAIN MENU
	// ==================================================

	if (screen == 0)
	{
		iShowBMP(
			0,
			0,
			"assets\\menu_bg.bmp"
			);


		if (startHover)
			drawHoverButton(
			startX1,
			startY1,
			startX2,
			startY2,
			"START"
			);
		else
			drawButton(
			startX1,
			startY1,
			startX2,
			startY2,
			"START"
			);


		if (howHover)
			drawHoverButton(
			howX1,
			howY1,
			howX2,
			howY2,
			"HOW TO PLAY"
			);
		else
			drawButton(
			howX1,
			howY1,
			howX2,
			howY2,
			"HOW TO PLAY"
			);


		if (creditsHover)
			drawHoverButton(
			creditsX1,
			creditsY1,
			creditsX2,
			creditsY2,
			"CREDITS"
			);
		else
			drawButton(
			creditsX1,
			creditsY1,
			creditsX2,
			creditsY2,
			"CREDITS"
			);


		if (exitHover)
			drawHoverButton(
			exitX1,
			exitY1,
			exitX2,
			exitY2,
			"EXIT"
			);
		else
			drawButton(
			exitX1,
			exitY1,
			exitX2,
			exitY2,
			"EXIT"
			);


		if (highscoreHover)
			drawHoverButton(
			highscoreX1,
			highscoreY1,
			highscoreX2,
			highscoreY2,
			"HIGHSCORE"
			);
		else
			drawButton(
			highscoreX1,
			highscoreY1,
			highscoreX2,
			highscoreY2,
			"HIGHSCORE"
			);
	}


	// ==================================================
	// HOW TO PLAY
	// ==================================================

	else if (screen == 1)
	{
		iShowBMP(
			0,
			0,
			"assets\\how_bg.bmp"
			);


		if (backHover)
			drawHoverButton(
			15,
			30,
			180,
			90,
			"BACK"
			);
		else
			drawButton(
			15,
			30,
			180,
			90,
			"BACK"
			);
	}


	// ==================================================
	// CREDITS
	// ==================================================

	else if (screen == 2)
	{
		iShowBMP(
			0,
			0,
			"assets\\credits_bg.bmp"
			);


		if (backHover)
			drawHoverButton(
			15,
			30,
			180,
			90,
			"BACK"
			);
		else
			drawButton(
			15,
			30,
			180,
			90,
			"BACK"
			);
	}


	// ==================================================
	// STORY (LEVEL 1 BACKSTORY)
	// ==================================================

	else if (screen == 3)
	{
		drawStory();
	}


	// ==================================================
	// LEVEL 1
	// ==================================================

	else if (screen == 4)
	{
		drawLevel1();
	}


	// ==================================================
	// LEVEL 2
	// ==================================================

	else if (screen == 5)
	{
		drawLevel2();
	}


	// ==================================================
	// LEVEL 3
	// ==================================================

	else if (screen == 6)
	{
		drawLevel3();
	}

	// ==================================================
	// LEVEL 2 BACKSTORY
	// ==================================================

	else if (screen == 7)
	{
		drawLevel2Story();
	}

	// ==================================================
	// LEVEL SELECT
	// ==================================================

	else if (screen == 8)
	{
		drawLevelSelect();
	}

	// ==================================================
	// LEVEL 3 BACKSTORY
	// ==================================================

	else if (screen == 9)
	{
		drawLevel3Story();
	}

	// ==================================================
	// NAME INPUT (before Level 1)
	// ==================================================

	else if (screen == 10)
	{
		drawNameInput();
	}

	// ==================================================
	// HIGHSCORE BOARD
	// ==================================================

	else if (screen == 11)
	{
		drawHighscoreBoard();
	}
}


// ======================================================
// ENEMY MOVEMENT
// ======================================================

void moveEnemies()
{
	if (
		isGameOver ||
		isLevelCompleted ||
		screen != 4
		)
		return;


	for (int i = 0; i < NUM_ENEMIES; i++)
	{
		enemies[i].col +=
			enemies[i].dir;


		if (
			enemies[i].col >= enemies[i].maxCol ||
			enemies[i].col <= enemies[i].minCol
			)
		{
			enemies[i].dir *= -1;
		}


		if (
			enemies[i].row == playerRow &&
			enemies[i].col == playerCol
			)
		{
			lives--;


			if (lives <= 0)
			{
				isGameOver = true;

				stopBGM();

				playGameOverSound();
			}
			else
			{
				mciSendString(
					"play \"assets\\hit.mp3\" from 0",
					NULL,
					0,
					NULL
					);

				playerRow = START_ROW;
				playerCol = START_COL;
			}
		}
	}
}


// ======================================================
// LEVEL 1 PLAYER
// ======================================================

void updatePlayer()
{
	if (
		isGameOver ||
		isLevelCompleted ||
		screen != 4
		)
		return;


	static bool wPressed = false;
	static bool aPressed = false;
	static bool sPressed = false;
	static bool dPressed = false;


	int newRow = playerRow;
	int newCol = playerCol;

	bool moved = false;


	// UP
	if (
		(isKeyPressed('w') ||
		isSpecialKeyPressed(GLUT_KEY_UP))
		&&
		!wPressed
		)
	{
		newRow--;

		moved = true;

		wPressed = true;

		playerDirection = 1;
	}

	if (
		!(isKeyPressed('w') ||
		isSpecialKeyPressed(GLUT_KEY_UP))
		)
	{
		wPressed = false;
	}


	// DOWN
	if (
		(isKeyPressed('s') ||
		isSpecialKeyPressed(GLUT_KEY_DOWN))
		&&
		!sPressed
		)
	{
		newRow++;

		moved = true;

		sPressed = true;

		playerDirection = 0;
	}

	if (
		!(isKeyPressed('s') ||
		isSpecialKeyPressed(GLUT_KEY_DOWN))
		)
	{
		sPressed = false;
	}


	// LEFT
	if (
		(isKeyPressed('a') ||
		isSpecialKeyPressed(GLUT_KEY_LEFT))
		&&
		!aPressed
		)
	{
		newCol--;

		moved = true;

		aPressed = true;

		playerDirection = 2;
	}

	if (
		!(isKeyPressed('a') ||
		isSpecialKeyPressed(GLUT_KEY_LEFT))
		)
	{
		aPressed = false;
	}


	// RIGHT
	if (
		(isKeyPressed('d') ||
		isSpecialKeyPressed(GLUT_KEY_RIGHT))
		&&
		!dPressed
		)
	{
		newCol++;

		moved = true;

		dPressed = true;

		playerDirection = 3;
	}

	if (
		!(isKeyPressed('d') ||
		isSpecialKeyPressed(GLUT_KEY_RIGHT))
		)
	{
		dPressed = false;
	}


	if (moved)
	{
		if (
			newRow >= 0 &&
			newRow < ROWS &&
			newCol >= 0 &&
			newCol < COLS
			)
		{
			if (maze[newRow][newCol] != 1)
			{
				playerRow = newRow;
				playerCol = newCol;


				// Coin
				if (maze[playerRow][playerCol] == 2)
				{
					score += 10;

					maze[playerRow][playerCol] = 0;

					playCoinSound();     // coin.mp3 (mpeg)
				}


				// Book
				if (maze[playerRow][playerCol] == 3)
				{
					score += 50;

					maze[playerRow][playerCol] = 0;

					mciSendString(
						"play \"assets\\book.mp3\" from 0",
						NULL,
						0,
						NULL
						);
				}


				// Gate
				if (maze[playerRow][playerCol] == 4)
				{
					isLevelCompleted = true;

					// Level 1 cleared -> unlock Level 2 on the level select screen
					level2Unlocked = true;

					// Save the score to the top-5 highscore list, using
					// the name the player typed before starting Level 1
					if (playerNameLength == 0)
						addHighscore("Player", score);
					else
						addHighscore(playerNameInput, score);

					stopBGM();

					playWinSound();
				}
			}
		}
	}


	// Enemy collision
	for (int i = 0; i < NUM_ENEMIES; i++)
	{
		if (
			enemies[i].row == playerRow &&
			enemies[i].col == playerCol
			)
		{
			lives--;


			if (lives <= 0)
			{
				isGameOver = true;

				stopBGM();

				playGameOverSound();
			}
			else
			{
				mciSendString(
					"play \"assets\\hit.mp3\" from 0",
					NULL,
					0,
					NULL
					);

				playerRow = START_ROW;
				playerCol = START_COL;
			}
		}
	}
}

// ======================================================
// NAME INPUT - POLLED (screen == 10)
// iKeyboard callback is unreliable in this project (same
// reason movement/SPACE/R/M are polled elsewhere), so name
// typing, backspace and enter are polled here too instead
// of depending on the GLUT keyboard event.
// ======================================================

void pollNameInput()
{
	static bool letterPressed[26] = { false };
	static bool digitPressed[10] = { false };
	static bool spacePressedName = false;
	static bool backspacePressedName = false;
	static bool enterPressedName = false;

	if (screen != 10)
	{
		// Reset debounce state when leaving the screen so a key
		// still held down doesn't get "swallowed" next time we
		// come back to Name Input.
		for (int i = 0; i < 26; i++)
			letterPressed[i] = false;

		for (int i = 0; i < 10; i++)
			digitPressed[i] = false;

		spacePressedName = false;
		backspacePressedName = false;
		enterPressedName = false;

		return;
	}

	// Letters A-Z
	// 
	for (int i = 0; i < 26; i++)
	{
		int vk = 'A' + i;

		if (GetAsyncKeyState(vk) & 0x8000)
		{
			if (!letterPressed[i])
			{
				letterPressed[i] = true;

				if (playerNameLength < 19)
				{
					bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
					bool capsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

					// Caps Lock ON inverts what Shift alone would give,
					// same as a real keyboard.
					bool uppercase = shift != capsLockOn;

					char c = uppercase ? (char)('A' + i) : (char)('a' + i);

					playerNameInput[playerNameLength] = c;
					playerNameLength++;
					playerNameInput[playerNameLength] = '\0';
				}
			}
		}
		else
		{
			letterPressed[i] = false;
		}
	}

	// Digits 0-9
	for (int i = 0; i < 10; i++)
	{
		int vk = '0' + i;

		if (GetAsyncKeyState(vk) & 0x8000)
		{
			if (!digitPressed[i])
			{
				digitPressed[i] = true;

				if (playerNameLength < 19)
				{
					playerNameInput[playerNameLength] = (char)('0' + i);
					playerNameLength++;
					playerNameInput[playerNameLength] = '\0';
				}
			}
		}
		else
		{
			digitPressed[i] = false;
		}
	}

	// Space
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		if (!spacePressedName)
		{
			spacePressedName = true;

			if (playerNameLength < 19)
			{
				playerNameInput[playerNameLength] = ' ';
				playerNameLength++;
				playerNameInput[playerNameLength] = '\0';
			}
		}
	}
	else
	{
		spacePressedName = false;
	}

	// Backspace
	if (GetAsyncKeyState(VK_BACK) & 0x8000)
	{
		if (!backspacePressedName)
		{
			backspacePressedName = true;

			if (playerNameLength > 0)
			{
				playerNameLength--;
				playerNameInput[playerNameLength] = '\0';
			}
		}
	}
	else
	{
		backspacePressedName = false;
	}

	// Enter -> confirm name, go to Level 1 backstory
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		if (!enterPressedName)
		{
			enterPressedName = true;

			if (playerNameLength == 0)
			{
				strcpy_s(playerNameInput, "Player");
				playerNameLength = 6;
			}

			screen = 3;
			storyPage = 1;
			storyTimerStarted = false;
		}
	}
	else
	{
		enterPressedName = false;
	}
}

// ======================================================
// FIXED UPDATE
// ======================================================

void fixedUpdate()
{
	static bool spacePressed = false;
	static bool rPressed = false;
	static bool nPressed = false;

	static bool mPressed = false;

	static bool r2Pressed = false;
	static bool n2Pressed = false;
	static bool m2Pressed = false;

	static bool r3Pressed = false;
	static bool m3Pressed = false;

	// Name Input screen (screen == 10) is polled every tick,
	// same reliability reasoning as everything else below.
	pollNameInput();


	// ==================================================
	// STORY (LEVEL 1 BACKSTORY)
	// SPACE advances one page at a time (page 1 -> 2 -> 3 -> 4),
	// same pattern as the Level 2 backstory below; the last page
	// (4) starts the game instead of advancing further.
	// ==================================================

	if (screen == 3)
	{
		if (
			isKeyPressed(' ') &&
			!spacePressed
			)
		{
			spacePressed = true;

			if (storyPage < 4)
			{
				storyPage++;
			}
			else
			{
				screen = 4;

				resetGame();
			}
		}


		if (!isKeyPressed(' '))
			spacePressed = false;
	}
	// ==================================================
	// LEVEL 2 BACKSTORY
	// ==================================================

	if (screen == 7)
	{
		if (
			isKeyPressed(' ') &&
			!spacePressed
			)
		{
			spacePressed = true;

			// Story page 1 -> 2
			// Story page 2 -> 3
			// Story page 3 -> 4

			if (level2StoryPage < 4)
			{
				level2StoryPage++;
			}

			// Page 4 -> Level 2
			else
			{
				screen = 5;

				initLevel2();
			}
		}

		if (!isKeyPressed(' '))
		{
			spacePressed = false;
		}
	}

	// ==================================================
	// LEVEL 3 BACKSTORY
	// ==================================================

	if (screen == 9)
	{
		if (
			isKeyPressed(' ') &&
			!spacePressed
			)
		{
			spacePressed = true;

			screen = 6;
			initLevel3();
		}

		if (!isKeyPressed(' '))
		{
			spacePressed = false;
		}
	}

	// ==================================================
	// LEVEL 1
	// ==================================================

	if (screen == 4)
	{
		// Restart
		if (
			(isKeyPressed('r') ||
			isKeyPressed('R'))
			&&
			!rPressed
			)
		{
			resetGame();

			rPressed = true;
		}

		if (
			!(isKeyPressed('r') ||
			isKeyPressed('R'))
			)
		{
			rPressed = false;
		}

		// Level 2
		if (
			isLevelCompleted &&
			(isKeyPressed('n') ||
			isKeyPressed('N'))
			&&
			!nPressed
			)
		{
			nPressed = true;

			screen = 7;
			level2StoryPage = 1;
		}

		if (
			!(isKeyPressed('n') ||
			isKeyPressed('N'))
			)
		{
			nPressed = false;
		}

		// M = Back to Level Select
		if (
			(isKeyPressed('m') || isKeyPressed('M'))
			&&
			!mPressed
			)
		{
			mPressed = true;

			screen = 8;
		}

		if (!(isKeyPressed('m') || isKeyPressed('M')))
		{
			mPressed = false;
		}

		updatePlayer();

		// Camera
		float targetCameraX =
			(playerCol * cellSize) -
			(SCREEN_WIDTH / 3.0f);

		if (targetCameraX < 0)
			targetCameraX = 0;

		if (
			targetCameraX >
			(COLS * cellSize) - SCREEN_WIDTH
			)
		{
			targetCameraX =
				(COLS * cellSize) -
				SCREEN_WIDTH;
		}

		cameraX +=
			(targetCameraX - cameraX) * 0.2f;
	}

	// ==================================================
	// LEVEL 2
	// ==================================================

	else if (screen == 5)
	{
		// R = Restart Level 2 (works any time, win/lose/mid-game)
		if (
			(isKeyPressed('r') || isKeyPressed('R'))
			&&
			!r2Pressed
			)
		{
			r2Pressed = true;

			initLevel2();
		}

		if (!(isKeyPressed('r') || isKeyPressed('R')))
		{
			r2Pressed = false;
		}


		// N = Go to Level 3 (only after winning)
		if (
			(isKeyPressed('n') || isKeyPressed('N'))
			&&
			!n2Pressed
			)
		{
			n2Pressed = true;

			if (level2Won)
			{
				screen = 9;
			}
		}

		if (!(isKeyPressed('n') || isKeyPressed('N')))
		{
			n2Pressed = false;
		}


		// M = Back to Level Select
		if (
			(isKeyPressed('m') || isKeyPressed('M'))
			&&
			!m2Pressed
			)
		{
			m2Pressed = true;

			screen = 8;
		}

		if (!(isKeyPressed('m') || isKeyPressed('M')))
		{
			m2Pressed = false;
		}


		updateLevel2();
	}

	// ==================================================
	// LEVEL 3 - FINAL TRIAL
	// ==================================================

	else if (screen == 6)
	{
		updateLevel3();
	}
}


// ======================================================
// MOUSE MOVE
// ======================================================

void iPassiveMouseMove(int mx, int my)
{
	startHover = false;
	howHover = false;
	creditsHover = false;
	exitHover = false;
	backHover = false;
	highscoreHover = false;

	lvl1Hover = false;
	lvl2Hover = false;
	lvl3Hover = false;


	if (screen == 0)
	{
		if (
			mx >= startX1 &&
			mx <= startX2 &&
			my >= startY1 &&
			my <= startY2
			)
		{
			startHover = true;
		}

		else if (
			mx >= howX1 &&
			mx <= howX2 &&
			my >= howY1 &&
			my <= howY2
			)
		{
			howHover = true;
		}

		else if (
			mx >= creditsX1 &&
			mx <= creditsX2 &&
			my >= creditsY1 &&
			my <= creditsY2
			)
		{
			creditsHover = true;
		}

		else if (
			mx >= exitX1 &&
			mx <= exitX2 &&
			my >= exitY1 &&
			my <= exitY2
			)
		{
			exitHover = true;
		}

		else if (
			mx >= highscoreX1 &&
			mx <= highscoreX2 &&
			my >= highscoreY1 &&
			my <= highscoreY2
			)
		{
			highscoreHover = true;
		}
	}


	else if (screen == 1 || screen == 2 || screen == 11)
	{
		if (
			mx >= 15 &&
			mx <= 180 &&
			my >= 30 &&
			my <= 90
			)
		{
			backHover = true;
		}
	}


	else if (screen == 8)
	{
		if (
			mx >= lvl1X1 &&
			mx <= lvl1X2 &&
			my >= lvl1Y1 &&
			my <= lvl1Y2
			)
		{
			lvl1Hover = true;
		}

		else if (
			level2Unlocked &&
			mx >= lvl2X1 &&
			mx <= lvl2X2 &&
			my >= lvl2Y1 &&
			my <= lvl2Y2
			)
		{
			lvl2Hover = true;
		}

		else if (
			level3Unlocked &&
			mx >= lvl3X1 &&
			mx <= lvl3X2 &&
			my >= lvl3Y1 &&
			my <= lvl3Y2
			)
		{
			lvl3Hover = true;
		}
	}
}


void iMouseMove(int mx, int my)
{
}


// ======================================================
// MOUSE CLICK
// ======================================================

void iMouse(int button, int state, int mx, int my)
{
	if (state != GLUT_DOWN)
		return;


	if (button == GLUT_LEFT_BUTTON)
	{
		// Main menu
		if (screen == 0)
		{
			if (
				mx >= startX1 &&
				mx <= startX2 &&
				my >= startY1 &&
				my <= startY2
				)
			{
				// START now goes to Level Select instead of straight to the story
				screen = 8;
			}


			else if (
				mx >= howX1 &&
				mx <= howX2 &&
				my >= howY1 &&
				my <= howY2
				)
			{
				screen = 1;
			}


			else if (
				mx >= creditsX1 &&
				mx <= creditsX2 &&
				my >= creditsY1 &&
				my <= creditsY2
				)
			{
				screen = 2;
			}


			else if (
				mx >= exitX1 &&
				mx <= exitX2 &&
				my >= exitY1 &&
				my <= exitY2
				)
			{
				exit(0);
			}

			else if (
				mx >= highscoreX1 &&
				mx <= highscoreX2 &&
				my >= highscoreY1 &&
				my <= highscoreY2
				)
			{
				screen = 11;
			}
		}


		// Back
		else if (screen == 1 || screen == 2 || screen == 11)
		{
			if (
				mx >= 15 &&
				mx <= 180 &&
				my >= 30 &&
				my <= 90
				)
			{
				screen = 0;
			}
		}


		// Story - page advance is handled by SPACE in fixedUpdate() now.
		else if (screen == 3)
		{
		}


		// Level Select
		else if (screen == 8)
		{
			// Level 1 - always unlocked
			if (
				mx >= lvl1X1 &&
				mx <= lvl1X2 &&
				my >= lvl1Y1 &&
				my <= lvl1Y2
				)
			{
				// Ask for the player's name first, then the
				// backstory, then Level 1 itself.
				screen = 10;
				playerNameInput[0] = '\0';
				playerNameLength = 0;
			}


			// Level 2 - only if unlocked
			else if (
				level2Unlocked &&
				mx >= lvl2X1 &&
				mx <= lvl2X2 &&
				my >= lvl2Y1 &&
				my <= lvl2Y2
				)
			{
				screen = 7;
				level2StoryPage = 1;
			}


			// Level 3 - only if unlocked
			else if (
				level3Unlocked &&
				mx >= lvl3X1 &&
				mx <= lvl3X2 &&
				my >= lvl3Y1 &&
				my <= lvl3Y2
				)
			{
				screen = 9;
			}
		}
	}


	// Right click = previous story
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (
			screen == 3 &&
			storyPage > 1
			)
		{
			storyPage--;
		}
	}
}


// ======================================================
// KEYBOARD
// ======================================================

void iKeyboard(unsigned char key)
{
	// Level 1
	if (screen == 4)
	{
		if (
			(key == 'n' || key == 'N') &&
			isLevelCompleted
			)
		{
			screen = 7;

			level2StoryPage = 1;
		}

		// Quick return to Level Select
		if (key == 'm' || key == 'M')
		{
			screen = 8;
		}
	}


	// Level 2
	else if (screen == 5)
	{
		level2Keyboard(key);

		// Quick return to Level Select
		if (key == 'm' || key == 'M')
		{
			screen = 8;
		}
	}


	// Level 3
	else if (screen == 6)
	{
		level3Keyboard(key);

		// Quick return to Level Select
		if (key == 'm' || key == 'M')
		{
			screen = 8;
		}
	}


	// Level Select
	else if (screen == 8)
	{
		// Optional: press ESC/back key to Main Menu if you want,
		// currently no key is bound here.
	}


	// Name Input (before Level 1)
	// NOTE: Typing/ENTER/BACKSPACE for this screen are now handled
	// by pollNameInput() inside fixedUpdate() (GetAsyncKeyState
	// polling), the same way movement/SPACE/R/M are handled
	// elsewhere in this project, because this iKeyboard callback
	// is not reliable here. This block is kept only as a
	// harmless fallback in case the callback does fire.
	else if (screen == 10)
	{
		// ENTER (13) -> save name, go to Level 1 backstory
		if (key == 13)
		{
			if (playerNameLength == 0)
			{
				strcpy_s(playerNameInput, "Player");
				playerNameLength = 6;
			}

			screen = 3;
			storyPage = 1;
			storyTimerStarted = false;
		}

		// BACKSPACE (8) -> erase last character
		else if (key == 8)
		{
			if (playerNameLength > 0)
			{
				playerNameLength--;
				playerNameInput[playerNameLength] = '\0';
			}
		}

		// Printable letters/digits only, up to 19 characters
		else if (
			playerNameLength < 19 &&
			key >= 32 && key <= 126
			)
		{
			playerNameInput[playerNameLength] = key;
			playerNameLength++;
			playerNameInput[playerNameLength] = '\0';
		}
	}


	// Highscore Board
	else if (screen == 11)
	{
		if (key == 'm' || key == 'M')
		{
			screen = 0;
		}
	}
}


void iSpecialKeyboard(unsigned char key)
{
	if (screen == 6)
	{
		level3SpecialKeyboard((int)key);
	}
}



void level2Keyboard(unsigned char key)
{
	// R = Restart Level 2
	if (key == 'r' || key == 'R')
	{
		initLevel2();
		return;
	}

	// N = Go to Level 3 Backstory first
	if ((key == 'n' || key == 'N') && level2Won)
	{
		screen = 9;
		return;
	}
}
// ======================================================
// MAIN
// ======================================================

int main()
{
	// Save original maze
	for (int r = 0; r < ROWS; r++)
	{
		for (int c = 0; c < COLS; c++)
		{
			initialMaze[r][c] =
				maze[r][c];
		}
	}


	resetGame();

	loadHighscores();


	// Enemy timer
	iSetTimer(
		600,
		moveEnemies
		);


	// Main update timer      
	iSetTimer(
		20,
		fixedUpdate
		);


	iInitialize(
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		"Echoes of Arcana"
		);


	iStart();


	return 0;
}