Mini Doom Port


Welcome to the Mini Doom Port project! This repository contains a minimalistic, hardware-optimized port inspired by the legendary game Doom. Designed for embedded systems, this project offers a compact, efficient implementation of a 2D raycasting engine and basic game mechanics tailored for microcontroller platforms.
________________________________________
Features
•	2D Raycasting Engine: Real-time rendering using a lightweight raycasting algorithm.
•	Basic Gameplay: Includes a player character, enemies, collectibles, and interactions.
•	Sound Integration: Uses embedded sound effects for immersive feedback.
•	Custom Input Handling:
o	Keyboard-based or SNES controller support.
o	Configurable GPIO inputs for flexibility.
•	Optimized Graphics:
o	Supports SSD1306 OLED displays.
o	Gradient effects and sprite rendering with minimal memory usage.
•	Entity System: Dynamic handling of players, enemies, and other interactable objects.
________________________________________
Requirements
Hardware
•	Microcontroller: Compatible with AVR-based systems (e.g., Arduino).
•	Display: SSD1306 OLED 128x64.
•	Input Devices:
o	GPIO buttons or SNES controller.
•	Sound: Configurable sound output pin for basic audio playback.
Software
•	Arduino IDE: Version 1.8.13 or later.
•	Adafruit SSD1306 Library: Required for display handling.
•	Hardware Timer: Utilized for sound effects and frame synchronization.
________________________________________
File Structure
•	doom.ino: The main game loop and logic.
•	entities.*: Manages game entities like the player, enemies, and collectibles.
•	input.*: Handles input from buttons or the SNES controller.
•	types.*: Core types and utilities for coordinates and unique IDs.
•	constants.h: Global definitions for gameplay settings and hardware pins.
•	sound.h: Embedded sound effects and playback utilities.
•	sprites.h: Bitmaps and sprite data for rendering graphics.
________________________________________
Getting Started
1.	Clone this repository:
bash
Copiar código
git clone https://github.com/yourusername/mini-doom-port.git
cd mini-doom-port
2.	Install required libraries in the Arduino IDE:
o	Adafruit GFX
o	Adafruit SSD1306
3.	Open doom.ino in the Arduino IDE.
4.	Configure hardware pins in constants.h to match your setup.
5.	Upload the code to your microcontroller.
________________________________________
Controls
Action	Keyboard (GPIO)	SNES Controller
Move Left	Configurable	Left D-Pad
Move Right	Configurable	Right D-Pad
Move Up	Configurable	Up D-Pad
Move Down	Configurable	Down D-Pad
Fire Weapon	Configurable	Y Button
Start Game	N/A	Start Button

