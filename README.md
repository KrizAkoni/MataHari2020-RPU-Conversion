21JUL2026: Fixed logic inversion on Arduino jumper read for Dual Boot mode. boolean switchStateClosed = !digitalRead(RPU_SWITCH_PIN);
           Fixed chime assignments in MataHari2020.h
           Fixed chime push to solenoid stack routine.
           Self test and audit needs to be reviewed - I pulled one in from somewhere else and think I broke it. 

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