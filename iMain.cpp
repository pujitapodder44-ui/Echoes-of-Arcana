#include "iGraphics.h"
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

// ======================================================
// ECHOES OF ARCANA
// Main Menu + How To Play + Credits + Level 1
// ======================================================

#define SCREEN_WIDTH 1365
#define SCREEN_HEIGHT 768

// ------------------------------------------------------
// Screen states
// ------------------------------------------------------
int screen = 0;

// 0 = Main Menu
// 1 = How To Play
// 2 = Credits
// 3 = Level 1


// ------------------------------------------------------
// Mouse hover variables
// ------------------------------------------------------
bool startHover = false;
bool howHover = false;
bool creditsHover = false;
bool exitHover = false;
bool backHover = false;


// ------------------------------------------------------
// Button coordinates
// IMPORTANT:
// iGraphics Y coordinate starts from BOTTOM.
// ------------------------------------------------------

int startX1 = 520;
int startX2 = 845;
int startY1 = 440;
int startY2 = 510;

int howX1 = 480;
int howX2 = 885;
int howY1 = 350;
int howY2 = 420;

int creditsX1 = 500;
int creditsX2 = 865;
int creditsY1 = 260;
int creditsY2 = 330;

int exitX1 = 535;
int exitX2 = 830;
int exitY1 = 170;
int exitY2 = 240;


// ------------------------------------------------------
// Draw button
// ------------------------------------------------------

void drawButton(int x1, int y1, int x2, int y2, char text[])
{
	// Transparent-looking dark button
	iSetColor(20, 10, 35);
	iFilledRectangle(x1, y1, x2 - x1, y2 - y1);

	// Border
	iSetColor(180, 120, 255);
	iRectangle(x1, y1, x2 - x1, y2 - y1);

	// Text
	iSetColor(255, 255, 255);

	int textX = (x1 + x2) / 2 - 45;

	iText(textX, y1 + 23, text, GLUT_BITMAP_TIMES_ROMAN_24);
}


// ------------------------------------------------------
// Draw hovered button
// ------------------------------------------------------

void drawHoverButton(int x1, int y1, int x2, int y2, char text[])
{
	// Glow-like outer rectangle
	iSetColor(220, 170, 255);
	iRectangle(x1 - 3, y1 - 3, x2 - x1 + 6, y2 - y1 + 6);

	// Button
	iSetColor(65, 30, 100);
	iFilledRectangle(x1, y1, x2 - x1, y2 - y1);

	// Border
	iSetColor(255, 220, 255);
	iRectangle(x1, y1, x2 - x1, y2 - y1);

	// Text
	iSetColor(255, 255, 255);

	int textX = (x1 + x2) / 2 - 45;

	iText(textX, y1 + 23, text, GLUT_BITMAP_TIMES_ROMAN_24);
}


// ======================================================
// iDraw
// ======================================================

void iDraw()
{
	iClear();

	// ==================================================
	// MAIN MENU
	// ==================================================

	if (screen == 0)
	{
		// 1365 x 768 background
		iShowBMP(0, 0, "assets\\menu_bg.bmp");


		// START
		if (startHover)
			drawHoverButton(startX1, startY1, startX2, startY2, "START");
		else
			drawButton(startX1, startY1, startX2, startY2, "START");


		// HOW TO PLAY
		if (howHover)
			drawHoverButton(howX1, howY1, howX2, howY2, "HOW TO PLAY");
		else
			drawButton(howX1, howY1, howX2, howY2, "HOW TO PLAY");


		// CREDITS
		if (creditsHover)
			drawHoverButton(creditsX1, creditsY1, creditsX2, creditsY2, "CREDITS");
		else
			drawButton(creditsX1, creditsY1, creditsX2, creditsY2, "CREDITS");


		// EXIT
		if (exitHover)
			drawHoverButton(exitX1, exitY1, exitX2, exitY2, "EXIT");
		else
			drawButton(exitX1, exitY1, exitX2, exitY2, "EXIT");
	}


	// ==================================================
	// HOW TO PLAY
	// ==================================================

	else if (screen == 1)
	{
		iShowBMP(0, 0, "assets\\how_bg.bmp");

		// Back button
		if (backHover)
			drawHoverButton(35, 30, 180, 90, "BACK");
		else
			drawButton(35, 30, 180, 90, "BACK");
	}


	// ==================================================
	// CREDITS
	// ==================================================

	else if (screen == 2)
	{
		iShowBMP(0, 0, "assets\\credits_bg.bmp");

		// Back button
		if (backHover)
			drawHoverButton(35, 30, 180, 90, "BACK");
		else
			drawButton(35, 30, 180, 90, "BACK");
	}


	// ==================================================
	// LEVEL 1
	// ==================================================

	else if (screen == 3)
	{
		iSetColor(10, 10, 20);

		iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		iSetColor(255, 255, 255);

		iText(600, 400, "LEVEL 1 STARTED!", GLUT_BITMAP_TIMES_ROMAN_24);
	}
}


