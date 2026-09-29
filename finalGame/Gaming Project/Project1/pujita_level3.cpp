#include "pujita_level3.h"
#include "iGraphics.h"
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma comment(lib, "winmm.lib")


// ======================================================
// RESULT FLAGS (defined here, declared extern in the header)
// ======================================================

bool level3GameWon = false;
bool level3GameLost = false;


// ======================================================
// PHASES
// ======================================================

enum L3Phase
{
	L3_PHASE_SPIDERS,
	L3_PHASE_TRANSITION_1,
	L3_PHASE_SMALL_ENEMIES,
	L3_PHASE_TRANSITION_2,
	L3_PHASE_BOSS
};

static int L3_Phase = L3_PHASE_SPIDERS;
static int L3_TransitionTimer = 0;


// ======================================================
// PLAYER
// ======================================================

static float L3_PlayerX, L3_PlayerY;
static float L3_PlayerSpeed = 4.5f;

// 0 = Down, 1 = Up, 2 = Left, 3 = Right
static int L3_PlayerDir = 1;

static int L3_PlayerHP;
static int L3_PlayerMaxHP = 100;

static int L3_PlayerMana;
static int L3_PlayerMaxMana = 100;

static int L3_ManaRegenTimer = 0;
static int L3_SpellCooldown = 0;
static int L3_InvulnTimer = 0;

static bool L3_Dashing = false;
static int L3_DashTimer = 0;
static int L3_DashCooldown = 0;


// ======================================================
// SPIDERS (Phase 1)
// ======================================================

struct L3Spider
{
	float x, y;
	int hp;
	bool active;
};

#define L3_MAX_SPIDERS 4
static L3Spider L3_Spiders[L3_MAX_SPIDERS];


// ======================================================
// SMALL ENEMIES (Phase 2)
// ======================================================

struct L3SmallEnemy
{
	float x, y;
	int hp;
	bool active;
	int shootTimer;
};

#define L3_MAX_SMALL 4
static L3SmallEnemy L3_SmallEnemies[L3_MAX_SMALL];


// ======================================================
// ARCANE GUARDIAN (Boss, Phase 3)
// ======================================================

static int L3_GuardianHP;
static int L3_GuardianMaxHP = 100;

static float L3_GuardianX, L3_GuardianY;
static bool L3_GuardianActive = false;

static int L3_GuardianAttackTimer = 0;
static int L3_GuardianSpellTimer = 0;
static int L3_GuardianHitFlash = 0;


// ======================================================
// PLAYER SPELLS (projectiles fired by SPACE)
// ======================================================

struct L3Spell
{
	float x, y;
	float dx, dy;
	bool active;
};

#define L3_MAX_SPELLS 24
static L3Spell L3_Spells[L3_MAX_SPELLS];


// ======================================================
// ENEMY SPELLS (small-enemy ranged shots + guardian spells)
// ======================================================

struct L3EnemySpell
{
	float x, y;
	float dx, dy;
	bool active;
	int damage;
};

#define L3_MAX_ENEMY_SPELLS 24
static L3EnemySpell L3_EnemySpells[L3_MAX_ENEMY_SPELLS];


// ======================================================
// FORWARD DECLARATIONS (internal helpers)
// ======================================================

static void spawnSpiders();
static void updateSpiders();

static void spawnSmallEnemies();
static void updateSmallEnemies();

static void spawnGuardian();
static void updateGuardian();


// ======================================================
// INIT
// ======================================================

void initLevel3Game()
{
	srand((unsigned int)time(NULL));

	L3_PlayerX = SCREEN_WIDTH / 2.0f - 35;
	L3_PlayerY = 150;
	L3_PlayerDir = 1;

	L3_PlayerHP = L3_PlayerMaxHP;
	L3_PlayerMana = L3_PlayerMaxMana;

	L3_ManaRegenTimer = 0;
	L3_SpellCooldown = 0;
	L3_InvulnTimer = 0;

	L3_Dashing = false;
	L3_DashTimer = 0;
	L3_DashCooldown = 0;


	for (int i = 0; i < L3_MAX_SPELLS; i++)
		L3_Spells[i].active = false;

	for (int i = 0; i < L3_MAX_ENEMY_SPELLS; i++)
		L3_EnemySpells[i].active = false;


	L3_Phase = L3_PHASE_SPIDERS;
	L3_TransitionTimer = 0;

	spawnSpiders();

	for (int i = 0; i < L3_MAX_SMALL; i++)
		L3_SmallEnemies[i].active = false;

	L3_GuardianActive = false;
	L3_GuardianHP = L3_GuardianMaxHP;
	L3_GuardianHitFlash = 0;


	level3GameWon = false;
	level3GameLost = false;
}


