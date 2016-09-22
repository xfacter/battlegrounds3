 -------------------------------
|    Battlegrounds 3 - v0.4x    |
 -------------------------------
by Xfacter

[About]
BG3 is a hovertank deathmatch game. Using a variety of weapons, defeat
your opponents to bring glory to your homeland.

[Controls]
Movement - TRIANGLE, CIRCLE, CROSS, SQUARE
Look - Analog Nub
Change Weapon - D-Pad Left/Right
Fire - Right Trigger

(Note: Change look inversion and analog deadzone in the config.ini file)

[Gameplay]
There are a range of weapons in BG3 with varying levels of effectiveness.
In order to defeat an enemy you must deplete his shields and armor. The
machine gun is a good all-around weapon. The laser is effectiveagainst
shields. The tank shells are effective against armor. The missiles are
better all-around weapons.

[Content Creation]
It's pretty simple to make maps. Their format is described below.

All files for a map go in a folder named with a 2-digit number. This is so
the game can index all the maps easily. Within this folder are the following:

map.ini:		Configuration file used to set various values for the maps.
				Some of these need to be standardized. Checking the default
				maps should give a fairly straightforward view of what these
				should look like. When defining points on the map, use tile
				coordinates, eg, for the bottom left tile on the map, it
				would be "0 0" and for the top right tile of a 256x256 map
				it would be "255 255".
terrain.raw:	16-bit binary mapping of scaleable height values from lower
				left of map (beginning of file) to upper right of map (end
				of file)
terrain.tga:	Detail texture for heightmap, as it would look from a
				top-down perspective. For best performance, use palletized
				textures.

You can use whatever software you like to create the actual heightmaps. I use
earthsculptor - http://www.earthsculptor.com

[Credits]
Design, Coding, Graphics - Alex Wickes "Xfacter"
Tank Model - Harry Ridgeway - http://www.harrysite.net/
Sound FX - http://www.sounddogs.com/

Originally released for PSP-Hacks Homebrew Idol 2
http://www.psp-hacks.com/tag/homebrew-idol-2/
http://www.psp-hacks.com/forums/f141/homebrew-idol-2-battlegrounds-3-t239957/

[Special Thanks]
#psp-programing on irc.freenode.net - Couldn't have done it without you guys.
Raphael, Tomaz, InsertWittyName - Used much of your code for various purposes. Sorry if
	anything is not credited properly

[Contact]
Questions or comments? Want to help with development (coding, graphics, etc.)? Email me!
For more details on this and other great games, visit my website.
Please consider donating at my website, these games take a lot of time and effort to make.

email: xfacter@gmail.com
website: http://xfacter.wordpress.com/
