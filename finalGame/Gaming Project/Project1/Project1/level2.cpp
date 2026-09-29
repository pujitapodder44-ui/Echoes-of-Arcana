#include "level2.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ======================================================
// LEVEL 2 - POTIONS AND SPELLS CLASS
// ======================================================

#define SCREEN_WIDTH 1365
#define SCREEN_HEIGHT 768

// ======================================================
// PLAYER
// ======================================================

int level2PlayerX = 650;
int level2PlayerY = 80;

int level2PlayerWidth = 70;
int level2PlayerHeight = 80;

int level2PlayerSpeed = 7;

// ======================================================
// SCORE
// ======================================================

int level2Wands = 0;
int level2Books = 0;

// ======================================================
// TIMER
// ======================================================

int level2Time = 60;

bool level2Started = false;
bool level2Won = false;
bool level2Lost = false;

// Timer frame counter
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
// RANDOM ITEM
// ======================================================

ItemType getLevel2RandomItem()
{
	int r = rand() % 100;

	if (r < 20)
		return WAND;

	if (r < 40)
		return ENCHANTED_BOOK;

	if (r < 46)
		return MAGIC_CRYSTAL;

	if (r < 50)
		return GOLDEN_SPELLBOOK;

	if (r < 64)
		return POISON_POTION;

	if (r < 76)
		return DARK_ARTIFACT;

	if (r < 86)
		return CURSED_BOOK;

	if (r < 91)
		return SHIELD;

	if (r < 96)
		return MAGIC_MAGNET;

	return TIME_CRYSTAL;
}

// ======================================================
// CREATE ITEM
// ======================================================