// ======================================================
// SPIDER WAVE
// ======================================================

static void spawnSpiders()
{
	int startPositions[L3_MAX_SPIDERS][2] =
	{
		{ 130, 600 },
		{ SCREEN_WIDTH - 180, 600 },
		{ 130, 220 },
		{ SCREEN_WIDTH - 180, 220 }
	};

	for (int i = 0; i < L3_MAX_SPIDERS; i++)
	{
		L3_Spiders[i].x = (float)startPositions[i][0];
		L3_Spiders[i].y = (float)startPositions[i][1];
		L3_Spiders[i].hp = 30;
		L3_Spiders[i].active = true;
	}
}


static void updateSpiders()
{
	bool anyAlive = false;

	for (int i = 0; i < L3_MAX_SPIDERS; i++)
	{
		if (!L3_Spiders[i].active)
			continue;

		anyAlive = true;

		float dx = L3_PlayerX - L3_Spiders[i].x;
		float dy = L3_PlayerY - L3_Spiders[i].y;
		float d = sqrtf(dx * dx + dy * dy);

		if (d > 1.0f)
		{
			dx /= d;
			dy /= d;

			L3_Spiders[i].x += dx * 1.3f;
			L3_Spiders[i].y += dy * 1.3f;
		}

		// Contact damage
		if (L3_InvulnTimer <= 0 && d < 45.0f)
		{
			L3_PlayerHP -= 8;
			L3_InvulnTimer = 30;

			if (L3_PlayerHP <= 0)
			{
				L3_PlayerHP = 0;
				level3GameLost = true;
			}
		}
	}

	if (!anyAlive)
	{
		L3_Phase = L3_PHASE_TRANSITION_1;
		L3_TransitionTimer = 70;
	}
}


// ======================================================
// SMALL ENEMY WAVE
// ======================================================

static void spawnSmallEnemies()
{
	int startPositions[L3_MAX_SMALL][2] =
	{
		{ 150, 550 },
		{ SCREEN_WIDTH - 220, 550 },
		{ 150, 260 },
		{ SCREEN_WIDTH - 220, 260 }
	};

	for (int i = 0; i < L3_MAX_SMALL; i++)
	{
		L3_SmallEnemies[i].x = (float)startPositions[i][0];
		L3_SmallEnemies[i].y = (float)startPositions[i][1];
		L3_SmallEnemies[i].hp = 40;
		L3_SmallEnemies[i].active = true;
		L3_SmallEnemies[i].shootTimer = 60 + i * 20;
	}
}


static void updateSmallEnemies()
{
	bool anyAlive = false;

	for (int i = 0; i < L3_MAX_SMALL; i++)
	{
		if (!L3_SmallEnemies[i].active)
			continue;

		anyAlive = true;

		float dx = L3_PlayerX - L3_SmallEnemies[i].x;
		float dy = L3_PlayerY - L3_SmallEnemies[i].y;
		float d = sqrtf(dx * dx + dy * dy);

		float ndx = dx, ndy = dy;

		if (d > 1.0f)
		{
			ndx /= d;
			ndy /= d;

			L3_SmallEnemies[i].x += ndx * 2.0f;
			L3_SmallEnemies[i].y += ndy * 2.0f;
		}

		// Melee contact damage
		if (L3_InvulnTimer <= 0 && d < 45.0f)
		{
			L3_PlayerHP -= 10;
			L3_InvulnTimer = 30;

			if (L3_PlayerHP <= 0)
			{
				L3_PlayerHP = 0;
				level3GameLost = true;
			}
		}

		// Ranged attack — only the odd-indexed enemies shoot,
		// so the wave isn't overwhelming
		if (i % 2 == 1)
		{
			L3_SmallEnemies[i].shootTimer--;

			if (L3_SmallEnemies[i].shootTimer <= 0 && d < 650.0f)
			{
				L3_SmallEnemies[i].shootTimer = 110;

				for (int j = 0; j < L3_MAX_ENEMY_SPELLS; j++)
				{
					if (!L3_EnemySpells[j].active)
					{
						L3_EnemySpells[j].active = true;
						L3_EnemySpells[j].x = L3_SmallEnemies[i].x;
						L3_EnemySpells[j].y = L3_SmallEnemies[i].y;

						float sp = 6.5f;
						L3_EnemySpells[j].dx = ndx * sp;
						L3_EnemySpells[j].dy = ndy * sp;
						L3_EnemySpells[j].damage = 8;

						break;
					}
				}
			}
		}
	}

	if (!anyAlive)
	{
		L3_Phase = L3_PHASE_TRANSITION_2;
		L3_TransitionTimer = 90;
	}
}


