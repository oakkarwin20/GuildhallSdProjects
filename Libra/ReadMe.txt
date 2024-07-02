										How to Use Libra
Summary

	- Upon application start, the game begins in "Attract mode" which is essentially a main menu splash screen. 

	- Players can start and enter the game mode in which the player tank, bullet (when fired) and enemy tanks wil render.

	- The playerTank renders in center of screen and moves according to inputs from an Xbox controller and/or a keyboard 	  		as specified below.

	- Enemy entities spawn on start up and move in directions as hardcoded.
	
	- Scorpio has ray-tracing functionality where the color changes when playerTank is in view.

	- Bullets fire when 'spacebar' is held down. 

	- Collision functionality exists between all entities except bullet.

	- Application quits process
		
		- exit game mode, then attract mode, then back to windows.


In Attract Mode
	Quit Game (Exit application)
	- Keyboard Keycode 'ESC'

In game mode
	Game Pause (Sound plays when pause is pressed)
	- Xbox BUTTON_Start
	- Keyboard Keycode 'P'

	Move playerTank
	- Xbox LEFT_JOYSTICK rotates in direction of analog
	- Keyboard Keycode 'S' moves West
	- Keyboard Keycode 'F' moves East
	- Keyboard Keycode 'E' moves North
	- Keyboard Keycode 'D' moves South

	Fire Bullet (*Note - bullets do not have delay when firing with key held down)
	- Keyboard Keycode 'SPACEBAR' 

	Quit Game mode (return to Attract Mode)
	- Keyboard Keycode 'ESC'

											Known Issues
1. PlayerTank movement is fully complete

2. Health system is NOT fully implemented. Entities can NOT kill each other yet. Code is partially implemented.

3. Bullets don't collide with walls or tiles. 

4. Enemy entities only have physics. No wandering or functionality for shooting bullets.

										
										Functional Features (what works)

1. Textures are fully functional.
2. Text-in-box.
3. SpriteAnimDef
4. Blend Mode support
5. Event systems
6. HeatMaps
7. XML support and utilities
8. Entity management