// ======================================================
// Mouse Move
// ======================================================

void iMouseMove(int mx, int my)
{
}


// ======================================================
// Passive Mouse Move
// ======================================================

void iPassiveMouseMove(int mx, int my)
{
	// Reset hover
	startHover = false;
	howHover = false;
	creditsHover = false;
	exitHover = false;
	backHover = false;


	// ==============================================
	// MAIN MENU
	// ==============================================

	if (screen == 0)
	{
		if (mx >= startX1 && mx <= startX2 &&
			my >= startY1 && my <= startY2)
		{
			startHover = true;
		}

		else if (mx >= howX1 && mx <= howX2 &&
			my >= howY1 && my <= howY2)
		{
			howHover = true;
		}

		else if (mx >= creditsX1 && mx <= creditsX2 &&
			my >= creditsY1 && my <= creditsY2)
		{
			creditsHover = true;
		}

		else if (mx >= exitX1 && mx <= exitX2 &&
			my >= exitY1 && my <= exitY2)
		{
			exitHover = true;
		}
	}


	// ==============================================
	// HOW TO PLAY / CREDITS
	// ==============================================

	else if (screen == 1 || screen == 2)
	{
		if (mx >= 35 && mx <= 180 &&
			my >= 30 && my <= 90)
		{
			backHover = true;
		}
	}
}


// ======================================================
// Mouse Click
// ======================================================

void iMouse(int button, int state, int mx, int my)
{
	if (button == GLUT_LEFT_BUTTON &&
		state == GLUT_DOWN)
	{

		// ==================================================
		// MAIN MENU
		// ==================================================

		if (screen == 0)
		{
			// START
			if (mx >= startX1 && mx <= startX2 &&
				my >= startY1 && my <= startY2)
			{
				screen = 3;
			}

			// HOW TO PLAY
			else if (mx >= howX1 && mx <= howX2 &&
				my >= howY1 && my <= howY2)
			{
				screen = 1;
			}

			// CREDITS
			else if (mx >= creditsX1 && mx <= creditsX2 &&
				my >= creditsY1 && my <= creditsY2)
			{
				screen = 2;
			}

			// EXIT
			else if (mx >= exitX1 && mx <= exitX2 &&
				my >= exitY1 && my <= exitY2)
			{
				exit(0);
			}
		}

		// ==================================================
		// HOW TO PLAY / CREDITS
		// ==================================================

		else if (screen == 1 || screen == 2)
		{
			if (mx >= 35 && mx <= 180 &&
				my >= 30 && my <= 90)
			{
				screen = 0;
			}
		}
	}
}


// ======================================================
// Keyboard
// ======================================================

void iKeyboard(unsigned char key)
{
	// ESC = back to main menu
	if (key == 27)
	{
		if (screen == 1 ||
			screen == 2 ||
			screen == 3)
		{
			screen = 0;
		}
	}
}


// ======================================================
// Special Keyboard
// ======================================================

void iSpecialKeyboard(unsigned char key)
{
}

// ======================================================
// iGraphics Linker Error Prevention
// ======================================================
void fixedUpdate()
{
	// iGraphics এর নতুন ভার্সনের জন্য এই ফাঁকা ফাংশনটি বাধ্যতামূলক
}


// ======================================================
// MAIN
// ======================================================

int main()
{
	// Background Music
	PlaySound(
		TEXT("assets\\bgm.wav"),
		NULL,
		SND_FILENAME | SND_ASYNC | SND_LOOP
		);

	// Start iGraphics
	iInitialize(
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		"Echoes of Arcana"
		);
	iStart();
	return 0;
}