// ======================================================
// ARCANE GUARDIAN (BOSS)
// ======================================================

static void spawnGuardian()
{
	L3_GuardianHP = L3_GuardianMaxHP;
	L3_GuardianX = SCREEN_WIDTH / 2.0f - 60;
	L3_GuardianY = SCREEN_HEIGHT - 260;

	L3_GuardianActive = true;
	L3_GuardianAttackTimer = 70;
	L3_GuardianSpellTimer = 100;
	L3_GuardianHitFlash = 0;
}


static void updateGuardian()
{
	if (!L3_GuardianActive)
		return;


	// ---- Determine phase from current HP ----
	int phase;

	if (L3_GuardianHP > 60)
		phase = 1;
	else if (L3_GuardianHP >= 30)
		phase = 2;
	else
		phase = 3;

	float speed =
		(phase == 1) ? 1.4f :
		(phase == 2) ? 2.2f : 3.0f;

	int punchCooldownMax =
		(phase == 1) ? 70 :
		(phase == 2) ? 50 : 35;

	int spellCooldownMax =
		(phase == 1) ? 100 :
		(phase == 2) ? 65 : 35;

	int numSpellsAtOnce = (phase == 3) ? 3 : 1;


	// ---- Move toward player ----
	float dx = L3_PlayerX - L3_GuardianX;
	float dy = L3_PlayerY - L3_GuardianY;
	float d = sqrtf(dx * dx + dy * dy);

	float ndx = dx, ndy = dy;

	if (d > 1.0f)
	{
		ndx /= d;
		ndy /= d;
	}

	if (d > 95.0f)
	{
		L3_GuardianX += ndx * speed;
		L3_GuardianY += ndy * speed;
	}


	// ---- Punch attack ----
	L3_GuardianAttackTimer--;

	if (L3_GuardianAttackTimer <= 0)
	{
		if (d < 95.0f && L3_InvulnTimer <= 0)
		{
			L3_PlayerHP -= 15;
			L3_InvulnTimer = 30;

			if (L3_PlayerHP <= 0)
			{
				L3_PlayerHP = 0;
				level3GameLost = true;
			}
		}

		L3_GuardianAttackTimer = punchCooldownMax;
	}


	// ---- Spell attack ----
	L3_GuardianSpellTimer--;

	if (L3_GuardianSpellTimer <= 0)
	{
		L3_GuardianSpellTimer = spellCooldownMax;

		float baseAngle = atan2f(ndy, ndx);

		for (int k = 0; k < numSpellsAtOnce; k++)
		{
			float angleOffset =
				(k - (numSpellsAtOnce - 1) / 2.0f) * 0.35f;

			float a = baseAngle + angleOffset;
			float sp = 6.5f;

			for (int j = 0; j < L3_MAX_ENEMY_SPELLS; j++)
			{
				if (!L3_EnemySpells[j].active)
				{
					L3_EnemySpells[j].active = true;
					L3_EnemySpells[j].x = L3_GuardianX + 40;
					L3_EnemySpells[j].y = L3_GuardianY + 50;
					L3_EnemySpells[j].dx = cosf(a) * sp;
					L3_EnemySpells[j].dy = sinf(a) * sp;
					L3_EnemySpells[j].damage = 10;

					break;
				}
			}
		}
	}


	if (L3_GuardianHitFlash > 0)
		L3_GuardianHitFlash--;
}


// ======================================================
// MAIN UPDATE
// ======================================================

