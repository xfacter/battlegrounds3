## About

Battlegrounds 3 is a hovertank deathmatch game for the Sony PSP. Use a deadly array of weapons to defeat the enemy and return glory to your homeland.

## Controls

| Action | Keys |
| ------ | ---- |
| Movement | △ ◯ ✕ □ |
| Look | Analog |
| Change Weapon | ⬅ / ➡ |
| Fire | RT |

(Note: Change look inversion and analog deadzone in the config.ini file)

## Gameplay

There are a range of weapons in BG3 with varying levels of effectiveness. In order to defeat an enemy you must deplete his shields and armor. The machine gun is a good all-around weapon. The laser is effective against shields. Tank shells are effective against armor. Missiles are great all-around.

## Content Creation

It's straightforward to make maps. All map files go in a folder named with a 2-digit number. This is so the game can index all the maps easily. Within this folder are the following:

* map.ini --- Configuration file used to set various values for the maps. Some of these need to be standardized. Checking the default maps should give a fairly straightforward view of what these should look like. When defining points on the map, use tile coordinates, eg, for the bottom left tile on the map, it would be "0 0" and for the top right tile of a 256x256 map it would be "255 255".
* terrain.raw --- 16-bit binary mapping of scaleable height values from lower left of map (beginning of file) to upper right of map (end of file)
* terrain.tga --- Detail texture for heightmap, as it would look from a top-down perspective. For best performance, use palletized textures.

You can use any heightmap tools that support the .raw and .tga files as described.

## Build

To build the project:

1. Install [psptoolchain](https://github.com/pspdev/psptoolchain/)
1. `make [release|debug]`
1. `make install`
  - Installs to `$DESTDIR` or `$PSP_ROOT` or `~/psproot`

## Credits

* Author / Maintainer -- [xfacter](https://battlegroundspsp.wordpress.com/)
* Tank Model -- [Harry Ridgeway](http://www.harrysite.net/)
* Sound FX -- http://www.sounddogs.com/
* Heightmap software -- http://www.earthsculptor.com
* Originally released for PSP-Hacks Homebrew Idol 2
  - http://www.psp-hacks.com/tag/homebrew-idol-2/
  - http://www.psp-hacks.com/forums/f141/homebrew-idol-2-battlegrounds-3-t239957/
* Special thanks
  - irc://chat.freenode.net/##psp-programming
  - Raphael, Tomaz, InsertWittyName
