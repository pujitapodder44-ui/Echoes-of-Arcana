
#include "pujita_level3.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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
	}
}

// =====================================================
// INITIALIZE LEVEL 3
// =====================================================

void initLevel3()
{
	int i;

	level3PlayerX = 620;
	level3PlayerY = 100;
	level3PlayerDirection = 1;

	level3PlayerHP = 100;
	level3Mana = 100;

	level3Phase = 1;
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
	level3TransitionTimer = 70;

	for (i = 0; i < L3_MAX_SMALL_ENEMIES; i++)
	{
		smallEnemies[i].active = true;
		smallEnemies[i].hp = 40;
		smallEnemies[i].attackCooldown = 0;
		smallEnemies[i].shootCooldown = 50 + i * 15;
	}
}

void l3StartPhase3()
{
	level3Phase = 3;
	level3TransitionTimer = 90;
	level3GuardianActive = false;
	level3GuardianHP = 100;
	level3GuardianPunchCooldown = 30;
	level3GuardianMagicCooldown = 50;
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
				l3SpawnEnemySpell(smallEnemies[i].x + 35,
					smallEnemies[i].y + 45,
					10, 9, false);
			}

			smallEnemies[i].shootCooldown = 90 + rand() % 50;
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

	// Game Over
	if (level3GameOver)
	{
		iSetColor(0, 0, 0);
		iFilledRectangle(0, 0, L3_SCREEN_WIDTH, L3_SCREEN_HEIGHT);

		iSetColor(255, 60, 60);
		iText(555, 430, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

		iSetColor(255, 255, 255);
		iText(500, 380, "The Final Trial defeated you.", GLUT_BITMAP_HELVETICA_18);
		iText(525, 330, "Press R to Retry", GLUT_BITMAP_HELVETICA_18);
		iText(525, 290, "Press M for Level Select", GLUT_BITMAP_HELVETICA_18);
	}

	// Victory
	if (level3Won)
	{
		iSetColor(0, 0, 0);
		iFilledRectangle(0, 0, L3_SCREEN_WIDTH, L3_SCREEN_HEIGHT);

		iSetColor(255, 215, 0);
		iText(505, 440, "LEVEL 3 COMPLETE!", GLUT_BITMAP_TIMES_ROMAN_24);

		iSetColor(255, 255, 255);
		iText(485, 390, "The Arcane Guardian has fallen.", GLUT_BITMAP_HELVETICA_18);
		iText(500, 340, "You completed the Final Trial!", GLUT_BITMAP_HELVETICA_18);
		iText(525, 290, "Press R to Play Again", GLUT_BITMAP_HELVETICA_18);
		iText(525, 250, "Press M for Level Select", GLUT_BITMAP_HELVETICA_18);
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
			initLevel3();
		return;
	}

	if (level3Won)
	{
		if (key == 'r' || key == 'R')
			initLevel3();
		return;
	}

	if (key == 'w' || key == 'W')
	{
		level3PlayerDirection = 1;
		l3MovePlayer(0, level3PlayerSpeed);
	}
	else if (key == 's' || key == 'S')
	{
		level3PlayerDirection = 0;
		l3MovePlayer(0, -level3PlayerSpeed);
	}
	else if (key == 'a' || key == 'A')
	{
		level3PlayerDirection = 2;
		l3MovePlayer(-level3PlayerSpeed, 0);
	}
	else if (key == 'd' || key == 'D')
	{
		level3PlayerDirection = 3;
		l3MovePlayer(level3PlayerSpeed, 0);
	}
	else if (key == ' ')
	{
		l3SpawnPlayerSpell();
	}
	else if (key == 'r' || key == 'R')
	{
		initLevel3();
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