void updateLevel3Game()
{
	if (level3GameWon || level3GameLost)
		return;


	// ---- Movement input ----
	float moveX = 0, moveY = 0;

	if (isKeyPressed('w') || isKeyPressed('W') || isSpecialKeyPressed(GLUT_KEY_UP))
	{
		moveY += 1;
		L3_PlayerDir = 1;
	}

	if (isKeyPressed('s') || isKeyPressed('S') || isSpecialKeyPressed(GLUT_KEY_DOWN))
	{
		moveY -= 1;
		L3_PlayerDir = 0;
	}

	if (isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		moveX -= 1;
		L3_PlayerDir = 2;
	}

	if (isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		moveX += 1;
		L3_PlayerDir = 3;
	}

	float len = sqrtf(moveX * moveX + moveY * moveY);

	if (len > 0.0f)
	{
		moveX /= len;
		moveY /= len;
	}

	float speed = L3_PlayerSpeed;

	if (L3_Dashing)
		speed = L3_PlayerSpeed * 3.2f;

	L3_PlayerX += moveX * speed;
	L3_PlayerY += moveY * speed;

	// Arena boundary
	if (L3_PlayerX < 60)
		L3_PlayerX = 60;

	if (L3_PlayerX > SCREEN_WIDTH - 130)
		L3_PlayerX = SCREEN_WIDTH - 130;

	if (L3_PlayerY < 110)
		L3_PlayerY = 110;

	if (L3_PlayerY > SCREEN_HEIGHT - 150)
		L3_PlayerY = SCREEN_HEIGHT - 150;


	// ---- Dash (F key) ----
	static bool fPressed = false;

	if (
		(isKeyPressed('f') || isKeyPressed('F'))
		&&
		!fPressed
		&&
		L3_DashCooldown <= 0
		)
	{
		fPressed = true;

		L3_Dashing = true;
		L3_DashTimer = 8;
		L3_InvulnTimer = 14;
		L3_DashCooldown = 60;
	}

	if (!(isKeyPressed('f') || isKeyPressed('F')))
		fPressed = false;

	if (L3_DashTimer > 0)
	{
		L3_DashTimer--;

		if (L3_DashTimer <= 0)
			L3_Dashing = false;
	}

	if (L3_DashCooldown > 0)
		L3_DashCooldown--;

	if (L3_InvulnTimer > 0)
		L3_InvulnTimer--;


	// ---- Mana regen ----
	L3_ManaRegenTimer++;

	if (L3_ManaRegenTimer >= 15)
	{
		L3_ManaRegenTimer = 0;

		if (L3_PlayerMana < L3_PlayerMaxMana)
			L3_PlayerMana++;
	}

	if (L3_SpellCooldown > 0)
		L3_SpellCooldown--;


	// ---- Cast spell (SPACE) ----
	if (
		isKeyPressed(' ')
		&&
		L3_SpellCooldown <= 0
		&&
		L3_PlayerMana >= 10
		)
	{
		L3_SpellCooldown = 14;
		L3_PlayerMana -= 10;

		for (int i = 0; i < L3_MAX_SPELLS; i++)
		{
			if (!L3_Spells[i].active)
			{
				L3_Spells[i].active = true;
				L3_Spells[i].x = L3_PlayerX + 30;
				L3_Spells[i].y = L3_PlayerY + 40;

				float sp = 12.0f;

				if (L3_PlayerDir == 0)
				{
					L3_Spells[i].dx = 0;
					L3_Spells[i].dy = -sp;
				}
				else if (L3_PlayerDir == 1)
				{
					L3_Spells[i].dx = 0;
					L3_Spells[i].dy = sp;
				}
				else if (L3_PlayerDir == 2)
				{
					L3_Spells[i].dx = -sp;
					L3_Spells[i].dy = 0;
				}
				else
				{
					L3_Spells[i].dx = sp;
					L3_Spells[i].dy = 0;
				}

				break;
			}
		}
	}


	// ---- Update player spells + collisions ----
	for (int i = 0; i < L3_MAX_SPELLS; i++)
	{
		if (!L3_Spells[i].active)
			continue;

		L3_Spells[i].x += L3_Spells[i].dx;
		L3_Spells[i].y += L3_Spells[i].dy;

		if (
			L3_Spells[i].x < 0 || L3_Spells[i].x > SCREEN_WIDTH ||
			L3_Spells[i].y < 0 || L3_Spells[i].y > SCREEN_HEIGHT
			)
		{
			L3_Spells[i].active = false;
			continue;
		}

		if (L3_Phase == L3_PHASE_SPIDERS)
		{
			for (int s = 0; s < L3_MAX_SPIDERS; s++)
			{
				if (!L3_Spiders[s].active)
					continue;

				if (
					fabs(L3_Spells[i].x - L3_Spiders[s].x) < 35 &&
					fabs(L3_Spells[i].y - L3_Spiders[s].y) < 35
					)
				{
					L3_Spiders[s].hp -= 10;
					L3_Spells[i].active = false;

					if (L3_Spiders[s].hp <= 0)
						L3_Spiders[s].active = false;

					break;
				}
			}
		}
		else if (L3_Phase == L3_PHASE_SMALL_ENEMIES)
		{
			for (int e = 0; e < L3_MAX_SMALL; e++)
			{
				if (!L3_SmallEnemies[e].active)
					continue;

				if (
					fabs(L3_Spells[i].x - L3_SmallEnemies[e].x) < 35 &&
					fabs(L3_Spells[i].y - L3_SmallEnemies[e].y) < 35
					)
				{
					L3_SmallEnemies[e].hp -= 10;
					L3_Spells[i].active = false;

					if (L3_SmallEnemies[e].hp <= 0)
						L3_SmallEnemies[e].active = false;

					break;
				}
			}
		}
		else if (L3_Phase == L3_PHASE_BOSS && L3_GuardianActive)
		{
			if (
				fabs(L3_Spells[i].x - L3_GuardianX) < 45 &&
				fabs(L3_Spells[i].y - L3_GuardianY) < 55
				)
			{
				L3_GuardianHP -= 10;
				L3_GuardianHitFlash = 6;
				L3_Spells[i].active = false;

				if (L3_GuardianHP <= 0)
				{
					L3_GuardianHP = 0;
					L3_GuardianActive = false;
					level3GameWon = true;
				}
			}
		}
	}


	// ---- Phase-specific updates ----
	if (L3_Phase == L3_PHASE_SPIDERS)
	{
		updateSpiders();
	}
	else if (L3_Phase == L3_PHASE_TRANSITION_1)
	{
		L3_TransitionTimer--;

		if (L3_TransitionTimer <= 0)
		{
			L3_Phase = L3_PHASE_SMALL_ENEMIES;
			spawnSmallEnemies();
		}
	}
	else if (L3_Phase == L3_PHASE_SMALL_ENEMIES)
	{
		updateSmallEnemies();
	}
	else if (L3_Phase == L3_PHASE_TRANSITION_2)
	{
		L3_TransitionTimer--;

		if (L3_TransitionTimer <= 0)
		{
			L3_Phase = L3_PHASE_BOSS;
			spawnGuardian();
		}
	}
	else if (L3_Phase == L3_PHASE_BOSS)
	{
		updateGuardian();
	}


	// ---- Update enemy spells (small enemy + guardian projectiles) ----
	for (int i = 0; i < L3_MAX_ENEMY_SPELLS; i++)
	{
		if (!L3_EnemySpells[i].active)
			continue;

		L3_EnemySpells[i].x += L3_EnemySpells[i].dx;
		L3_EnemySpells[i].y += L3_EnemySpells[i].dy;

		if (
			L3_EnemySpells[i].x < 0 || L3_EnemySpells[i].x > SCREEN_WIDTH ||
			L3_EnemySpells[i].y < 0 || L3_EnemySpells[i].y > SCREEN_HEIGHT
			)
		{
			L3_EnemySpells[i].active = false;
			continue;
		}

		if (
			L3_InvulnTimer <= 0 &&
			fabs(L3_EnemySpells[i].x - (L3_PlayerX + 35)) < 32 &&
			fabs(L3_EnemySpells[i].y - (L3_PlayerY + 45)) < 40
			)
		{
			L3_PlayerHP -= L3_EnemySpells[i].damage;
			L3_InvulnTimer = 25;
			L3_EnemySpells[i].active = false;

			if (L3_PlayerHP <= 0)
			{
				L3_PlayerHP = 0;
				level3GameLost = true;
			}
		}
	}
}


