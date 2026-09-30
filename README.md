
# 🪄 Echoes of Arcana

> A 2D Fantasy Adventure Game built with C++ and the iGraphics Library.

---

## 🎮 Game Description

**Echoes of Arcana** is a 2D fantasy adventure game developed using the **iGraphics library**.

The player takes the role of a new student at a magical academy who must complete **three magical trials** to prove their skills and graduate as a skilled mage.

Each trial introduces a different gameplay mechanic:

* 🧩 **Exploration** – Maze navigation
* ⚡ **Reflexes** – Catching and avoiding falling magical items
* ⚔️ **Strategy** – Fighting the final boss

The three trials are connected through one continuous magical-academy story.

---

## ✨ Features

* 🏰 Interactive fantasy-themed main menu
* 🧩 Three unique playable levels
* 🗺️ Maze exploration with wall collision
* 📚 Collectible enchanted books
* 🪙 Coin collection and scoring system
* 🪄 Falling magical items
* ⚡ Good and bad item collision system
* 🎯 Target score system
* 🔓 Level unlocking system
* 👾 Final boss battle
* 🪄 Spell casting
* ❤️ Player and boss health system
* ⌨️ Keyboard controls
* 💥 Collision detection
* 🏆 Victory and Game Over screens
* 📈 Increasing difficulty throughout the game

The core features are based on the project's documented gameplay and feature plan.

---

# 🧙 Gameplay

## 🗺️ Level 1 – Labyrinth of Whispers

The player enters a magical maze and must find the correct path to reach the academy's **Grand Hall**.

### Objective

* Navigate through the maze.
* Avoid walls.
* Collect enchanted books and coins.
* Increase the score through collectibles.
* Find the correct path and reach the exit.

### Main Mechanic

**Maze exploration + collision detection + item collection**

---

## 🧪 Level 2 – Potions and Spells Class

In this level, magical objects continuously fall from above.

The player must move left and right to collect useful magical objects while avoiding dangerous ones.

### Good Items

* 🪄 Wands
* 📖 Enchanted / Spell Books

### Dangerous Items

* ☠️ Poison Potions
* 🌑 Dark Artifacts

### Objective

Collect enough useful items and reach the required target score to unlock the final trial.

### Main Mechanic

**Falling-item system + collision detection + scoring**

---

## ⚔️ Level 3 – Final Trial Arena

The final trial takes place in the **Final Trial Arena**, where the player faces the **Arcane Guardian**.

### Objective

* Move around the battlefield.
* Avoid magical attacks.
* Cast spells against the Arcane Guardian.
* Reduce the guardian's health to zero.

### Winning Condition

The player wins when the Arcane Guardian's health reaches **0**.

### Losing Condition

The trial is lost if the player's health reaches **0** first.

### Main Mechanic

**Boss battle + spell casting + health system**

---

# 🎯 Game Rules

The three levels are played sequentially.

| Level   | Objective                  | Main Mechanic           |
| ------- | -------------------------- | ----------------------- |
| Level 1 | Reach the exit             | Maze exploration        |
| Level 2 | Reach the target score     | Falling-item collection |
| Level 3 | Defeat the Arcane Guardian | Boss battle             |

Each level has its own winning and losing conditions.

---

# ⌨️ Controls

## Level 1

| Action     | Keyboard  |
| ---------- | --------- |
| Move Up    | `W` / `↑` |
| Move Down  | `S` / `↓` |
| Move Left  | `A` / `←` |
| Move Right | `D` / `→` |
| Reset      | `R`       |

## Level 2

| Action     | Keyboard  |
| ---------- | --------- |
| Move Left  | `A` / `←` |
| Move Right | `D` / `→` |

## Level 3

| Action        | Keyboard   |
| ------------- | ---------- |
| Movement      | `________` |
| Spell Casting | `________` |

> ✏️ Replace the blank Level 3 controls with your final controls before submission.

---

# 🛠️ Project Details

