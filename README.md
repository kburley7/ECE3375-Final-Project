# Minesweeper VGA (DE10-Standard)

This is a classic 7x7 Minesweeper game implemented in C for the DE10-Standard FPGA board. The game renders graphics using the VGA pixel buffer and is controlled using the board's hardware buttons and switches.

---

## 🔧 Features
- 7x7 Minesweeper grid with 10 randomly placed mines
- VGA pixel buffer rendering (colored grid, digit drawing, flag icons)
- Highlighted cursor navigation
- Button-based tile reveal and movement
- Switch-based flagging and downward movement
- Automatic mine reveal upon hitting a mine

---

## 🧃 Controls

### 🔘 Push Buttons (KEY[3:0])
| Button     | Function            |
|------------|---------------------|
| `KEY[0]`   | Reveal current tile |
| `KEY[1]`   | Move right          |
| `KEY[2]`   | Move left           |
| `KEY[3]`   | Move up             |

### 🔀 Switches (SW[2:0])
| Switch     | Function             |
|------------|----------------------|
| `SW[0]`    | Move down (edge-detected) |
| `SW[1]`    | Toggle flag on tile (edge-detected) |
| `SW[2]`    | Restart the game |


---

## 🖥️ VGA Display
- **Each tile**: 30x30 pixels
- **Colors**:
  - Gray: Unrevealed tile
  - White: Revealed safe tile
  - Red: Mine
  - Blue: Number of adjacent mines (1–8)
  - Magenta: Flag
  - Yellow border: Cursor highlight

---

## 🛠️ Build & Flash

### 🔧 Requirements
- DE10-Standard FPGA board
- VGA monitor connection
- `arm-eabi-gcc` toolchain
- Linker script (`build_arm_c.ld` or similar)

### 🧱 Compilation Example
```bash
arm-eabi-gcc -Wall -O1 -mcpu=cortex-a9 -mfloat-abi=softfp -mno-unaligned-access -c -o minesweeper.o minesweeper_pixel_vga.c
arm-eabi-gcc -T build_arm_c.ld -o minesweeper.elf minesweeper.o
```

Flash `minesweeper.elf` to your board using the appropriate loader.

---

## 🎮 Gameplay
- Move the cursor with buttons/switches
- Reveal safe tiles to clear the board
- If you reveal a mine, the game ends and all mines are shown
- Use flags to mark potential mines

> ⚠️ The game runs indefinitely after loss — restart logic can be added optionally

---

## 🚀 Future Improvements
- Win condition detection
- Game reset via SW[2]
- Animated tile reveals
- Victory/loss message overlay
- Audio feedback

---

## 📁 Files
```
minesweeper.c   # Main game source code
README.md                 # This file
```

Enjoy classic Minesweeper rendered in pixels on your FPGA! 🎉
