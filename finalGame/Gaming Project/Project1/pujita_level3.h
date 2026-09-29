#ifndef PUJITA_LEVEL3_H
#define PUJITA_LEVEL3_H

// ======================================================
// SCREEN SIZE
// (Redeclared here with include-guards so this header
//  can be compiled standalone in pujita_level3.cpp.
//  If your main file already defines these via #define,
//  these guards simply prevent a duplicate-definition error.)
// ======================================================

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 1365
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 768
#endif


// ======================================================
// SHARED HELPER (already defined in imain.cpp)
// pujita_level3.cpp reuses it instead of writing its own copy.
// ======================================================

extern void iTextCentered(int centerX, int y, const char* str, void* font);


// ======================================================
// PUBLIC API — call these from imain.cpp
// ======================================================

// Call ONCE, exactly when the screen switches to the Level 3 battle
// (e.g. screen = 9; initLevel3Game();)
void initLevel3Game();

// Call every frame inside iDraw() when screen == 9
void drawLevel3Game();

// Call every tick inside fixedUpdate() when screen == 9
void updateLevel3Game();


// ======================================================
// RESULT FLAGS — imain.cpp can read these if it wants to
// react to win/lose (optional; the level already shows its
// own victory/defeat overlay).
// ======================================================

extern bool level3GameWon;
extern bool level3GameLost;

#endif