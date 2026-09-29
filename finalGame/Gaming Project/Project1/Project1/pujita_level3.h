#ifndef PUJITA_LEVEL3_H
#define PUJITA_LEVEL3_H

// =====================================================
// ECHOES OF ARCANA
// LEVEL 3 - FINAL TRIAL
// =====================================================

// -----------------------------------------------------
// Level 3 initialization
// -----------------------------------------------------
void initLevel3();

// -----------------------------------------------------
// Level 3 drawing
// -----------------------------------------------------
void drawLevel3();

// -----------------------------------------------------
// Level 3 update / game logic
// -----------------------------------------------------
void updateLevel3();

// -----------------------------------------------------
// Level 3 keyboard controls
// -----------------------------------------------------
void level3Keyboard(unsigned char key);
void level3SpecialKeyboard(int key);

// -----------------------------------------------------
// Level 3 mouse controls
// -----------------------------------------------------
void level3Mouse(int button, int state, int x, int y);

// -----------------------------------------------------
// Level 3 mouse movement
// -----------------------------------------------------
void level3MouseMove(int x, int y);
void level3MousePassiveMove(int x, int y);

#endif