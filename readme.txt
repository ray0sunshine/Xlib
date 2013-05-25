This is lunar lander...with a twist:
In a mission to explore the dark side of the moon, we have discovered strange alien warp holes spewing out large
numbers of alien spacecrafts placed on the lunar surface. There are designated landing zones setup for landing and
further investigation, but first it's a good idea to destroy all of the alien warp gates in the area...they seem hostile.

controls:
-both WASD and ARROWS can be used for activating vector thrusters
-SPACE works as specified: start, restart, pause, continue
-mouse + click: aim and fire plasma gatling gun
-q: quits the game entirely

cheats, mods & other:
-delete: turns off alien spawning, kills all aliens on map
-k: self-destruct, press this to see fireworks
-h: hardcore mode, touch any terrain other than landing pads or land too fast results in fireworks
-i: toggles fast health regeneration
-p: toggles fast power regeneration
-m: toggles no knock-back from getting hit by aliens
-f: toggles frames per second display

enhancements:
-Modification and cheat toggle indicators in top right
-FPS display, score display and alien warp-gate tracker on bottom left
-Health and Power indicator bars in top left:
	> changes color based on amount remaining
	> gradually regenerates
	> fireworks when health = 0
	> ship can lose power if plasma gun and thrusters are overly used (but it a fusion reactor, you'd have to actually try to run out)
-Pilot icon on bottom right...some .xbm and pixmaps:
	> reacts when taking damage
-Hi-tech ship:
	> glowing reactor in center, glows less when ship is running low on power
	> drawing using polygons, multiple points for collision
	> emits particles when thrusters activated
	> shoot things using mouse and plasma gatling gun
-Randomly generated terrain:
	> secondary terrain in background to give sense of depth
	> destructible terrain, shooting the ground and detonating the ship near terrain will create craters
	> finely tuned random sine functions for more convincing terrain generation
-Hostile aliens:
	> warp-gates spread on the ground that spawn aliens at random
	> aliens fly towards ship and self-detonates damaging the ship
-extended physics:
	> ship bounces off terrain with a damp if going fast enough
	> gravity and terminal velocity
	> things that Michael Bay likes
-dynamic camera:
	> used so the terrain can extend beyond screen limits
	> not stuck to center of ship, will follow and accelerate based on ship motion
-Particles:
	> ship hitting the terrain will create sparks
	> plasma shots emit particles and explode on contact with ground
	> aliens and warp-gates explode in what I attempted to make like lava explosions
	> particles also do physics and collisions YAY!!
	> fades over time
	> press k for Canada day celebration!
	
additional notes:
-Just "make" is sufficient for compile and run.
-Highly recommended to test on a native environment as opposed to VM, it will hold 60fps without any enhancments, but once the sparks go flying, the VM starts the crying.
-Also play maximized, there's potentially quite a bit of clicking.
-"too small" message is modified to be more informative and is only displayed during actual game play, doesn't really make sense to have it in menus
-Colors may appear very saturated or off hue on certain displays, it looked different on my VM compared to my linux installation (still correct colors just looks different)
-to get base functionalities, toggle hardcore mode and turn off alien spawns, landing pads are blue. (good idea to pause, enter toggles, un-pause)
-trust me, most of the values set are fine tuned.
-no amount of double buffering will save me from the lag of VMs...though it's still used anyways to prevent flicker.