| Category             | Details              |
| -------------------- | -------------------- |
| **Project Name**     | Echoes of Arcana     |
| **Language**         | C++                  |
| **Graphics Library** | iGraphics            |
| **IDE**              | Visual Studio 2013   |
| **Platform**         | Windows PC           |
| **Genre**            | 2D Fantasy Adventure |

---

# 💻 How to Run the Project

## Requirements

Make sure you have:

* **Visual Studio 2013**
* **iGraphics Library**
* Required project files
* Required game assets

The project was developed as a Windows-based iGraphics project using Visual Studio.

## Steps

1. Clone or download this repository.
2. Open the project in **Visual Studio 2013**.
3. Open the `.sln` file.
4. Make sure the iGraphics library is properly configured.
5. Make sure all required game assets are available in their correct folders.
6. Select **Build → Build Solution**.
7. Run using **Debug → Start Without Debugging**.

---

# 🖼️ Screenshots

## 🏠 Main Menu

<img width="1704" height="959" alt="Fig02_Level_Select" src="https://github.com/user-attachments/assets/5c9de293-56bb-49fb-97da-9ba7b9c76765" />


## 🗺️ Level 1 – Labyrinth of Whispers
<img width="1704" height="959" alt="Fig03_L1_Gameplay" src="https://github.com/user-attachments/assets/c7a0b0b4-3260-4193-aedc-282c56b96ec6" />


## 🧪 Level 2 – Potions and Spells Class
<img width="1704" height="959" alt="Fig06_L2_Gameplay" src="https://github.com/user-attachments/assets/3a8e45a3-a8d0-475c-a902-1b6c16c220d7" />


## ⚔️ Level 3 – Final Trial Arena 
<img width="1704" height="959" alt="Fig09_L3_Phase1_Spiders" src="https://github.com/user-attachments/assets/888bcbbf-b271-44e9-8ef5-e8ace259884b" />
<img width="1704" height="959" alt="Fig10_L3_Phase2_Minions" src="https://github.com/user-attachments/assets/f3b3d20c-6a5c-484d-9300-b28b1089e640" />
<img width="1704" height="959" alt="Fig11_L3_Phase3_Guardian" src="https://github.com/user-attachments/assets/9fbdd88c-2342-48c7-9fc9-f13bc89dde41" />



# 🎥 Gameplay Video

Watch the complete gameplay/demo here:

**YouTube Link:**
https://youtu.be/Gn5JBRQx3vI?si=pAwDPUOmGPXuERvQ

# 📄 Project Report

**Project Report:**https://github.com/pujitapodder44-ui/Echoes-of-Arcana/blob/main/Echoes_of_Arcana_Project_Final_Report-3%20(1).pdf

# 💡 Innovative Elements

The game combines three different beginner-friendly mechanics into one fantasy adventure:

1. **Maze Exploration** – Players explore and navigate through a magical maze.
2. **Falling-Item Challenge** – Players catch useful items and avoid harmful ones.
3. **Boss Battle** – Players fight the Arcane Guardian using spells.

The project also maintains a consistent magical-academy theme across all three levels, while each level introduces a different gameplay mechanic to reduce repetition.

---

# 👥 Project Contributors

### Team Members

1. **Pujita Podder**
2. **Debi Podder**
3. **Shahara Jebin Raisha**
4. **Nabonita Saha**

The project proposal identifies the work as a four-member team project.

---

# 🎨 Inspiration

**Echoes of Arcana** is inspired by:

* Classic fantasy adventure games
* Maze puzzle games
* Catch-and-avoid arcade games
* Simple action battle mechanics

---

# 📚 Project Information

**Course:** CSE-1200 – Software Development I
**Department:** Computer Science and Engineering
**Institution:** Ahsanullah University of Science and Technology (AUST)

**Project Title:** Echoes of Arcana

---

# 🏆 Conclusion

**Echoes of Arcana** combines exploration, reflex-based challenges, and magical combat into one fantasy adventure.

Through three different trials, the player progresses from a new student at a magical academy to a skilled mage. Each level provides a different gameplay experience while remaining connected through the same fantasy-themed story.

The project was developed collaboratively by a four-member team as a CSE-1200 Software Development I project.

---
