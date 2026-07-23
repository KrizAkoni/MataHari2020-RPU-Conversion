23JUL2026: Fixed freeplay option not showing and cleaned up bugs in adjust state definitions. 
22JUL2026: Fixed a potential initialization error in RPU.cpp line ~1226 - void InitializeU11PIA() had 0x30 instead of 0x31 -- RPU_DataWrite(ADDRESS_U11_A_CONTROL, 0x31); 
           The effect being traced was inconsistent read of the self test switch.

21JUL2026: Fixed logic inversion on Arduino jumper read for Dual Boot mode. boolean switchStateClosed = !digitalRead(RPU_SWITCH_PIN);
           Fixed chime assignments in MataHari2020.h
           Fixed chime push to solenoid stack routine and implemented apron credit lamp in MataHari2026Beta.ino
           Implemented some DIP switch pulls for BallsPerGame, HighScoreReplay, MaximumCredits, CreditDisplay, and MatchFeature
           Self test and audit needs to be reviewed - I pulled one in from somewhere else and think I broke it. Missing free play setting. 

20JUL2026: Updated RPU to handle REV 3 hardware creditresetbutton boot select. Currently configured to boot to native pin hardware unless credit start button is held. Investigating potential bug regarding handling of reset line in REV 4 hardware.

16JUL2026: Replaced BSOS files with RPU files and updating references, performing cleanup and debug.

## Mata Hari 2020
10JUL2026: Imported old BSOS files from https://github.com/RetroPinUpgrade/Stars2020-21 to make this stand alone. BSOS has migrated to RPU. The code will need to be ported to the newest libraries.

Note: This code has a dependency on BallySternOS - it won't build without those files. They're located in a repository here:
https://github.com/BallySternOS/BallySternOS
The base library is separated from this implementation so that it can be used by multiple projects without needing to be updated multiple times. For best results, always get all files (both the base library and the MataHari2020 files) each time you build. Read on for basic instructions on how to build this code.


### To use this code
* Download the zip file (Code > Download ZIP) or clone the repository to your hard drive.  
* Get the BallySternOS files ( BallySternOS.* and SelfTestAndAudit.* ) from the repository here:  
 * https://github.com/BallySternOS/BallySternOS/tree/master
 * (Code > Download ZIP)
* Unzip the MataHari2020 repository and name the folder that it's in as:
  * MataHari2020  
* Copy BallySternOS.* and SelfTestAndAudit.* into the MataHari2020 folder
* Open the MataHari2020.ino in Arduino's IDE
  
Refer to the PDF or [Wiki for instructions](https://ballysternos.github.io/) on how to build the hardware. 

# Coin Door Tests, Audits, and Settings  
```
Tests (test number shown in Credits, Ball in Play is blank)
1 - Lamps
2 - Displays
3 - Solenoids
4 - Switches
5 - Sounds (not applicable)

Settings & Audits (page number shown in Ball in Play, Credits is blank)
1 - Award Score 1
2 - Award Score 2
3 - Award Score 3
4 - High Score
5 - Credits
6 - Total Plays
7 - Total Replays
8 - High Score Beat
9 - Chute 2 Coins
10 - Chute 1 Coins
11 - Chute 3 Coins
12 - Free Play
13 - Ball Save
14 - Music Level
15 - Tournament Scoring
16 - Reboot (All displays show 8007 (as in "BOOT"), and Credit/Reset button restarts)
17 - Skill Shot
18 - Tilt Warnings
19 - Award Scores (0 = all extra balls, 7 = all specals)
20 - Number of Balls Per Game (99 = Use DIP Settings)
21 - Scrolling Scores
22 - Extra Ball Award (for tournament scoring)
23 - Special Award (for tournament scoring)
24 - Playfield Valid (0-3)
25 - Wizard Mode Time (15/30/45/60)
26 - Wizard Award
27 - Dim Level
```
