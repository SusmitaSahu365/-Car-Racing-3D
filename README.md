# Car Racing 3D

A text-mode car racing game written in C for **Turbo C / Turbo C++** (DOS), built as a Computer Graphics micro-project.

| | |
|---|---|
| **Institute** | Government Polytechnic Thane |
| **Department** | Computer Engineering |
| **Subject** | Computer Graphics (22318) |
| **Academic year** | 3rd Semester, 2022-23 |


---

## Game overview

You drive the green car at the bottom of a three-lane road. Red cars come down the lanes, and you change lanes to dodge them. Every car you pass scores **10 points**. Every **100 points** the game gets faster. If a red car hits you, the game is over.

The game always keeps at least one lane free near the top of the road, so every situation can be survived.

## Controls

| Key | Action |
|---|---|
| Left arrow | Move one lane left |
| Right arrow | Move one lane right |
| P | Pause (press any key to resume) |
| X | Exit |
| R | Play again (after Game Over) |

## Files

| File | Description |
|---|---|
| `CARRACE.C` | Complete game source for Turbo C (DOS) |
| `CARRACE_DEVCPP.cpp` | Same game for Dev-C++ / MinGW on Windows |
| `car_racing_3d.html` | Optional browser version with the same logic, for demos without Turbo C |
| `README.md` | This file |

## How to run

### Option 1: Dev-C++ on Windows
1. Open Dev-C++ and choose **File > New > Source File**.
2. Paste the contents of `CARRACE_DEVCPP.cpp` and save it (for example `carrace.cpp`).
3. Press **F11** (Compile & Run). The game opens in the console window.

This version replaces Turbo C's `conio.h` functions (`gotoxy`, `textcolor`, `clrscr`, `delay`, `sound`) with Windows console API calls, so the game logic is the same.
If the window does not resize to 80x25, run the generated `.exe` from Command Prompt instead of Windows Terminal.

### Option 2: Turbo C on DOS or DOSBox (the original project)
1. Install Turbo C / Turbo C++ (or use DOSBox with Turbo C mounted).
2. Open `CARRACE.C` in the Turbo C editor.
3. Press **Alt+F9** to compile, then **Ctrl+F9** to run.
4. Run it in a full-screen text window so the 80x25 layout shows correctly.

`CARRACE.C` uses Turbo C-specific headers (`conio.h`, `dos.h`), so it **will not compile** in Dev-C++, GCC, Clang or Visual Studio. Use `CARRACE_DEVCPP.cpp` for those.

### Option 3: Browser version
Open `car_racing_3d.html` in any modern browser. It works with the keyboard or the on-screen buttons on a phone.

## Screen layout (80 x 25 text mode)

```
cols 1-19      cols 20-50          cols 55-79
 blue bg    [ road, 3 lanes ]     info panel
            border = col 20       title, instructions,
            lanes  = 10 cols      score, level, status
            border = col 50
```

- Each lane is 10 columns wide.
- Each car is 6 columns wide and 4 rows tall, centred in its lane.
- The player's car is fixed at rows 21 to 24.

## How it works

The game is a loop. Each pass of the loop (one frame) does the following:

1. **Read the keyboard**: `kbhit()` checks for a key without stopping the game, and `getch()` reads it. Arrow keys come as two codes (a `0` followed by 75 for left or 77 for right).
2. **Maybe spawn an enemy car** with `random(6) == 0`, in a random free lane, just above the screen.
3. **Move every enemy car down one row**: erase it at the old position, add 1 to its row, and draw it at the new position.
4. **Check for a collision**: the game ends if an enemy is in the player's lane and its rows overlap the player's rows.
5. **Award points** when an enemy leaves the bottom of the screen, and speed up the game as the level rises.
6. **Redraw the player's car and the lane dividers**, then `delay(speed)`.

The delay starts at 80 ms and drops by 5 ms per level, with a minimum of 25 ms.

## Computer Graphics concepts used

| Concept | Where it appears |
|---|---|
| Text-mode screen addressing with `gotoxy()` | Placing every car, border and label |
| Colours with `textcolor()` and `textbackground()` | Blue background, black road, red/green cars |
| Drawing with extended ASCII characters | Solid block (219) for cars, double line (186) for borders, single line (179) for lane dividers |
| 2D object modelling | Car stored as a 6x4 character pattern (`carShape`) |
| Animation by erase and redraw | `drawCar()` with an erase mode |
| Clipping | Cars entering or leaving the screen only draw the rows that are on screen |
| Motion illusion | Scrolling dashed lane dividers in `drawDividers()` |
| Real-time input | `kbhit()` and `getch()` |
| Timing and sound | `delay()`, `sound()` and `nosound()` for pause and crash tones |

## Code structure

| Function | Purpose |
|---|---|
| `drawCar(lane, top, color)` | Draws a car, or erases it when `color < 0`, clipping at the screen edges |
| `drawRoad()` | Draws the black road and the border lines once per game |
| `drawDividers(tick)` | Draws the scrolling lane markers |
| `drawPanel()` | Draws the instructions and the labels for score, level and status |
| `showStats()` / `showStatus()` | Update the score, level and status text |
| `crashSound()` / `pauseSound()` | Sound effects |
| `play()` | Runs one full game and returns 1 to restart or 0 to quit |
| `main()` | Sets up the screen and repeats `play()` until the player quits |

## Settings you can change

All are `#define` values at the top of `CARRACE.C`:

| Constant | Meaning | Default |
|---|---|---|
| `LANES` | Number of lanes | 3 |
| `PLAYER_Y` | Row where the player's car sits | 21 |
| `MIN_DELAY` | Fastest game speed (smaller is faster) | 25 ms |

The starting speed (`speed = 80`) and the spawn rate (`random(6)`) are set inside `play()`.

## Known limitations

- `CARRACE.C` runs only in Turbo C or DOSBox, and `CARRACE_DEVCPP.cpp` runs only on Windows.
- It is a 2D text-mode game. The "3D" feel comes from the scrolling road and the moving cars, not from real 3D projection.
- The sounds use the PC speaker, so they may be silent in some DOSBox setups.

## Possible improvements

- High-score saving to a file
- Different car shapes or coloured traffic
- A "lives" system instead of instant Game Over
- Porting to `graphics.h` (BGI) for pixel-based drawing

## Learning outcomes

- Manipulating visual and geometric information on screen
- Understanding how real-time graphics loops, input and timing work together
- Developing graphics programs by writing your own algorithms
- Debugging a program through repeated testing and correction
