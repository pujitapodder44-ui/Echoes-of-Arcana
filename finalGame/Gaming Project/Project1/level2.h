#ifndef LEVEL2_H
#define LEVEL2_H

#include <stdio.h>
#include <math.h>

// Level 2 Variables
static int l2_score = 0;
static int l2_lives = 3;
static bool l2_isGameOver = false;
static bool l2_isCompleted = false;

// Level 2 Functions
inline void initLevel2()
{
	l2_score = 0;
	l2_lives = 3;
	l2_isGameOver = false;
	l2_isCompleted = false;
}

inline void drawLevel2()
{
	if (l2_isGameOver)
	{
		iSetColor(255, 50, 50);
		iText(550, 380, "LEVEL 2: GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		return;
	}

	// Background or Grid rendering for Level 2
	iSetColor(15, 15, 35);
	iFilledRectangle(0, 0, 1365, 768);

	// Title / Level 2 HUD
	iSetColor(255, 215, 0);
	iText(580, 700, "--- LEVEL 2 ---", GLUT_BITMAP_TIMES_ROMAN_24);
	iText(50, 720, "Level 2 Under Construction", GLUT_BITMAP_HELVETICA_18);
}

inline void updateLevel2()
{
	if (l2_isGameOver || l2_isCompleted) return;

	// Level 2 update logic / Enemy movement goes here
}

inline void level2Keyboard(unsigned char key)
{
	// Level 2 specific controls
	if (key == 'r' || key == 'R')
	{
		initLevel2();
	}
}

#endif // LEVEL2_H