void createLevel2Item(int index)
{
	level2Items[index].x =
		30 + rand() % (SCREEN_WIDTH - 100);

	level2Items[index].y =
		SCREEN_HEIGHT + rand() % 500;

	level2Items[index].width = 55;
	level2Items[index].height = 55;

	level2Items[index].type =
		getLevel2RandomItem();

	level2Items[index].active = true;

	// Difficulty
	if (level2Time > 45)
	{
		level2Items[index].speed =
			3 + rand() % 2;
	}
	else if (level2Time > 15)
	{
		level2Items[index].speed =
			5 + rand() % 3;
	}
	else
	{
		// Magical Chaos Mode
		level2Items[index].speed =
			8 + rand() % 5;
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

	level2Wands = 0;
	level2Books = 0;

	level2Time = 60;

	level2Started = true;
	level2Won = false;
	level2Lost = false;

	level2TimerCounter = 0;

	level2Shield = false;
	level2Magnet = false;
	level2Golden = false;
	level2Slow = false;

	shieldTimer = 0;
	magnetTimer = 0;
	goldenTimer = 0;
	slowTimer = 0;

	for (int i = 0; i < MAX_LEVEL2_ITEMS; i++)
	{
		createLevel2Item(i);

		level2Items[i].y =
			SCREEN_HEIGHT +
			rand() % 900;
	}
}

// ======================================================
// COLLISION
// ======================================================

bool level2Collision(FallingItem &item)
{
	if (!item.active)
		return false;

	if (level2PlayerX <
		item.x + item.width &&
		level2PlayerX +
		level2PlayerWidth > item.x &&
		level2PlayerY <
		item.y + item.height &&
		level2PlayerY +
		level2PlayerHeight > item.y)
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
	item.active = false;

	switch (item.type)
	{
		// ==================================================
		// GOOD ITEMS
		// ==================================================

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


		// ==================================================
		// HARMFUL
		// ==================================================

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


		// ==================================================
		// POWER UPS
		// ==================================================

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

	// Maximum time
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

	// A
	if (isKeyPressed('a') ||
		isKeyPressed('A'))
	{
		level2PlayerX -= speed;
	}

	// D
	if (isKeyPressed('d') ||
		isKeyPressed('D'))
	{
		level2PlayerX += speed;
	}

	// LEFT
	if (isSpecialKeyPressed(GLUT_KEY_LEFT))
	{
		level2PlayerX -= speed;
	}

	// RIGHT
	if (isSpecialKeyPressed(GLUT_KEY_RIGHT))
	{
		level2PlayerX += speed;
	}

	// Boundary
	if (level2PlayerX < 20)
		level2PlayerX = 20;

	if (level2PlayerX >
		SCREEN_WIDTH -
		level2PlayerWidth -
		20)
	{
		level2PlayerX =
			SCREEN_WIDTH -
			level2PlayerWidth -
			20;
	}
}

// ======================================================
// UPDATE ITEMS
// ======================================================

void updateLevel2Items()
{
	if (level2Won || level2Lost)
		return;

	for (int i = 0;
		i < MAX_LEVEL2_ITEMS;
		i++)
	{
		if (!level2Items[i].active)
			continue;

		level2Items[i].y -=
			level2Items[i].speed;

		// Collision
		if (level2Collision(level2Items[i]))
		{
			collectLevel2Item(
				level2Items[i]);

			continue;
		}

		// Reached bottom
		if (level2Items[i].y < 30)
		{
			createLevel2Item(i);
		}
	}
}

// ======================================================
// MAGNET
// ======================================================

void updateLevel2Magnet()
{
	if (!level2Magnet)
		return;

	for (int i = 0;
		i < MAX_LEVEL2_ITEMS;
		i++)
	{
		if (!level2Items[i].active)
			continue;

		if (level2Items[i].type != WAND &&
			level2Items[i].type != ENCHANTED_BOOK &&
			level2Items[i].type != MAGIC_CRYSTAL &&
			level2Items[i].type != GOLDEN_SPELLBOOK)
		{
			continue;
		}

		int centerX =
			level2PlayerX +
			level2PlayerWidth / 2;

		if (level2Items[i].x < centerX - 5)
			level2Items[i].x += 3;

		else if (level2Items[i].x >
			centerX + 5)
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
// GAME TIMER
// ======================================================

void updateLevel2Timer()
{
	level2TimerCounter++;

	// fixedUpdate = 20 ms
	// 50 times = 1 second

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
	// WIN
	if (level2Wands >= 10 &&
		level2Books >= 10)
	{
		level2Won = true;
		return;
	}

	// LOSE
	if (level2Time <= 0)
	{
		level2Lost = true;
		return;
	}
}

// ======================================================
// MAIN UPDATE
// ======================================================

void updateLevel2()
{
	if (!level2Started)
		return;

	if (level2Won ||
		level2Lost)
		return;

	updateLevel2Player();

	updateLevel2Items();

	updateLevel2Magnet();

	updateLevel2PowerUps();

	updateLevel2Timer();

	checkLevel2Result();
}

// ======================================================
// DRAW ITEM
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
			"level2_assets\\wand.bmp");

		break;


	case ENCHANTED_BOOK:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\enchanted_book.bmp");

		break;


	case MAGIC_CRYSTAL:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\magic_crystal.bmp");

		break;


	case GOLDEN_SPELLBOOK:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\golden_spellbook.bmp");

		break;


	case POISON_POTION:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\poison_potion.bmp");

		break;


	case DARK_ARTIFACT:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\dark_artifact.bmp");

		break;


	case CURSED_BOOK:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\cursed_book.bmp");

		break;


	case SHIELD:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\shield.bmp");

		break;


	case MAGIC_MAGNET:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\magic_magnet.bmp");

		break;


	case TIME_CRYSTAL:

		iShowBMP(
			item.x,
			item.y,
			"level2_assets\\time_crystal.bmp");

		break;
	}
}

// ======================================================
// DRAW HUD
// ======================================================

void drawLevel2HUD()
{
	// HUD
	iSetColor(20, 10, 35);

	iFilledRectangle(
		0,
		SCREEN_HEIGHT - 80,
		SCREEN_WIDTH,
		80);

	char text[100];

	iSetColor(255, 255, 255);

	// Wand
	sprintf_s(
		text,
		"Wands: %d / 10",
		level2Wands);

	iText(
		30,
		SCREEN_HEIGHT - 45,
		text,
		GLUT_BITMAP_HELVETICA_18);


	// Book
	sprintf_s(
		text,
		"Books: %d / 10",
		level2Books);

	iText(
		220,
		SCREEN_HEIGHT - 45,
		text,
		GLUT_BITMAP_HELVETICA_18);


	// Time
	if (level2Time <= 15)
		iSetColor(255, 50, 50);
	else
		iSetColor(255, 255, 255);

	sprintf_s(
		text,
		"Time: %d",
		level2Time);

	iText(
		410,
		SCREEN_HEIGHT - 45,
		text,
		GLUT_BITMAP_HELVETICA_18);


	// Chaos Mode
	if (level2Time <= 15)
	{
		iSetColor(255, 80, 80);

		iText(
			550,
			SCREEN_HEIGHT - 45,
			"MAGICAL CHAOS MODE!",
			GLUT_BITMAP_HELVETICA_18);
	}


	// Shield
	if (level2Shield)
	{
		iSetColor(100, 200, 255);

		iText(
			950,
			SCREEN_HEIGHT - 35,
			"SHIELD",
			GLUT_BITMAP_HELVETICA_12);
	}


	// Magnet
	if (level2Magnet)
	{
		iSetColor(255, 220, 50);

		iText(
			1030,
			SCREEN_HEIGHT - 35,
			"MAGNET",
			GLUT_BITMAP_HELVETICA_12);
	}


	// Slow
	if (level2Slow)
	{
		iSetColor(200, 100, 255);

		iText(
			1120,
			SCREEN_HEIGHT - 35,
			"SLOW",
			GLUT_BITMAP_HELVETICA_12);
	}
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
		"level2_assets\\level2_bg.bmp");


	// Falling items
	for (int i = 0;
		i < MAX_LEVEL2_ITEMS;
		i++)
	{
		drawLevel2Item(
			level2Items[i]);
	}


	// Player
	iShowBMP(
		level2PlayerX,
		level2PlayerY,
		"level2_assets\\level2_player.bmp");


	// HUD
	drawLevel2HUD();


	// ==================================================
	// WIN SCREEN
	// ==================================================

	if (level2Won)
	{
		iSetColor(0, 0, 0);

		iFilledRectangle(
			250,
			180,
			865,
			350);


		iSetColor(255, 215, 0);

		iText(
			510,
			430,
			"LEVEL 2 COMPLETE!",
			GLUT_BITMAP_TIMES_ROMAN_24);


		iSetColor(255, 255, 255);

		iText(
			470,
			370,
			"You mastered the Potions and Spells Trial!",
			GLUT_BITMAP_HELVETICA_18);


		iText(
			500,
			320,
			"Wands: 10 / 10",
			GLUT_BITMAP_HELVETICA_18);


		iText(
			700,
			320,
			"Books: 10 / 10",
			GLUT_BITMAP_HELVETICA_18);


		iSetColor(150, 255, 150);

		iText(
			500,
			250,
			"Press N for Level 3",
			GLUT_BITMAP_HELVETICA_18);
	}


	// ==================================================
	// LOSE SCREEN
	// ==================================================

	if (level2Lost)
	{
		iSetColor(0, 0, 0);

		iFilledRectangle(
			250,
			180,
			865,
			350);


		iSetColor(255, 70, 70);

		iText(
			520,
			430,
			"TRIAL FAILED!",
			GLUT_BITMAP_TIMES_ROMAN_24);


		iSetColor(255, 255, 255);

		iText(
			470,
			370,
			"The magical experiment overwhelmed you.",
			GLUT_BITMAP_HELVETICA_18);


		char text[100];


		sprintf_s(
			text,
			"Wands: %d / 10",
			level2Wands);

		iText(
			500,
			320,
			text,
			GLUT_BITMAP_HELVETICA_18);


		sprintf_s(
			text,
			"Books: %d / 10",
			level2Books);

		iText(
			500,
			280,
			text,
			GLUT_BITMAP_HELVETICA_18);


		iSetColor(255, 200, 100);

		iText(
			520,
			230,
			"Press R to Retry",
			GLUT_BITMAP_HELVETICA_18);
	}
}

// ======================================================
// KEYBOARD
// ======================================================

void level2Keyboard(unsigned char key)
{
	// Retry
	if (level2Lost &&
		(key == 'r' ||
		key == 'R'))
	{
		initLevel2();
		return;
	}

	// Level 3
	// এখনো Level 3 তৈরি হয়নি,
	// তাই আপাতত N কিছু করবে না।

	if (level2Won &&
		(key == 'n' ||
		key == 'N'))
	{
		return;
	}
}