// ======================================================
// DRAW
// ======================================================

void drawLevel3Game()
{
	iShowBMP(0, 0, "level3_assets\\arena.bmp");


	// ---- Player spells ----
	for (int i = 0; i < L3_MAX_SPELLS; i++)
	{
		if (L3_Spells[i].active)
		{
			iShowBMP(
				(int)L3_Spells[i].x,
				(int)L3_Spells[i].y,
				"level3_assets\\spell.bmp"
				);
		}
	}


	// ---- Enemy spells ----
	for (int i = 0; i < L3_MAX_ENEMY_SPELLS; i++)
	{
		if (L3_EnemySpells[i].active)
		{
			iShowBMP(
				(int)L3_EnemySpells[i].x,
				(int)L3_EnemySpells[i].y,
				"level3_assets\\guardian_spell.bmp"
				);
		}
	}


	// ---- Spiders ----
	if (L3_Phase == L3_PHASE_SPIDERS)
	{
		for (int i = 0; i < L3_MAX_SPIDERS; i++)
		{
			if (L3_Spiders[i].active)
			{
				iShowBMP2(
					(int)L3_Spiders[i].x - 30,
					(int)L3_Spiders[i].y - 30,
					"level3_assets\\spider.bmp",
					0
					);
			}
		}
	}


	// ---- Small enemies ----
	if (L3_Phase == L3_PHASE_SMALL_ENEMIES)
	{
		for (int i = 0; i < L3_MAX_SMALL; i++)
		{
			if (L3_SmallEnemies[i].active)
			{
				iShowBMP2(
					(int)L3_SmallEnemies[i].x - 30,
					(int)L3_SmallEnemies[i].y - 30,
					"level3_assets\\small_enemy.bmp",
					0
					);
			}
		}
	}


	// ---- Guardian ----
	if (L3_Phase == L3_PHASE_BOSS && L3_GuardianActive)
	{
		iShowBMP2(
			(int)L3_GuardianX,
			(int)L3_GuardianY,
			"level3_assets\\guardian.bmp",
			0
			);
	}


	// ---- Player ----
	if (L3_PlayerDir == 1)
	{
		iShowBMP2(
			(int)L3_PlayerX,
			(int)L3_PlayerY,
			"level3_assets\\player_up.bmp",
			0
			);
	}
	else if (L3_PlayerDir == 2)
	{
		iShowBMP2(
			(int)L3_PlayerX,
			(int)L3_PlayerY,
			"level3_assets\\player_left.bmp",
			0
			);
	}
	else if (L3_PlayerDir == 3)
	{
		iShowBMP2(
			(int)L3_PlayerX,
			(int)L3_PlayerY,
			"level3_assets\\player_right.bmp",
			0
			);
	}
	else
	{
		iShowBMP2(
			(int)L3_PlayerX,
			(int)L3_PlayerY,
			"level3_assets\\player_down.bmp",
			0
			);
	}


	// ==================================================
	// HUD
	// ==================================================

	glDisable(GL_TEXTURE_2D);


	// Player HP bar
	iSetColor(60, 10, 10);
	iFilledRectangle(20, SCREEN_HEIGHT - 40, 300, 22);

	iSetColor(255, 60, 60);

	int hpW = (int)(296 * (L3_PlayerHP / (float)L3_PlayerMaxHP));
	if (hpW < 0) hpW = 0;

	iFilledRectangle(22, SCREEN_HEIGHT - 38, hpW, 18);

	iSetColor(255, 255, 255);

	char hpText[40];
	sprintf_s(hpText, "HP: %d/%d", L3_PlayerHP, L3_PlayerMaxHP);

	iText(25, SCREEN_HEIGHT - 45, hpText, GLUT_BITMAP_HELVETICA_12);


	// Mana bar
	iSetColor(10, 10, 60);
	iFilledRectangle(340, SCREEN_HEIGHT - 40, 260, 22);

	iSetColor(80, 140, 255);

	int manaW = (int)(256 * (L3_PlayerMana / (float)L3_PlayerMaxMana));
	if (manaW < 0) manaW = 0;

	iFilledRectangle(342, SCREEN_HEIGHT - 38, manaW, 18);

	iSetColor(255, 255, 255);

	char manaText[40];
	sprintf_s(manaText, "MANA: %d/%d", L3_PlayerMana, L3_PlayerMaxMana);

	iText(345, SCREEN_HEIGHT - 45, manaText, GLUT_BITMAP_HELVETICA_12);


	// Phase info / Boss bar
	if (L3_Phase == L3_PHASE_SPIDERS)
	{
		int remaining = 0;

		for (int i = 0; i < L3_MAX_SPIDERS; i++)
		if (L3_Spiders[i].active)
			remaining++;

		char t[60];
		sprintf_s(t, "Spiders remaining: %d", remaining);

		iSetColor(255, 255, 255);
		iTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, t, GLUT_BITMAP_HELVETICA_18);
	}
	else if (L3_Phase == L3_PHASE_TRANSITION_1)
	{
		iSetColor(255, 215, 0);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2,
			"More enemies incoming...",
			GLUT_BITMAP_TIMES_ROMAN_24
			);
	}
	else if (L3_Phase == L3_PHASE_SMALL_ENEMIES)
	{
		int remaining = 0;

		for (int i = 0; i < L3_MAX_SMALL; i++)
		if (L3_SmallEnemies[i].active)
			remaining++;

		char t[60];
		sprintf_s(t, "Enemies remaining: %d", remaining);

		iSetColor(255, 255, 255);
		iTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, t, GLUT_BITMAP_HELVETICA_18);
	}
	else if (L3_Phase == L3_PHASE_TRANSITION_2)
	{
		iSetColor(255, 60, 60);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2,
			"THE ARCANE GUARDIAN AWAKENS...",
			GLUT_BITMAP_TIMES_ROMAN_24
			);
	}
	else if (L3_Phase == L3_PHASE_BOSS && L3_GuardianActive)
	{
		iSetColor(30, 10, 10);
		iFilledRectangle(SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT - 30, 500, 20);

		if (L3_GuardianHitFlash > 0)
			iSetColor(255, 255, 255);
		else
			iSetColor(200, 30, 30);

		int gW = (int)(496 * (L3_GuardianHP / (float)L3_GuardianMaxHP));
		if (gW < 0) gW = 0;

		iFilledRectangle(SCREEN_WIDTH / 2 - 248, SCREEN_HEIGHT - 28, gW, 16);

		iSetColor(255, 255, 255);

		char t[60];
		sprintf_s(t, "ARCANE GUARDIAN: %d/%d", L3_GuardianHP, L3_GuardianMaxHP);

		iTextCentered(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 45, t, GLUT_BITMAP_HELVETICA_18);
	}


	// Controls hint
	iSetColor(180, 180, 180);
	iText(
		20,
		15,
		"WASD/Arrows = Move  |  SPACE = Cast Spell  |  F = Dash  |  R = Retry  |  M = Level Select",
		GLUT_BITMAP_HELVETICA_12
		);


	// ==================================================
	// WIN OVERLAY
	// ==================================================

	if (level3GameWon)
	{
		iSetColor(0, 0, 0);
		iFilledRectangle(SCREEN_WIDTH / 2 - 260, SCREEN_HEIGHT / 2 - 90, 520, 180);

		iSetColor(255, 215, 0);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 + 40,
			"ARCANE GUARDIAN DEFEATED!",
			GLUT_BITMAP_TIMES_ROMAN_24
			);

		iSetColor(255, 255, 255);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2,
			"You have completed Echoes of Arcana!",
			GLUT_BITMAP_HELVETICA_18
			);

		iSetColor(200, 200, 200);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 - 40,
			"Press M for Level Select",
			GLUT_BITMAP_HELVETICA_18
			);
	}


	// ==================================================
	// LOSE OVERLAY
	// ==================================================

	if (level3GameLost)
	{
		iSetColor(0, 0, 0);
		iFilledRectangle(SCREEN_WIDTH / 2 - 260, SCREEN_HEIGHT / 2 - 90, 520, 180);

		iSetColor(255, 60, 60);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 + 40,
			"YOU HAVE FALLEN...",
			GLUT_BITMAP_TIMES_ROMAN_24
			);

		iSetColor(255, 255, 255);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2,
			"The Arcane Guardian was too strong.",
			GLUT_BITMAP_HELVETICA_18
			);

		iSetColor(255, 200, 100);
		iTextCentered(
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2 - 40,
			"Press R to Retry",
			GLUT_BITMAP_HELVETICA_18
			);
	}
}