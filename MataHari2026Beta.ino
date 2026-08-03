/**************************************************************************
    =======================================================================
    MataHari2026 V1.0 13JUL2026
    Updated by John Kriz (KrizAkoni@gmail.com) to new RPU code base.
    Introduced baseline game behaviors with improved MataHAri2020 mode
    overlays.   
    =======================================================================

    This file is part of MataHari2020.

    I, Dick Hamill, the author of this program disclaim all copyright
    in order to make this program freely available in perpetuity to
    anyone who would like to use it. Dick Hamill, 6/1/2020

    MataHari2020 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MataHari2020 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.

*/

#include "RPU_Config.h"
#include "RPU.h"
#include "MataHari2020.h"
#include "SelfTestAndAudit.h"
#include "AudioHandler.h"
#include <EEPROM.h>

// The defines for sound can be used separately or in combination
//#define USE_WAV_TRIGGER
//#define USE_WAV_TRIGGER_1p3
#define USE_CHIMES

/*
#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
#include <wavTrigger.h>
wavTrigger wTrig;             // Our WAV Trigger object
#endif
*/

#define MATAHARI2026_MAJOR_VERSION  2026
#define MATAHARI2026_MINOR_VERSION  1
#define DEBUG_MESSAGES  1

// This constant defines how much gap is inserted between chime hits
// during melodies and sound effects
// 40 - quick chimes
// 50 - normal (default)
// 75 - slow chimes
#define CHIME_SPACING_CONSTANT    50

/*********************************************************************

    Game specific code

*********************************************************************/

// MachineState
//  0 - Attract Mode
//  negative - self-test modes
//  positive - game play
char MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_UNVALIDATED     3
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_WIZARD_MODE     5
#define MACHINE_STATE_WAIT_FOR_BALL   6
#define MACHINE_STATE_COUNTDOWN_BONUS 99
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110

#define MACHINE_STATE_ADJUST_FREEPLAY           -17
#define MACHINE_STATE_ADJUST_BALL_SAVE          -18
#define MACHINE_STATE_ADJUST_MUSIC_LEVEL        -19
#define MACHINE_STATE_ADJUST_TOURNAMENT_SCORING -20
#define MACHINE_STATE_ADJUST_REBOOT             -21
#define MACHINE_STATE_ADJUST_SKILL_SHOT_AWARD   -22
#define MACHINE_STATE_ADJUST_TILT_WARNING       -23
#define MACHINE_STATE_ADJUST_AWARD_OVERRIDE     -24
#define MACHINE_STATE_ADJUST_BALLS_OVERRIDE     -25
#define MACHINE_STATE_ADJUST_SCROLLING_SCORES   -26
#define MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD   -27
#define MACHINE_STATE_ADJUST_SPECIAL_AWARD      -28
#define MACHINE_STATE_ADJUST_PLAYFIELD_VALID    -29
#define MACHINE_STATE_ADJUST_WIZARD_DURATION    -30
#define MACHINE_STATE_ADJUST_WIZARD_REWARD      -31
#define MACHINE_STATE_ADJUST_DIM_LEVEL          -32
#define MACHINE_STATE_ADJUST_DONE               -33

#define GAME_MODE_SKILL_SHOT            0
#define GAME_MODE_QUALIFY_SELECT        1
#define GAME_MODE_SELECT_MODE           2
#define GAME_MODE_AB_LANES              3
#define GAME_MODE_LEFT_DROP_TARGETS     4
#define GAME_MODE_RIGHT_DROP_TARGETS    5
#define GAME_MODE_POP_BUMPERS           6
#define GAME_MODE_SLINGS_AND_LANES      7
#define GAME_MODE_WIZARD                8

#define MODE_STATUS_BIT_AB_LANES          0x01
#define MODE_STATUS_BIT_LEFT_DROPS        0x02
#define MODE_STATUS_BIT_RIGHT_DROPS       0x04
#define MODE_STATUS_BIT_POP_BUMPERS       0x08
#define MODE_STATUS_BIT_SLINGS_AND_LANES  0x10

#define EEPROM_BALL_SAVE_BYTE           100
#define EEPROM_FREE_PLAY_BYTE           101
#define EEPROM_MUSIC_LEVEL_BYTE         102
#define EEPROM_SKILL_SHOT_BYTE          103
#define EEPROM_TILT_WARNING_BYTE        104
#define EEPROM_AWARD_OVERRIDE_BYTE      105
#define EEPROM_BALLS_OVERRIDE_BYTE      106
#define EEPROM_TOURNAMENT_SCORING_BYTE  107
#define EEPROM_SCROLLING_SCORES_BYTE    110
#define EEPROM_PLAYFIELD_VALID_BYTE     111
#define EEPROM_WIZARD_DURATION_BYTE     112
#define EEPROM_DIM_LEVEL_BYTE           113
#define EEPROM_EXTRA_BALL_SCORE_BYTE    140
#define EEPROM_SPECIAL_SCORE_BYTE       144
#define EEPROM_WIZARD_REWARD_BYTE       152

#define SOUND_EFFECT_NONE               0
#define SOUND_EFFECT_BONUS_COUNT        1
#define SOUND_EFFECT_2X_BONUS_COUNT     2
#define SOUND_EFFECT_3X_BONUS_COUNT     3
#define SOUND_EFFECT_5X_BONUS_COUNT     5
#define SOUND_EFFECT_OUTLANE_UNLIT      10
#define SOUND_EFFECT_OUTLANE_LIT        11
#define SOUND_EFFECT_INLANE             12
#define SOUND_EFFECT_BUMPER             13
#define SOUND_EFFECT_BUMPER_LIT         14
#define SOUND_EFFECT_DROP_TARGET        15
#define SOUND_EFFECT_ADD_CREDIT         16
#define SOUND_EFFECT_ADD_PLAYER         17
#define SOUND_EFFECT_PLAYER_UP          18
#define SOUND_EFFECT_BALL_OVER          19
#define SOUND_EFFECT_GAME_OVER          20
#define SOUND_EFFECT_EXTRA_BALL         21
#define SOUND_EFFECT_MACHINE_START      22
#define SOUND_EFFECT_SKILL_SHOT         23
#define SOUND_EFFECT_TILT_WARNING       24
#define SOUND_EFFECT_WIZARD_SCORE       25
#define SOUND_EFFECT_MATCH_SPIN         26
#define SOUND_EFFECT_WIZARD_TIMER       27
#define SOUND_EFFECT_SLING_SHOT         28
#define SOUND_EFFECT_10PT_SWITCH        29
#define SOUND_EFFECT_BUMPER_10          30
#define SOUND_EFFECT_AB_LANE_1          31
#define SOUND_EFFECT_AB_LANE_2          32
#define SOUND_EFFECT_AB_LANE_3          33
#define SOUND_EFFECT_SAUCER             34
#define SOUND_EFFECT_BIG_SLING_SHOT     35
#define SOUND_EFFECT_BIG_INLANE         36
#define SOUND_EFFECT_TIMEOUT            37
#define SOUND_EFFECT_BACKGROUND_1       90
#define SOUND_EFFECT_BACKGROUND_2       91
#define SOUND_EFFECT_BACKGROUND_3       92
#define SOUND_EFFECT_BACKGROUND_WIZ     93
#include "MataHari2020Chimes.h"

#define SKILL_SHOT_DURATION 15
#define MAX_DISPLAY_BONUS     69
#define TILT_WARNING_DEBOUNCE_TIME 1000

#define MODE_LENGTH_IN_SECONDS          30
#define AB_TIME_TO_QUALIFY_MODE         10
#define NUM_ORBITS_IN_AB_GOAL           2 //5
#define NUM_POP_BUMPERS_HIT_GOAL        2 //20
#define NUM_LEFT_TARGETS_GOAL           2 //8
#define NUM_RIGHT_TARGETS_GOAL          2 //8
#define NUM_SLINGS_AND_INLANES          2 //15


/*********************************************************************

    Machine state and options

*********************************************************************/
unsigned long HighScore = 0;
unsigned long AwardScores[3];
byte Credits = 0;
boolean FreePlayMode = false;
byte MusicLevel = 2;
byte BallSaveNumSeconds = 0;
unsigned long ExtraBallValue = 0;
unsigned long SpecialValue = 0;
unsigned long CurrentTime = 0;
byte MaximumCredits = 5;
byte BallsPerGame = 3;
byte InitialBonusXPotential = 1;
boolean CreditDisplay = false;
byte DimLevel = 2;
byte ScoreAwardReplay = 0;
boolean HighScoreReplay = false;
boolean MatchFeature = false;
byte SpecialLightAward = 0;
boolean BonusCountdown1000Steps = false;
boolean MaximumNumber4Players = true;
boolean TournamentScoring = false;
boolean ResetScoresToClearVersion = false;
boolean ScrollingScores = true;
unsigned long WizardSwitchReward = 10000;
byte WizardModeTimeLimit = 30;
byte dipBank0, dipBank1, dipBank2, dipBank3;
boolean GameReady = true;
boolean ABMaxedOut[4] = {false, false, false, false}; // Tracks maxed status for all 4 players
boolean leftBumperLit = false;
boolean rightBumperLit = false;
boolean outlanesSwapped = false;
//unsigned long LastABHitTime = 0;
boolean CurrentlyShowingABHits = false;
int LastABReportedValue = -1; 


/*********************************************************************

    Game State

*********************************************************************/
byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
byte Bonus;
byte BonusX;
byte BonusXPotential = InitialBonusXPotential; // Starts at DIP baseline

unsigned long CurrentScores[4];

byte ABLaneState;
byte ModeCompletionStatus[4];
byte PopBumperGoal[4];
byte ABLaneGoal[4];
byte SlingsAndLanesGoal[4];
byte LeftTargetGoal[4];
byte RightTargetGoal[4];

byte LeftOutlane;
byte RightOutlane;

byte PlayfieldValidation = 0;
byte MaxTiltWarnings = 2;
byte NumTiltWarnings = 0;

byte SkillShotAwardsLevel = 0;
byte GameMode = GAME_MODE_SKILL_SHOT;
byte ProspectiveGameMode = GAME_MODE_AB_LANES;
byte PopBumperPhase = 0;
byte LeftDropTargetStatus;
byte RightDropTargetStatus;
byte LeftDropsSweptCount = 0;
byte Full8DropsSweptCount = 0;
boolean LastTargetScoresSpecial = false;

boolean CurrentlyShowingBallSave = false;
boolean SkillShotRunning = false;
boolean SamePlayerShootsAgain = false;
boolean BallSaveUsed = false;
boolean DropTargetsScoreSpecial = false;


unsigned long BallFirstSwitchHitTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long LastTiltWarningTime = 0;
unsigned long GameModeStartTime = 0;
unsigned long GameModeEndTime = 0;
unsigned long LastModeShotTime = 0;
unsigned long ResetLeftDropTargetStatusTime;
unsigned long ResetRightDropTargetStatusTime;
unsigned long LastAHit = 0;
unsigned long LastBHit = 0;
unsigned long LastPopBumperHit = 0;
unsigned long LastTimeSlingOrLaneHit = 0;


void GetDIPSwitches() {
  dipBank0 = RPU_GetDipSwitches(0);
  dipBank1 = RPU_GetDipSwitches(1);
  dipBank2 = RPU_GetDipSwitches(2);
  dipBank3 = RPU_GetDipSwitches(3);
}

void DecodeDIPSwitchParameters() {
  // ScoreAwardReplay = (dipBank0&0x20) ? 7 : 0;
  // GameMelodyMinimal = (dipBank0&0x80)?false:true;
 
  BallsPerGame = (dipBank1 & 0x80) ? 5 : 3;
  HighScoreReplay = (dipBank1&0x20)?true:false;
  MaximumCredits = (dipBank2&0x07)*5 + 5;
  CreditDisplay = (dipBank2&0x08)?true:false;
  MatchFeature = (dipBank2&0x10)?true:false;
  InitialBonusXPotential = (dipBank2 & 0x40) ? 2 : 1;

}

void ReadStoredParameters() {
  HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
  Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
  if (Credits > MaximumCredits) Credits = MaximumCredits;

  ReadSetting(EEPROM_FREE_PLAY_BYTE, 0);
  FreePlayMode = (EEPROM.read(EEPROM_FREE_PLAY_BYTE)) ? true : false;

  BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, 16);
  if (BallSaveNumSeconds > 21) BallSaveNumSeconds = 16;

  MusicLevel = ReadSetting(EEPROM_MUSIC_LEVEL_BYTE, 2);
#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (MusicLevel > 5) MusicLevel = 5;
#else
  if (MusicLevel > 3) MusicLevel = 3;
#endif

  TournamentScoring = (ReadSetting(EEPROM_TOURNAMENT_SCORING_BYTE, 0)) ? true : false;

  SkillShotAwardsLevel = (ReadSetting(EEPROM_SKILL_SHOT_BYTE, 0)) ? true : false;

  MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, 2);
  if (MaxTiltWarnings > 2) MaxTiltWarnings = 2;

  byte awardOverride = ReadSetting(EEPROM_AWARD_OVERRIDE_BYTE, 99);
  if (awardOverride != 99) {
    ScoreAwardReplay = awardOverride;
  }

  byte ballsOverride = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  if (ballsOverride == 3 || ballsOverride == 5) {
    BallsPerGame = ballsOverride;
  } else {
    if (ballsOverride != 99) EEPROM.write(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  }

  ScrollingScores = (ReadSetting(EEPROM_SCROLLING_SCORES_BYTE, 1)) ? true : false;

  ExtraBallValue = RPU_ReadULFromEEProm(EEPROM_EXTRA_BALL_SCORE_BYTE);
  if (ExtraBallValue % 1000 || ExtraBallValue > 100000) ExtraBallValue = 20000;

  SpecialValue = RPU_ReadULFromEEProm(EEPROM_SPECIAL_SCORE_BYTE);
  if (SpecialValue % 1000 || SpecialValue > 100000) SpecialValue = 40000;

  PlayfieldValidation = ReadSetting(EEPROM_PLAYFIELD_VALID_BYTE, 1);
  if (PlayfieldValidation > 3) PlayfieldValidation = 1;

  WizardModeTimeLimit = ReadSetting(EEPROM_WIZARD_DURATION_BYTE, 30);
  if (WizardModeTimeLimit > 60) WizardModeTimeLimit = 30;

  WizardSwitchReward = RPU_ReadULFromEEProm(EEPROM_WIZARD_REWARD_BYTE);
  if (WizardSwitchReward % 5000 || WizardSwitchReward > 100000 || WizardSwitchReward == 0) WizardSwitchReward = 50000;

  DimLevel = ReadSetting(EEPROM_DIM_LEVEL_BYTE, 2);
  if (DimLevel < 2 || DimLevel > 3) DimLevel = 2;
  RPU_SetDimDivisor(1, DimLevel);

  AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
  AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
  AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);
}

//==================================================================
//==================================================================
void setup() {
/*  
  if (DEBUG_MESSAGES) {
    Serial.begin(115200);
  }
*/

  // Tell the OS about game-specific lights and switches
  RPU_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, TriggeredSwitches);

  // Set up dual boot configuration
  RPU_InitializeMPU(RPU_CMD_BOOT_ORIGINAL_IF_CREDIT_RESET | RPU_CMD_BOOT_ORIGINAL_IF_NOT_SWITCH_CLOSED, SW_CREDIT_RESET);

  // Set Quiescent State
  RPU_DisableSolenoidStack();
  RPU_SetDisableFlippers(true);

  // Use dip switches to set up game variables
  GetDIPSwitches();
  DecodeDIPSwitchParameters();

  // Read parameters from EEProm
  ReadStoredParameters();

  CurrentScores[0] = MATAHARI2026_MAJOR_VERSION;
  CurrentScores[1] = MATAHARI2026_MINOR_VERSION;
  CurrentScores[2] = RPU_OS_MAJOR_VERSION;
  CurrentScores[3] = RPU_OS_MINOR_VERSION;
  ResetScoresToClearVersion = true;

  #if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  // WAV Trigger startup at 57600
  wTrig.start();
  delay(10);

  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);
  #endif

  // Play machine start up sound and clear saucer
  CurrentTime = millis();
  PlaySoundEffect(SOUND_EFFECT_MACHINE_START);
  RPU_PushToSolenoidStack(SOL_SAUCER, 5, true);
  
}

byte ReadSetting(byte setting, byte defaultValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}

////////////////////////////////////////////////////////////////////////////
//
//  Lamp Management functions
//
////////////////////////////////////////////////////////////////////////////
void SetPlayerLamps(byte numPlayers, byte playerOffset = 0, int flashPeriod = 0) {
  // For Mata Hari, the "Player Up" lights are all +4 of the "Player" lights
  // so this function covers both sets of lights. Putting a 4 in playerOffset
  // will turn on/off the player up lights.
  for (int count = 0; count < 4; count++) {
    RPU_SetLampState(PLAYER_1 + playerOffset + count, (numPlayers == (count + 1)) ? 1 : 0, 0, flashPeriod);
  }
}

//================================================================
void ShowDropTargetSpecialLamp(byte mode, boolean isSpecialLit) {
  // RULE: The Special lamp should ONLY be active during QUALIFY_SELECT mode
  if (mode == GAME_MODE_QUALIFY_SELECT && isSpecialLit) {
    RPU_SetLampState(LAST_TARGET_SCORES_SPECIAL, 1); // True matching lamp constant
  } else {
    RPU_SetLampState(LAST_TARGET_SCORES_SPECIAL, 0); // Force OFF during other modes or if unlit
  }
}

//================================================================
void ShowBonusOnTree(byte bonus, byte dim=0) {
  if (bonus>MAX_DISPLAY_BONUS) bonus = MAX_DISPLAY_BONUS;

  if (bonus>=60) {
    RPU_SetLampState(BONUS_10, 1, dim, 250);
    bonus -= 20;
  } else if ( ((bonus/10)%2)==1 ) {
    RPU_SetLampState(BONUS_10, 1, dim);
    bonus -= 10;
  } else {
    RPU_SetLampState(BONUS_10, 0, dim, 250);
  }

  if (bonus>=40) {
    RPU_SetLampState(BONUS_20, 1, dim, 250);
    bonus -= 40;
  } else if (bonus>=20) {
    RPU_SetLampState(BONUS_20, 1, dim);
    bonus -= 20;   
  } else {
    RPU_SetLampState(BONUS_20, 0, dim);
  }
 
  for (byte count=0; count<9; count++) {
    if (count==(bonus-1)) RPU_SetLampState(BONUS_1+count, 1, dim);
    else RPU_SetLampState(BONUS_1+count, 0, dim);
  }
}

//================================================================
void ShowBonusLights(byte mode, byte prospectiveMode, byte bonus) {
  if (mode==GAME_MODE_QUALIFY_SELECT) {
    unsigned long mostRecentHit = LastAHit;
    if (LastBHit>mostRecentHit) mostRecentHit = LastBHit;
    if ((LastAHit || LastBHit) && ((CurrentTime-mostRecentHit)/1000)<AB_TIME_TO_QUALIFY_MODE) {
      byte numLights = 9 - (9*(CurrentTime-mostRecentHit)/1000)/AB_TIME_TO_QUALIFY_MODE;
      for (byte count=0; count<9; count++) RPU_SetLampState(BONUS_1+count, (count<numLights)?1:0, 1);
    } else {
      ShowBonusOnTree(bonus);
    }
  } else if (mode==GAME_MODE_SELECT_MODE) {
    if ((prospectiveMode==GAME_MODE_POP_BUMPERS || prospectiveMode==GAME_MODE_SLINGS_AND_LANES) && (((CurrentTime-GameModeStartTime)/2000)%2)==0) {
      byte lightPhase=100;
      if (prospectiveMode==GAME_MODE_POP_BUMPERS) {
        lightPhase = ((CurrentTime-GameModeStartTime)/100)%9;
        for (byte count=0; count<9; count++) RPU_SetLampState(BONUS_1+count, ((count==lightPhase)||(count==(lightPhase-1)))?1:0, (count==lightPhase)?0:1);
      } else if (prospectiveMode==GAME_MODE_SLINGS_AND_LANES) {
        lightPhase = 8-((CurrentTime-GameModeStartTime)/100)%9;
        for (byte count=0; count<9; count++) RPU_SetLampState(BONUS_1+count, ((count==lightPhase)||(count==(lightPhase+1)))?1:0, (count==lightPhase)?0:1);
      }
      RPU_SetLampState(BONUS_10, 0);
      RPU_SetLampState(BONUS_20, 0);
    } else {
      ShowBonusOnTree(bonus);
    }
  } else if (mode>=GAME_MODE_AB_LANES && mode<=GAME_MODE_SLINGS_AND_LANES) {
    // Show time remaining in mode
    byte displayPhase = ((CurrentTime-GameModeStartTime)/2000)%2;

    // For the moment, always show time countdown during a mode    
    if (displayPhase==0 || 1) {
      byte scaledTimeLeft = 0;
      if (GameModeEndTime>CurrentTime) {
        scaledTimeLeft = 9 - ((CurrentTime-GameModeStartTime)*9)/(GameModeEndTime-GameModeStartTime);
        for (byte count=0; count<9; count++) RPU_SetLampState(BONUS_1+count, (count<scaledTimeLeft)?1:0, 1, (scaledTimeLeft<3)?100:0);
      } else if (GameModeEndTime==0) {
        for (byte count=0; count<9; count++) RPU_SetLampState(BONUS_1+count, 1, 1, 250);
      } else {
        // This shouldn't happen (mode should end after CurrentTime>=GameModeEndTime
      }
      RPU_SetLampState(BONUS_10, 0);
      RPU_SetLampState(BONUS_20, 0);

    } else {
      // If we're in the other part of the display phase, show the bonus
      ShowBonusOnTree(bonus);
    }
  }
}

//================================================================
void ShowBonusXLights(byte mode, byte prospectiveMode, byte bonusX, unsigned long lastSlingAndLaneHit) {
  if (mode==GAME_MODE_SELECT_MODE) {  
    if (prospectiveMode==GAME_MODE_SLINGS_AND_LANES) {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/300)%2;
      RPU_SetLampState(BONUS_2X, (lightPhase==0)?1:0);
      RPU_SetLampState(BONUS_3X, (lightPhase==1)?1:0);
      RPU_SetLampState(BONUS_5X, (lightPhase==0)?1:0);
      RPU_SetLampState(SHOOT_AGAIN, (lightPhase==1)?1:0);
    } else {
      RPU_SetLampState(BONUS_2X, (bonusX==2)?1:0);
      RPU_SetLampState(BONUS_3X, (bonusX==3)?1:0);
      RPU_SetLampState(BONUS_5X, (bonusX==5)?1:0);
    }
  } else if (mode==GAME_MODE_SLINGS_AND_LANES) {
      if ((CurrentTime-lastSlingAndLaneHit)<400) {
        // Solid (no flash) right after a hit
        RPU_SetLampState(BONUS_2X, 1);
        RPU_SetLampState(BONUS_3X, 1);
        RPU_SetLampState(BONUS_5X, 1);
        RPU_SetLampState(SHOOT_AGAIN, 1);
     } else {
        // Flash the active multiplier
        RPU_SetLampState(BONUS_2X, 1, 0, 250);
        RPU_SetLampState(BONUS_3X, 1, 0, 250);
        RPU_SetLampState(BONUS_5X, 1, 0, 250);
        RPU_SetLampState(SHOOT_AGAIN, 1, 0, 250);
     }
  } else {
    RPU_SetLampState(BONUS_2X, (bonusX==2)?1:0);
    RPU_SetLampState(BONUS_3X, (bonusX==3)?1:0);
    RPU_SetLampState(BONUS_5X, (bonusX==5)?1:0);
  }
}

//================================================================
void ShowOutlanes(byte mode, byte prospectiveMode, bool leftOutlaneLit, bool rightOutlaneLit, unsigned long lastSlingAndLaneHit) {
  //Outlane Swapper
  boolean leftWants50K = (BonusXPotential == 5);
  boolean rightWants50K = (BonusX == 5);
  // Apply the swap logic to the lamps based on the bumper flag
  if (outlanesSwapped == false) {
    if (leftWants50K)  leftOutlaneLit = true;
    if (rightWants50K) rightOutlaneLit = true;
  } else {
    // Flipped state: Left lamp reads the Right variable, Right lamp reads the Left variable!
    if (rightWants50K) leftOutlaneLit = true;
    if (leftWants50K)  rightOutlaneLit = true;
  }
  
  if (mode==GAME_MODE_SELECT_MODE) {  
    if (prospectiveMode==GAME_MODE_SLINGS_AND_LANES) {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/300)%2;
      RPU_SetLampState(LEFT_OUTLANE_50, (lightPhase==1)?1:0, 1);
      RPU_SetLampState(RIGHT_OUTLANE_50, (lightPhase==0)?1:0, 1);
    } else {
      RPU_SetLampState(LEFT_OUTLANE_50, (leftOutlaneLit)?1:0, 1);
      RPU_SetLampState(RIGHT_OUTLANE_50, (rightOutlaneLit)?1:0, 1);
    }
  } else if (mode==GAME_MODE_SLINGS_AND_LANES) {
    if ((CurrentTime-lastSlingAndLaneHit)<150) {
      RPU_SetLampState(LEFT_OUTLANE_50, 1);
      RPU_SetLampState(RIGHT_OUTLANE_50, 1);
    } else {
      RPU_SetLampState(LEFT_OUTLANE_50, (leftOutlaneLit)?1:0, 1);
      RPU_SetLampState(RIGHT_OUTLANE_50, (rightOutlaneLit)?1:0, 1);
    }

  } else {
    RPU_SetLampState(LEFT_OUTLANE_50, (leftOutlaneLit)?1:0);
    RPU_SetLampState(RIGHT_OUTLANE_50, (rightOutlaneLit)?1:0);
  } 
}

//================================================================
// Saucer Lamp Control
//================================================================
unsigned long LastShowSaucerLamps = 0;
void ShowSaucerLamps(byte mode) {
// ------------------------------------------------------------------
  // 1. ACTIVE WIZARD FRENZY LIGHTS (Highest Priority)
  // ------------------------------------------------------------------
  if (mode == GAME_MODE_WIZARD) {
    // While the 30-second celebration is active, strobe all three saucer inserts together!
    RPU_SetLampState(BONUS_2X_POTENTIAL, 1);      
    RPU_SetLampState(BONUS_3X_POTENTIAL, 1);      
    RPU_SetLampState(BONUS_5X_POTENTIAL, 1);
    return; 
  }
  
  // ------------------------------------------------------------------
  // 2. WIZARD READY CALL-TO-ACTION (Qualified but not started)
  // ------------------------------------------------------------------
  else if (ModeCompletionStatus[CurrentPlayer] == 0x1F) {
    // If all 5 modes are done, but the ball is still out on the wood,
    // lock all three lights solidly ON (or duplicate your favorite clean phase)
    // to scream to the player that the finale is waiting to be claimed!
    RPU_SetLampState(BONUS_2X_POTENTIAL, 1, 0, 100);      
    RPU_SetLampState(BONUS_3X_POTENTIAL, 1, 0, 100);      
    RPU_SetLampState(BONUS_5X_POTENTIAL, 1, 0, 100);
    return; 
  }


  LastShowSaucerLamps = CurrentTime;
  if (mode==GAME_MODE_SKILL_SHOT) {
    byte lightPhase = ((CurrentTime-GameModeStartTime)/250)%24;
    if (lightPhase>14) {
      lightPhase-=15;
      SkillShotRunning = true;
      RPU_SetLampState(BONUS_2X_POTENTIAL, (lightPhase%3)==0);      
      RPU_SetLampState(BONUS_3X_POTENTIAL, (lightPhase%3)==1);      
      RPU_SetLampState(BONUS_5X_POTENTIAL, (lightPhase%3)==2);      
    } else {
      SkillShotRunning = false;
      RPU_SetLampState(BONUS_2X_POTENTIAL, 0);
      RPU_SetLampState(BONUS_3X_POTENTIAL, 0);
      RPU_SetLampState(BONUS_5X_POTENTIAL, 0);
    }
  
  } else if (mode == GAME_MODE_SELECT_MODE) {
      // 200ms per step. Modulo 3-step loop (0, 1, 2)
      byte selectPhase = ((CurrentTime - GameModeStartTime) / 200) % 3;
      RPU_SetLampState(BONUS_2X_POTENTIAL, (selectPhase == 0) ? 1 : 0);
      RPU_SetLampState(BONUS_3X_POTENTIAL, (selectPhase == 1) ? 1 : 0);
      RPU_SetLampState(BONUS_5X_POTENTIAL, (selectPhase == 2) ? 1 : 0);
  
  } else {
    // Light appropriate BonusXPoteltial
    RPU_SetLampState(BONUS_2X_POTENTIAL, (BonusXPotential == 2) ? 1 : 0);
    RPU_SetLampState(BONUS_3X_POTENTIAL, (BonusXPotential == 3) ? 1 : 0);
    RPU_SetLampState(BONUS_5X_POTENTIAL, (BonusXPotential == 5) ? 1 : 0);
  }
}

//================================================================
void ShowPopBumperLamps(byte mode, byte prospectiveMode, byte popStatus, unsigned long lastTimePopBumperHit) {
if (mode == GAME_MODE_SELECT_MODE) {
    byte lightPhase = ((CurrentTime - GameModeStartTime) / 200) % 2;
    if (prospectiveMode == GAME_MODE_POP_BUMPERS) {
      RPU_SetLampState(POP_BUMPER_1, lightPhase % 2);
      RPU_SetLampState(POP_BUMPER_2, (lightPhase % 2) ? 0 : 1);
    } else {
      // If a different mode is being highlighted, show strategic light states
      RPU_SetLampState(POP_BUMPER_1, leftBumperLit ? 1 : 0);
      RPU_SetLampState(POP_BUMPER_2, rightBumperLit ? 1 : 0);
    }
  } else if (mode == GAME_MODE_POP_BUMPERS) {
    if ((CurrentTime - lastTimePopBumperHit) < 1000) {
      RPU_SetLampState(POP_BUMPER_1, 1, 0, 100);
      RPU_SetLampState(POP_BUMPER_2, 1, 0, 100);
    } else {
       byte lightPhase = ((CurrentTime - GameModeStartTime) / 400) % 2;
      RPU_SetLampState(POP_BUMPER_1, 1, lightPhase);
      RPU_SetLampState(POP_BUMPER_2, 1, (lightPhase) ? false : true);
    }
  } else if (mode == GAME_MODE_SLINGS_AND_LANES) {
    if ((CurrentTime - LastTimeSlingOrLaneHit) < 150) {
      RPU_SetLampState(POP_BUMPER_1, 1);
      RPU_SetLampState(POP_BUMPER_2, 1);
    } else {
      RPU_SetLampState(POP_BUMPER_1, 0); 
      RPU_SetLampState(POP_BUMPER_2, 0); 
    }
  } else {
    // 3. BASELINE GAMEPLAY FALLBACK: For all other states (Skill Shot, Qualify, Drop Targets, etc.)
    RPU_SetLampState(POP_BUMPER_1, leftBumperLit ? 1 : 0);
    RPU_SetLampState(POP_BUMPER_2, rightBumperLit ? 1 : 0);
  }
}

//================================================================
void ShowABLamps(byte mode, byte prospectiveMode, byte abStatus) {
  bool showABStatus = false;
  
  if (mode==GAME_MODE_SKILL_SHOT) {
    byte lightPhase = ((CurrentTime-GameModeStartTime)/250)%24;
    if (lightPhase<8) {
      RPU_SetLampState(A_LANE, 1, lightPhase%2);
      RPU_SetLampState(B_LANE, 0);
    } else if (lightPhase<16) {
      RPU_SetLampState(A_LANE, 0);
      RPU_SetLampState(B_LANE, 1, lightPhase%2);
    } else {
      RPU_SetLampState(A_LANE, 0);
      RPU_SetLampState(B_LANE, 0);      
    }
  } 
  else if (mode==GAME_MODE_QUALIFY_SELECT) {
    unsigned long mostRecentHit = LastAHit;
    if (LastBHit>mostRecentHit) mostRecentHit = LastBHit;
    if ((LastAHit || LastBHit) && ((CurrentTime-mostRecentHit)/1000)<AB_TIME_TO_QUALIFY_MODE) {  
      RPU_SetLampState(A_LANE, 1, 1, (mostRecentHit==LastBHit)?100:0);
      RPU_SetLampState(B_LANE, 1, 1, (mostRecentHit==LastAHit)?100:0);
    } else {      
      showABStatus = true;
    }
  } 
  else if (mode==GAME_MODE_SELECT_MODE) {
    if (prospectiveMode==GAME_MODE_AB_LANES) {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/500)%2;
      RPU_SetLampState(A_LANE, lightPhase%2);
      RPU_SetLampState(B_LANE, (lightPhase%2)?0:1);
    } 
    else if (prospectiveMode==GAME_MODE_WIZARD) {
      byte lightPhase = (CurrentTime / 250) % 4;
      RPU_SetLampState(A_LANE, (lightPhase) ? 1 : 0, (lightPhase % 2) ? 1 : 0);
      RPU_SetLampState(B_LANE, (lightPhase) ? 1 : 0, (lightPhase % 2) ? 1 : 0);
    } 
    else {
      showABStatus = true;
    }
  }
  else if (mode==GAME_MODE_AB_LANES) {
    if (LastModeShotTime && (CurrentTime-LastModeShotTime)<1000) {
      RPU_SetLampState(A_LANE, 1, 0, 200);
      RPU_SetLampState(B_LANE, 1, 0, 200);
    } else {
      byte lightPhase = ((CurrentTime-GameModeStartTime)/250)%2;
      RPU_SetLampState(A_LANE, lightPhase%2);
      RPU_SetLampState(B_LANE, (lightPhase%2)?0:1);
    }
  }
  else if (mode == GAME_MODE_WIZARD) {
    byte lightPhase = (CurrentTime / 125) % 2; 
    RPU_SetLampState(A_LANE, lightPhase == 0);
    RPU_SetLampState(B_LANE, lightPhase == 1);
  } 
  else {
    showABStatus = true;
  }

  if (showABStatus) {
    byte aStatus = abStatus & 0x0F;
    byte bStatus = (abStatus & 0xF0)>>4;
    if (aStatus==bStatus) {
      byte lightPhase = ((CurrentTime)/250)%4;
      RPU_SetLampState(A_LANE, (lightPhase)?1:0, (lightPhase%2)?1:0);
      RPU_SetLampState(B_LANE, (lightPhase)?1:0, (lightPhase%2)?1:0);
    } else {
      RPU_SetLampState(A_LANE, 1, (aStatus>bStatus)?1:0, (aStatus<bStatus)?100:0);
      RPU_SetLampState(B_LANE, 1, (aStatus<bStatus)?1:0, (aStatus>bStatus)?100:0);
    }
  }
}

//================================================================
void ShowABRewardLamps(byte mode, byte prospectiveMode, byte abStatus) {

  byte modeShown = mode;
  
  if (mode==GAME_MODE_SKILL_SHOT) {
    for (int count=0; count<7; count++) RPU_SetLampState(AB_SCORES_1000+count, 0);
  } 
  else {
    if (mode==GAME_MODE_SELECT_MODE) {
      // The first two seconds, we'll show lights to point to the drop targets
      if (prospectiveMode==GAME_MODE_LEFT_DROP_TARGETS || prospectiveMode==GAME_MODE_RIGHT_DROP_TARGETS || prospectiveMode==GAME_MODE_AB_LANES) {
        if ((((CurrentTime-GameModeStartTime)/2000)%2)==0) {
          modeShown=prospectiveMode;        
        }        
      }
    }
    
    if (modeShown==GAME_MODE_LEFT_DROP_TARGETS) {
      byte phase = ((CurrentTime-GameModeStartTime)/100)%10;
      if (phase<7) {
        for (int count=0; count<7; count++) RPU_SetLampState(AB_SCORES_1000+count, (count==(6-phase))?1:0);
      } else {
        for (int count=1; count<7; count++) RPU_SetLampState(AB_SCORES_1000+count, 0);
        RPU_SetLampState(AB_SCORES_1000, (phase%2)?0:1);
      }
    } else if (modeShown==GAME_MODE_RIGHT_DROP_TARGETS) {
      byte phase = ((CurrentTime-GameModeStartTime)/100)%10;
      if (phase<7) {
        for (int count=0; count<7; count++) RPU_SetLampState(AB_SCORES_1000+count, (count==(phase))?1:0);
      } else {
        for (int count=0; count<6; count++) RPU_SetLampState(AB_SCORES_1000+count, 0);
        RPU_SetLampState(AB_SCORES_SPECIAL, (phase%2)?0:1);
      }
    
      } else {
        byte abWillScore = (abStatus&0x0F);
        byte bStatus = (abStatus&0xF0)>>4;
        if (bStatus<abWillScore) abWillScore = bStatus;
      
        // NATIVE TRANSLATION: Baseline 1 maps to Array Index 0 (1000 lamp)
        if (abWillScore > 0) {
          abWillScore -= 1;
      }
      
      // Show the current state of the AB reward across all 7 states
      for (int count=0; count<7; count++) {
        RPU_SetLampState(AB_SCORES_1000+count, (count==abWillScore)?1:0, 0);  
      }
    }
  }
}

//================================================================
void ShowSamePlayerLamps(byte mode, byte prospectiveMode) {

  // Skip lamp updates in this specific mode transition - the mode uses these lamps to attract
  if ((mode == GAME_MODE_SELECT_MODE && prospectiveMode == GAME_MODE_SLINGS_AND_LANES) || (mode == GAME_MODE_SLINGS_AND_LANES)) {
    return;
  }

  // --- Ball Save flashing ---
  if (!BallSaveUsed && BallFirstSwitchHitTime != 0) {
    unsigned long elapsed = (CurrentTime - BallFirstSwitchHitTime) / 1000;

    if (elapsed < BallSaveNumSeconds) {
      // Still in ball-save window
      if (elapsed >= (BallSaveNumSeconds - 3)) {
        // Last 3 seconds → fast flash
        RPU_SetLampState(SHOOT_AGAIN, 1, 0, 100);
      } else {
        // Normal ball-save flash
        RPU_SetLampState(SHOOT_AGAIN, 1, 0, 500);
      }
      return;   // Don’t fall through to solid extra-ball state
    }
  }
  // --- WIZARD Save flashing ---
  if (mode == GAME_MODE_WIZARD) {
        // fast flash
        RPU_SetLampState(SHOOT_AGAIN, 1, 0, 100);
      return;   // Don’t fall through to solid extra-ball state
  }
  // --- Normal / Extra Ball solid state ---
  RPU_SetLampState(SAME_PLAYER_SHOOTS_AGAIN, SamePlayerShootsAgain);
  RPU_SetLampState(SHOOT_AGAIN, SamePlayerShootsAgain);
}

////////////////////////////////////////////////////////////////////////////
//
//  Display Management functions
//
////////////////////////////////////////////////////////////////////////////
unsigned long LastTimeScoreChanged = 0;
unsigned long LastTimeOverrideAnimated = 0;
unsigned long LastFlashOrDash = 0;
unsigned long ScoreOverrideValue[4]= {0, 0, 0, 0};
byte ScoreOverrideStatus = 0;
byte LastScrollPhase = 0;

byte MagnitudeOfScore(unsigned long score) {
  if (score == 0) return 0;

  byte retval = 0;
  while (score > 0) {
    score = score / 10;
    retval += 1;
  }
  return retval;
}

//================================================================
void OverrideScoreDisplay(byte displayNum, unsigned long value, boolean animate) {
  if (displayNum>3) return;
  ScoreOverrideStatus |= (0x10<<displayNum);
  if (animate) ScoreOverrideStatus |= (0x01<<displayNum);
  else ScoreOverrideStatus &= ~(0x01<<displayNum);
  ScoreOverrideValue[displayNum] = value;
}

byte GetDisplayMask(byte numDigits) {
  byte displayMask = 0;
  for (byte digitCount=0; digitCount<numDigits; digitCount++) {
    displayMask |= (0x20>>digitCount);
  }  
  return displayMask;
}

//================================================================
void ShowPlayerScores(byte displayToUpdate, boolean flashCurrent, boolean dashCurrent, unsigned long allScoresShowValue=0) {

  if (displayToUpdate==0xFF) ScoreOverrideStatus = 0;

  byte displayMask = 0x3F;
  unsigned long displayScore = 0;
  unsigned long overrideAnimationSeed = CurrentTime/250;
  byte scrollPhaseChanged = false;

  byte scrollPhase = ((CurrentTime-LastTimeScoreChanged)/250)%16;
  if (scrollPhase!=LastScrollPhase) {
    LastScrollPhase = scrollPhase;
    scrollPhaseChanged = true;
  }

  for (byte scoreCount=0; scoreCount<4; scoreCount++) {
    // If this display is currently being overriden, then we should update it
    if (allScoresShowValue==0 && (ScoreOverrideStatus & (0x10<<scoreCount))) {
      displayScore = ScoreOverrideValue[scoreCount];
      byte numDigits = MagnitudeOfScore(displayScore);
      if (numDigits==0) numDigits = 1;
      if (numDigits<5 && (ScoreOverrideStatus & (0x01<<scoreCount))) {
        if (overrideAnimationSeed!=LastTimeOverrideAnimated) {
          LastTimeOverrideAnimated = overrideAnimationSeed;
          byte shiftDigits = (overrideAnimationSeed)%((7-numDigits)+(5-numDigits));
          if (shiftDigits>=(7-numDigits)) shiftDigits = (6-numDigits)*2-shiftDigits;
          byte digitCount;
          displayMask = GetDisplayMask(numDigits);
          for (digitCount=0; digitCount<shiftDigits; digitCount++) {
            displayScore *= 10;
            displayMask = displayMask>>1;
          }
          RPU_SetDisplayBlank(scoreCount, 0x00);
          RPU_SetDisplay(scoreCount, displayScore, false);
          RPU_SetDisplayBlank(scoreCount, displayMask);
        }
      } else {
        RPU_SetDisplay(scoreCount, displayScore, true);
      }
      
    } else {
      // No override, update scores designated by displayToUpdate
      if (allScoresShowValue==0) displayScore = CurrentScores[scoreCount];
      else displayScore = allScoresShowValue;
      
      if (displayToUpdate==0xFF || displayToUpdate==scoreCount || displayScore>999999) {

        // Don't show this score if it's not a current player score (even if it's scrollable)
        if (displayToUpdate==0xFF && (scoreCount>=CurrentNumPlayers&&CurrentNumPlayers!=0) && allScoresShowValue==0) {
          RPU_SetDisplayBlank(scoreCount, 0x00);
          continue;
        }

if (displayScore > 999999) {
  // Save the real score we want to display (works for both CurrentScores and allScoresShowValue)
  unsigned long originalScore = displayScore;

  // Score needs to be scrolled
  if ((CurrentTime - LastTimeScoreChanged) < 4000) {
    RPU_SetDisplay(scoreCount, originalScore % 1000000, true);  
  } else {

    // Scores are scrolled 10 digits and then we wait for 6
    if (scrollPhase < 11 && scrollPhaseChanged) {
      byte numDigits = MagnitudeOfScore(originalScore);
      
      // Figure out top part of score
      if (scrollPhase < 6) {
        displayMask = 0x3F;
        displayScore = originalScore;          // start from the real score
        for (byte scrollCount = 0; scrollCount < scrollPhase; scrollCount++) {
          displayScore = (displayScore % 1000000) * 10;
          displayMask = displayMask >> 1;
        }
      } else {
        displayScore = 0; 
        displayMask = 0x00;
      }

      // Add in lower part of score
      if ((numDigits + scrollPhase) > 10) {
        byte numDigitsNeeded = (numDigits + scrollPhase) - 10;
        unsigned long tempScore = originalScore;   // ← use the saved original, not CurrentScores
        for (byte scrollCount = 0; scrollCount < (numDigits - numDigitsNeeded); scrollCount++) {
          tempScore /= 10;
        }
        displayMask |= GetDisplayMask(MagnitudeOfScore(tempScore));
        displayScore += tempScore;
      }
      RPU_SetDisplayBlank(scoreCount, displayMask);
      RPU_SetDisplay(scoreCount, displayScore);
    }
  }          
} else {
          if (flashCurrent) {
            unsigned long flashSeed = CurrentTime/250;
            if (flashSeed != LastFlashOrDash) {
              LastFlashOrDash = flashSeed;
              if (((CurrentTime/250)%2)==0) RPU_SetDisplayBlank(scoreCount, 0x00);
              else RPU_SetDisplay(scoreCount, displayScore, true, 2);
            }
          } else if (dashCurrent) {
            unsigned long dashSeed = CurrentTime/50;
            if (dashSeed != LastFlashOrDash) {
              LastFlashOrDash = dashSeed;
              byte dashPhase = (CurrentTime/60)%36;
              byte numDigits = MagnitudeOfScore(displayScore);
              if (dashPhase<12) { 
                displayMask = GetDisplayMask((numDigits==0)?2:numDigits);
                if (dashPhase<7) {
                  for (byte maskCount=0; maskCount<dashPhase; maskCount++) {
                    displayMask &= ~(0x01<<maskCount);
                  }
                } else {
                  for (byte maskCount=12; maskCount>dashPhase; maskCount--) {
                    displayMask &= ~(0x20>>(maskCount-dashPhase-1));
                  }
                }
                RPU_SetDisplay(scoreCount, displayScore);
                RPU_SetDisplayBlank(scoreCount, displayMask);
              } else {
                RPU_SetDisplay(scoreCount, displayScore, true, 2);
              }
            }
          } else {
            RPU_SetDisplay(scoreCount, displayScore, true, 2);          
          }
        }
      } // End if this display should be updated
    } // End on non-overridden
  } // End loop on scores
}


////////////////////////////////////////////////////////////////////////////
//
//  Machine State Helper functions
//
////////////////////////////////////////////////////////////////////////////

boolean AddPlayer(boolean resetNumPlayers = false) {
  RPU_SetLampState(APRON_CREDIT, (Credits || FreePlayMode));
  if (Credits < 1 && !FreePlayMode) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  // if (CurrentNumPlayers >= 4) return false;

  // if (Credits < 1 && !FreePlayMode) return false;
  // if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers >= 4 || (CurrentNumPlayers >= 2 && !MaximumNumber4Players)) return false;

  CurrentNumPlayers += 1;
  RPU_SetDisplay(CurrentNumPlayers - 1, 0, true, 2);
//  RPU_SetDisplayBlank(CurrentNumPlayers - 1, 0x30);

  if (!FreePlayMode) {
    Credits -= 1;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_SetDisplayCredits(Credits);
    RPU_SetCoinLockout(false);
  }
  PlaySoundEffect(SOUND_EFFECT_MACHINE_START);
  SetPlayerLamps(CurrentNumPlayers);

  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}

//================================================================
void AddCoinToAudit(byte switchHit) {

  unsigned short coinAuditStartByte = 0;

  switch (switchHit) {
    case SW_COIN_3: coinAuditStartByte = RPU_CHUTE_3_COINS_START_BYTE; break;
    case SW_COIN_2: coinAuditStartByte = RPU_CHUTE_2_COINS_START_BYTE; break;
    case SW_COIN_1: coinAuditStartByte = RPU_CHUTE_1_COINS_START_BYTE; break;
  }

  if (coinAuditStartByte) {
    RPU_WriteULToEEProm(coinAuditStartByte, RPU_ReadULFromEEProm(coinAuditStartByte) + 1);
  }
}

boolean GetLeftOutlane(byte playerNum) {
  return (LeftOutlane&(1<<playerNum))?true:false;
}

void SetLeftOutlane(byte playerNum) {
  LeftOutlane |= (1<<playerNum);
}
boolean GetRightOutlane(byte playerNum) {
  return (RightOutlane&(1<<playerNum))?true:false;
}

void SetRightOutlane(byte playerNum) {
  RightOutlane |= (1<<playerNum);
}

#define ADJ_TYPE_LIST                 1
#define ADJ_TYPE_MIN_MAX              2
#define ADJ_TYPE_MIN_MAX_DEFAULT      3
#define ADJ_TYPE_SCORE                4
#define ADJ_TYPE_SCORE_WITH_DEFAULT   5
#define ADJ_TYPE_SCORE_NO_DEFAULT     6
byte AdjustmentType = 0;
byte NumAdjustmentValues = 0;
byte AdjustmentValues[8];
unsigned long AdjustmentScore;
byte *CurrentAdjustmentByte = NULL;
unsigned long *CurrentAdjustmentUL = NULL;
byte CurrentAdjustmentStorageByte = 0;
byte TempValue = 0;


int RunSelfTest(int curState, boolean curStateChanged) {
  int returnState = curState;
  CurrentNumPlayers = 0;

/*
  if (curStateChanged) {
    if (DEBUG_MESSAGES) {
      Serial.write("State changed in Self Test Mode\n\r");
    }
  }
*/

  
#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (curStateChanged) {
    // Send a stop-all command and reset the sample-rate offset, in case we have
    //  reset while the WAV Trigger was already playing.
    wTrig.stopAllTracks();
    wTrig.samplerateOffset(0);
  }
#endif

  // Any state that's greater than CHUTE_3 is handled by the Base Self-test code
  // Any that's less, is machine specific, so we handle it here.
  if (curState >= MACHINE_STATE_TEST_CHUTE_3_COINS) {
    returnState = RunBaseSelfTest(returnState, curStateChanged, CurrentTime, SW_CREDIT_RESET, SW_SLAM);
  } else {
    byte curSwitch = RPU_PullFirstFromSwitchStack();

    if (curSwitch == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      SetLastSelfTestChangedTime(CurrentTime);
      returnState -= 1;
    }

    if (curSwitch == SW_SLAM) {
      returnState = MACHINE_STATE_ATTRACT;
    }

    if (curStateChanged) {

      for (int count = 0; count < 4; count++) {
        RPU_SetDisplay(count, 0);
        RPU_SetDisplayBlank(count, 0x00);
      }
      RPU_SetDisplayCredits(MACHINE_STATE_TEST_SOUNDS - curState);
      RPU_SetDisplayBallInPlay(0, false);
      CurrentAdjustmentByte = NULL;
      CurrentAdjustmentUL = NULL;
      CurrentAdjustmentStorageByte = 0;

      AdjustmentType = ADJ_TYPE_MIN_MAX;
      AdjustmentValues[0] = 0;
      AdjustmentValues[1] = 1;
      TempValue = 0;

      switch (curState) {
        case MACHINE_STATE_ADJUST_FREEPLAY:
          CurrentAdjustmentByte = (byte *)&FreePlayMode;
          CurrentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_BALL_SAVE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 5;
          AdjustmentValues[1] = 6;
          AdjustmentValues[2] = 11;
          AdjustmentValues[3] = 16;
          AdjustmentValues[4] = 21;
          CurrentAdjustmentByte = &BallSaveNumSeconds;
          CurrentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_MUSIC_LEVEL:
          AdjustmentType = ADJ_TYPE_MIN_MAX_DEFAULT;
            #if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
              AdjustmentValues[1] = 5;
              #else
              AdjustmentValues[1] = 3;
            #endif
          CurrentAdjustmentByte = &MusicLevel;
          CurrentAdjustmentStorageByte = EEPROM_MUSIC_LEVEL_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_TOURNAMENT_SCORING:
          CurrentAdjustmentByte = (byte *)&TournamentScoring;
          CurrentAdjustmentStorageByte = EEPROM_TOURNAMENT_SCORING_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_REBOOT:
          for (byte count = 0; count < 4; count++) {
            RPU_SetDisplay(count, 8007, true);
          }
          CurrentAdjustmentByte = 0;
          break;
        
        case MACHINE_STATE_ADJUST_SKILL_SHOT_AWARD:
          CurrentAdjustmentByte = (byte *)&SkillShotAwardsLevel;
          CurrentAdjustmentStorageByte = EEPROM_SKILL_SHOT_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_TILT_WARNING:
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &MaxTiltWarnings;
          CurrentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_AWARD_OVERRIDE:
          AdjustmentType = ADJ_TYPE_MIN_MAX_DEFAULT;
          AdjustmentValues[1] = 7;
          CurrentAdjustmentByte = &ScoreAwardReplay;
          CurrentAdjustmentStorageByte = EEPROM_AWARD_OVERRIDE_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_BALLS_OVERRIDE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 3;
          AdjustmentValues[0] = 3;
          AdjustmentValues[1] = 5;
          AdjustmentValues[2] = 99;
          CurrentAdjustmentByte = &BallsPerGame;
          CurrentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        
        case MACHINE_STATE_ADJUST_SCROLLING_SCORES:
          CurrentAdjustmentByte = (byte *)&ScrollingScores;
          CurrentAdjustmentStorageByte = EEPROM_SCROLLING_SCORES_BYTE;
          break;

        case MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &ExtraBallValue;
          CurrentAdjustmentStorageByte = EEPROM_EXTRA_BALL_SCORE_BYTE;
          break;

        case MACHINE_STATE_ADJUST_SPECIAL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &SpecialValue;
          CurrentAdjustmentStorageByte = EEPROM_SPECIAL_SCORE_BYTE;
          break;

        case MACHINE_STATE_ADJUST_PLAYFIELD_VALID:
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &PlayfieldValidation;
          CurrentAdjustmentStorageByte = EEPROM_PLAYFIELD_VALID_BYTE;
          break;

        case MACHINE_STATE_ADJUST_WIZARD_DURATION:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 5;
          AdjustmentValues[1] = 15;
          AdjustmentValues[2] = 30;
          AdjustmentValues[3] = 45;
          AdjustmentValues[4] = 60;
          CurrentAdjustmentByte = &WizardModeTimeLimit;
          CurrentAdjustmentStorageByte = EEPROM_WIZARD_DURATION_BYTE;
          break;

        case MACHINE_STATE_ADJUST_WIZARD_REWARD:
          AdjustmentType = ADJ_TYPE_SCORE_NO_DEFAULT;
          CurrentAdjustmentUL = &WizardSwitchReward;
          CurrentAdjustmentStorageByte = EEPROM_WIZARD_REWARD_BYTE;
          break;

        case MACHINE_STATE_ADJUST_DIM_LEVEL:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 2;
          AdjustmentValues[0] = 2;
          AdjustmentValues[1] = 3;
          CurrentAdjustmentByte = &DimLevel;
          CurrentAdjustmentStorageByte = EEPROM_DIM_LEVEL_BYTE;
          for (int count = 0; count < 10; count++) RPU_SetLampState(BONUS_1 + count, 1, 1);
          break;

        case MACHINE_STATE_ADJUST_DONE:
          returnState = MACHINE_STATE_ATTRACT;
          break;
      }

    }

    // Change value, if the switch is hit
    if (curSwitch == SW_CREDIT_RESET) {

      if (CurrentAdjustmentByte && (AdjustmentType == ADJ_TYPE_MIN_MAX || AdjustmentType == ADJ_TYPE_MIN_MAX_DEFAULT)) {
        byte curVal = *CurrentAdjustmentByte;
        curVal += 1;
        if (curVal > AdjustmentValues[1]) {
          if (AdjustmentType == ADJ_TYPE_MIN_MAX) curVal = AdjustmentValues[0];
          else {
            if (curVal > 99) curVal = AdjustmentValues[0];
            else curVal = 99;
          }
        }
        *CurrentAdjustmentByte = curVal;
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, curVal);
      } else if (CurrentAdjustmentByte && AdjustmentType == ADJ_TYPE_LIST) {
        byte valCount = 0;
        byte curVal = *CurrentAdjustmentByte;
        byte newIndex = 0;
        for (valCount = 0; valCount < (NumAdjustmentValues - 1); valCount++) {
          if (curVal == AdjustmentValues[valCount]) newIndex = valCount + 1;
        }
        *CurrentAdjustmentByte = AdjustmentValues[newIndex];
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, AdjustmentValues[newIndex]);
      } else if (CurrentAdjustmentUL && (AdjustmentType == ADJ_TYPE_SCORE_WITH_DEFAULT || AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT)) {
        unsigned long curVal = *CurrentAdjustmentUL;
        curVal += 5000;
        if (curVal > 100000) curVal = 0;
        if (AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT && curVal == 0) curVal = 5000;
        *CurrentAdjustmentUL = curVal;
        if (CurrentAdjustmentStorageByte) RPU_WriteULToEEProm(CurrentAdjustmentStorageByte, curVal);
      }

      if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
        RPU_SetDimDivisor(1, DimLevel);
      }
      if (curState == MACHINE_STATE_ADJUST_REBOOT) {
        returnState = MACHINE_STATE_ATTRACT;
      }
    }

    // Show current value
    if (CurrentAdjustmentByte != NULL) {
      RPU_SetDisplay(0, (unsigned long)(*CurrentAdjustmentByte), true);
    } else if (CurrentAdjustmentUL != NULL) {
      RPU_SetDisplay(0, (*CurrentAdjustmentUL), true);
    }

  }

  if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
    for (int count = 0; count < 10; count++) RPU_SetLampState(BONUS_1 + count, 1, (CurrentTime / 1000) % 2);
  }

  if (returnState == MACHINE_STATE_ATTRACT) {
    // If any variables have been set to non-override (99), return
    // them to dip switch settings
    // Balls Per Game, Player Loses On Ties, Novelty Scoring, Award Score
    DecodeDIPSwitchParameters();
    ReadStoredParameters();
  }

  return returnState;
}




////////////////////////////////////////////////////////////////////////////
//
//  Audio Output functions
//
////////////////////////////////////////////////////////////////////////////

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
byte CurrentBackgroundSong = SOUND_EFFECT_NONE;
#endif


void PlayBackgroundSong(byte songNum) {

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (MusicLevel > 4) {
    if (CurrentBackgroundSong != songNum) {
      if (CurrentBackgroundSong != SOUND_EFFECT_NONE) wTrig.trackStop(CurrentBackgroundSong);
      if (songNum != SOUND_EFFECT_NONE) {
#ifdef USE_WAV_TRIGGER_1p3
        wTrig.trackPlayPoly(songNum, true);
#else
        wTrig.trackPlayPoly(songNum);
#endif
        wTrig.trackLoop(songNum, true);
      }
      CurrentBackgroundSong = songNum;
    }
  }
#else
  byte test = songNum;
  songNum = test;
#endif

}

unsigned long NextSoundEffectTime = 0;

void PlaySoundEffect(byte soundEffectNum) {

    if (MusicLevel == 0) return;

#if defined(USE_WAV_TRIGGER) || defined(USE_WAV_TRIGGER_1p3)
  if (MusicLevel > 3) {

#ifndef USE_WAV_TRIGGER_1p3
    if (  soundEffectNum == SOUND_EFFECT_BUMPER_HIT || soundEffectNum == SOUND_EFFECT_ROLLOVER ||
          soundEffectNum == SOUND_EFFECT_10PT_SWITCH || SOUND_EFFECT_SPINNER_HIGH ||
          SOUND_EFFECT_SPINNER_LOW ) wTrig.trackStop(soundEffectNum);
#endif
    wTrig.trackPlayPoly(soundEffectNum);
  }
#endif


//===========================================================
// Chime Loader
//===========================================================
#ifdef USE_CHIMES
  // If the user selects electronic sounds, don't do chimes
  if (!GameReady) return;
  if (MusicLevel>3) return;

  // Music level 3 = allow melodies to overlap
  if (CurrentTime>NextSoundEffectTime || MusicLevel==3) {
    NextSoundEffectTime = CurrentTime;
  } else if ( (NextSoundEffectTime-CurrentTime)>1000 ) {
    // if we already have one second of sound effects
    // lined up, simply return
    return;
  }
  int count = 0;

  unsigned long soundGapUL = (unsigned long)CHIME_SPACING_CONSTANT;

  byte longestGap = 0;

  // Look for chimes that need to be added based on the current sound effect
  int arrayCount;
  int arraySize;
  ChimeEntry *chimeArray;

  longestGap = 0;

  for (arrayCount=0; arrayCount<(2+MusicLevel*2); arrayCount++) {
    switch (arrayCount) {
      case 0: chimeArray = MataHariSFXLowPriorityLevel1; arraySize = sizeof(MataHariSFXLowPriorityLevel1)/sizeof(ChimeEntry); break;
      case 1: chimeArray = MataHariSFXHighPriorityLevel1; arraySize = sizeof(MataHariSFXHighPriorityLevel1)/sizeof(ChimeEntry); break;
      case 2: chimeArray = MataHariSFXLowPriorityLevel2; arraySize = sizeof(MataHariSFXLowPriorityLevel2)/sizeof(ChimeEntry); break;
      case 3: chimeArray = MataHariSFXHighPriorityLevel2; arraySize = sizeof(MataHariSFXHighPriorityLevel2)/sizeof(ChimeEntry); break;
      default: chimeArray = NULL;
    }
    bool solenoidOverride = (arrayCount%2)?true:false;
    if (chimeArray) {
      for (count=0; count<arraySize; count++) {
        if (chimeArray[count].SoundEffectNum==soundEffectNum) {
         // if (arrayCount<2)
         // {
         //   RPU_PushToTimedSolenoidStack(chimeArray[count].SolNumber, 3, CurrentTime, solenoidOverride);
         // } 
         // else
          {
            RPU_PushToTimedSolenoidStack(chimeArray[count].SolNumber, 3, NextSoundEffectTime + soundGapUL*((unsigned long)chimeArray[count].TimeOffset), solenoidOverride);
          }
          if (chimeArray[count].TimeOffset > longestGap) longestGap = chimeArray[count].TimeOffset;
        }
      }
    }
  }
  NextSoundEffectTime += (3 + (unsigned long)longestGap)*soundGapUL;  
#endif   
}


//================================================================
void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
    RPU_SetDisplayCredits(Credits);
    RPU_SetCoinLockout(false);
  } else {
    RPU_SetDisplayCredits(Credits);
    RPU_SetCoinLockout(true);
  }
  RPU_SetLampState(APRON_CREDIT, (Credits || FreePlayMode));
}


////////////////////////////////////////////////////////////////////////////
//
//  Attract Mode
//
////////////////////////////////////////////////////////////////////////////

byte AttractSweepLights = 1;
unsigned long AttractLastSweepTime = 0;
unsigned long AttractLastLadderTime = 0;
byte AttractLastLadderBonus = 0;
unsigned long AttractLastStarTime = 0;
byte AttractLastHeadMode = 255;
byte AttractLastPlayfieldMode = 255;
unsigned long SaucerLastKickTime = 0;  // === Safety timer for the top saucer ball eject ===
unsigned long LastSaucerScoreTime = 0; // === Tracks the last valid saucer hit for debounce ===
boolean StartButtonWasHeld = false;    // Tracks start button state


int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {
    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();
    RPU_SetDisableFlippers(true);
/*
    if (DEBUG_MESSAGES) {
      Serial.write("Entering Attract Mode\n\r");
    }
*/
    RPU_SetLampState(APRON_CREDIT, (Credits || FreePlayMode));
    AttractLastHeadMode = 0;
    AttractLastPlayfieldMode = 0;
  }

// Attract Sound - once every 5 minutes - uncomment to activate
//static unsigned long lastPlayerUpSound = 0;
//if (CurrentTime - lastPlayerUpSound >= 300000) {  // 5 minutes = 300,000 ms
//  lastPlayerUpSound = CurrentTime;
//  PlaySoundEffect(SOUND_EFFECT_ADD_PLAYER);
//}

//==================================================
// Attract head display logic
//==================================================
boolean showHighScore = false;

if (CurrentTime >= 6000) {
  showHighScore = ((CurrentTime - 6000) % 20000) < 10000;
}

if (showHighScore) {
  // High Score phase
  RPU_SetLampState(HIGH_SCORE_TO_DATE, 1, 0, 250);
  RPU_SetLampState(GAME_OVER, 0);
  SetPlayerLamps(0);

  if (AttractLastHeadMode != 1) {
    RPU_SetDisplayCredits(Credits, true);
    RPU_SetDisplayBallInPlay(0, true);
    AttractLastHeadMode = 1;
  }

  ShowPlayerScores(0xFF, false, false, HighScore);

} else {
  // Current scores phase (initial 6 s + later 10 s windows)
  RPU_SetLampState(HIGH_SCORE_TO_DATE, 0);
  RPU_SetLampState(GAME_OVER, 1);
  SetPlayerLamps(((CurrentTime / 250) % 4) + 1);

  if (AttractLastHeadMode != 2) {
    if (ResetScoresToClearVersion) {
      for (byte count = 0; count < 4; count++) CurrentScores[count] = 0;
      CurrentNumPlayers = 0;
      ResetScoresToClearVersion = false;
    }
    RPU_SetDisplayCredits(Credits, true);
    RPU_SetDisplayBallInPlay(0, true);
    AttractLastHeadMode = 2;
  }

  ShowPlayerScores(0xFF, false, false);
}

  // ==========================================================================
  // 2. PLAYFIELD LAMP ANIMATION STACKS
  // ==========================================================================
  if ((CurrentTime / 10000) % 2 == 0) {
    // --- STAGE 1: VERTICAL SWEEP (Geometry-Aligned Vertical Wave) ---
    if (AttractLastPlayfieldMode != 1) {
      RPU_TurnOffAllLamps(); 
      RPU_SetLampState(APRON_CREDIT, (Credits || FreePlayMode));
      AttractLastSweepTime = CurrentTime;
      AttractSweepLights = 0; 
    }
    AttractLastPlayfieldMode = 1;

    // 80ms transition speed
    if ((CurrentTime - AttractLastSweepTime) > 80) {
      AttractSweepLights++;
      if (AttractSweepLights > 13) AttractSweepLights = 0; 
      AttractLastSweepTime = CurrentTime;
    }

    // ==========================================================================
    // SMOOTH: State 1 = Active. Dim 1 = Low intensity trailing halo glow.
    // ==========================================================================
    
    // Lower Vertical Ladder Climbing Steps (0-5)
    RPU_SetLampState(BONUS_1, (AttractSweepLights >= 0 && AttractSweepLights <= 1),  (AttractSweepLights != 0));
    RPU_SetLampState(BONUS_2, (AttractSweepLights >= 0 && AttractSweepLights <= 2),  (AttractSweepLights != 1));
    RPU_SetLampState(BONUS_3, (AttractSweepLights >= 1 && AttractSweepLights <= 3),  (AttractSweepLights != 2));
    RPU_SetLampState(BONUS_4, (AttractSweepLights >= 2 && AttractSweepLights <= 4),  (AttractSweepLights != 3));
    RPU_SetLampState(BONUS_5, (AttractSweepLights >= 3 && AttractSweepLights <= 5),  (AttractSweepLights != 4));
    RPU_SetLampState(BONUS_6, (AttractSweepLights >= 4 && AttractSweepLights <= 6),  (AttractSweepLights != 5));

    // Step 6: Flanking 3X and Shoot Again line up alongside Bonus 7
    boolean step6Active = (AttractSweepLights >= 5 && AttractSweepLights <= 7);
    RPU_SetLampState(BONUS_7,     step6Active, (AttractSweepLights != 6));
    RPU_SetLampState(BONUS_3X,    step6Active, (AttractSweepLights != 6));
    RPU_SetLampState(SHOOT_AGAIN, step6Active, (AttractSweepLights != 6));

    // Step 7: Flanking 2X/5X and both Outlanes line up alongside Bonus 8
    boolean step7Active = (AttractSweepLights >= 6 && AttractSweepLights <= 8);
    RPU_SetLampState(BONUS_8,          step7Active, (AttractSweepLights != 7));
    RPU_SetLampState(BONUS_2X,         step7Active, (AttractSweepLights != 7));
    RPU_SetLampState(BONUS_5X,         step7Active, (AttractSweepLights != 7));
    RPU_SetLampState(LEFT_OUTLANE_50,  step7Active, (AttractSweepLights != 7));
    RPU_SetLampState(RIGHT_OUTLANE_50, step7Active, (AttractSweepLights != 7));

    // Steps 8-9: Top of column splitting to "T" branches
    RPU_SetLampState(BONUS_9,  (AttractSweepLights >= 7 && AttractSweepLights <= 9),   (AttractSweepLights != 8));
    RPU_SetLampState(BONUS_10, (AttractSweepLights >= 8 && AttractSweepLights <= 10),  (AttractSweepLights != 9));
    RPU_SetLampState(BONUS_20, (AttractSweepLights >= 8 && AttractSweepLights <= 10),  (AttractSweepLights != 9));

    // Step 10: Horizontal Middle AB line lights completely
    boolean step10Active = (AttractSweepLights >= 9 && AttractSweepLights <= 11);
    RPU_SetLampState(AB_SCORES_1000,    step10Active, (AttractSweepLights != 10));
    RPU_SetLampState(AB_SCORES_2000,    step10Active, (AttractSweepLights != 10));
    RPU_SetLampState(AB_SCORES_3000,    step10Active, (AttractSweepLights != 10));
    RPU_SetLampState(AB_SCORES_4000,    step10Active, (AttractSweepLights != 10));
    RPU_SetLampState(AB_SCORES_5000,    step10Active, (AttractSweepLights != 10));
    RPU_SetLampState(AB_SCORES_EB,      step10Active, (AttractSweepLights != 10));
    RPU_SetLampState(AB_SCORES_SPECIAL, step10Active, (AttractSweepLights != 10));

    // Step 11: A & B Lane pairs and Target Scores Special horizontal bridge
    boolean step11Active = (AttractSweepLights >= 10 && AttractSweepLights <= 12);
    RPU_SetLampState(LAST_TARGET_SCORES_SPECIAL, step11Active, (AttractSweepLights != 11));
    RPU_SetLampState(A_LANE,                     step11Active, (AttractSweepLights != 11));
    RPU_SetLampState(B_LANE,                     step11Active, (AttractSweepLights != 11));

    // Step 12: Upper Pop Bumper zone and 2X Potential flash
    boolean step12Active = (AttractSweepLights >= 11 && AttractSweepLights <= 13);
    RPU_SetLampState(POP_BUMPER_1,       step12Active, (AttractSweepLights != 12));
    RPU_SetLampState(POP_BUMPER_2,       step12Active, (AttractSweepLights != 12));
    RPU_SetLampState(BONUS_2X_POTENTIAL, step12Active, (AttractSweepLights != 12));

    // Step 13: 3X and 5X Potential apex flash
    RPU_SetLampState(BONUS_3X_POTENTIAL, (AttractSweepLights == 12 || AttractSweepLights == 13), (AttractSweepLights != 13));
    RPU_SetLampState(BONUS_5X_POTENTIAL, (AttractSweepLights == 12 || AttractSweepLights == 13), (AttractSweepLights != 13));


  } else {
    // --- STAGE 2: THE CROSS-FADING GRID MATRIX ---
    if (AttractLastPlayfieldMode != 2) {
      RPU_TurnOffAllLamps();
      RPU_SetLampState(APRON_CREDIT, (Credits || FreePlayMode));
      AttractLastLadderTime = CurrentTime;
    }
    AttractLastPlayfieldMode = 2;

    if ((CurrentTime - AttractLastLadderTime) > 350) {
      AttractLastLadderBonus = !AttractLastLadderBonus; 
      AttractLastLadderTime = CurrentTime;
    }

    boolean gridA = (AttractLastLadderBonus == 1);
    RPU_SetLampState(BONUS_1, gridA);
    RPU_SetLampState(BONUS_3, gridA);
    RPU_SetLampState(BONUS_5, gridA);
    RPU_SetLampState(BONUS_7, gridA);
    RPU_SetLampState(BONUS_9, gridA);
    RPU_SetLampState(BONUS_20, gridA);
    RPU_SetLampState(LEFT_OUTLANE_50, gridA);
    RPU_SetLampState(RIGHT_OUTLANE_50, gridA);
    RPU_SetLampState(BONUS_3X_POTENTIAL, gridA);
    RPU_SetLampState(LAST_TARGET_SCORES_SPECIAL, gridA);
    RPU_SetLampState(A_LANE,           gridA);
    RPU_SetLampState(BONUS_2X,         gridA);
    RPU_SetLampState(SHOOT_AGAIN,      gridA);
    
    RPU_SetLampState(AB_SCORES_1000,   gridA);
    RPU_SetLampState(AB_SCORES_3000,   gridA);
    RPU_SetLampState(AB_SCORES_5000,   gridA);
    RPU_SetLampState(AB_SCORES_SPECIAL,gridA);

    boolean gridB = (AttractLastLadderBonus == 0);
    RPU_SetLampState(BONUS_2, gridB);
    RPU_SetLampState(BONUS_4, gridB);
    RPU_SetLampState(BONUS_6, gridB);
    RPU_SetLampState(BONUS_8, gridB);
    RPU_SetLampState(BONUS_10, gridB);
    RPU_SetLampState(POP_BUMPER_1, gridB);
    RPU_SetLampState(POP_BUMPER_2, gridB);
    RPU_SetLampState(BONUS_2X_POTENTIAL, gridB);
    RPU_SetLampState(BONUS_5X_POTENTIAL, gridB);
    RPU_SetLampState(B_LANE,           gridB);
    RPU_SetLampState(BONUS_3X,         gridB);
    RPU_SetLampState(BONUS_5X,         gridB);
    
    RPU_SetLampState(AB_SCORES_2000,   gridB);
    RPU_SetLampState(AB_SCORES_4000,   gridB);
    RPU_SetLampState(AB_SCORES_EB,     gridB);
  }

  byte switchHit;
  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    if (switchHit == SW_CREDIT_RESET) {
      //if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
      if (AddPlayer(true)) returnState = MACHINE_STATE_WAIT_FOR_BALL;
    }
    if (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3) {
      AddCoinToAudit(switchHit);
      AddCredit(true, 1);
    }
    if (switchHit == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      returnState = MACHINE_STATE_TEST_LAMPS;
      SetLastSelfTestChangedTime(CurrentTime);
    }
  }

  return returnState;
}


////////////////////////////////////////////////////////////////////////////
//
//  Game Play functions
//
////////////////////////////////////////////////////////////////////////////

int RunWaitForBallMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  if (curStateChanged) {
    RPU_TurnOffAllLamps();
    // If we are entering this state, queue an immediate kick 
    // to clear out any ball that might be resting here on boot.
    RPU_PushToSolenoidStack(SOL_SAUCER, 5, true);
    // Set the safety window baseline to the current time so it won't fire again immediately
    SaucerLastKickTime = CurrentTime; 
  }
  //Clear Saucer Safely (with a 2-second delay between kicks)
  if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
    if (CurrentTime - SaucerLastKickTime > 2000) {
      RPU_PushToSolenoidStack(SOL_SAUCER, 5, true);
      SaucerLastKickTime = CurrentTime; // Reset the safety window
    } 
  }

   // Check if ball is missing from the outhole
  if (!RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    // Ball missing: Flash warning lamps
    if ((CurrentTime / 400) % 2 == 0) {
      RPU_SetLampState(BALL_IN_PLAY, 1);
      RPU_SetLampState(TILT, 1);
    } else {
      RPU_SetLampState(BALL_IN_PLAY, 0);
      RPU_SetLampState(TILT, 0);
    }
    
  // Play sound if start button is pressed while waiting for the ball
  if (RPU_ReadSingleSwitchState(SW_CREDIT_RESET)) {
    if (!StartButtonWasHeld) {
      PlaySoundEffect(SOUND_EFFECT_ADD_PLAYER);
      StartButtonWasHeld = true; // Mark as held so it doesn't repeat
    }
  } else {
    StartButtonWasHeld = false;   // Reset when the player lets go
  }
    
    // Service physical switch stack so coin door buttons still work while waiting
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY) {
      if (switchHit == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
        returnState = MACHINE_STATE_TEST_LAMPS;
        SetLastSelfTestChangedTime(CurrentTime);
      }
    }
  } else {
    // Ball is detected! Clean up warning lamps and advance to initialization
    RPU_SetLampState(BALL_IN_PLAY, 0);
    RPU_SetLampState(TILT, 0);
    returnState = MACHINE_STATE_INIT_GAMEPLAY; 
  } 
  return returnState;
}


//=======================================================
int InitGamePlay() {
/*
  if (DEBUG_MESSAGES) {
    Serial.write("Starting game\n\r");
  }
*/
  // The start button has been hit only once to get
  // us into this mode, so we assume a 1-player game
  // at the moment
  RPU_EnableSolenoidStack();
  RPU_SetDisableFlippers(true);        // Keep flippers OFF until real start
  RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);
  RPU_TurnOffAllLamps();
  GameReady = false;

  // Turn back on all lamps that are needed
  SetPlayerLamps(1);
  RPU_SetLampState(APRON_CREDIT, (Credits || FreePlayMode));

  // When we go back to attract mode, there will be no need to reset scores
  ResetScoresToClearVersion = false;
  SamePlayerShootsAgain = false;

  // Reset displays & game state variables
  for (int count = 0; count < 4; count++) {
    CurrentScores[count] = 0;

    // Initialize game-specific variables
    PopBumperGoal[count] = NUM_POP_BUMPERS_HIT_GOAL;
    ABLaneGoal[count] = NUM_ORBITS_IN_AB_GOAL;
    SlingsAndLanesGoal[count] = NUM_SLINGS_AND_INLANES;
    LeftTargetGoal[count] = NUM_LEFT_TARGETS_GOAL;
    RightTargetGoal[count] = NUM_RIGHT_TARGETS_GOAL;
    ModeCompletionStatus[count] = 0;
    BonusXPotential = InitialBonusXPotential; // Set to DIP setting
  }

  CurrentBallInPlay = 1;
  CurrentNumPlayers = 1;
  ShowPlayerScores(0xFF, false, false);
  CurrentPlayer = 0;
  LeftOutlane = 0;
  RightOutlane = 0;
  LastBHit = 0; 
  LastAHit = 0;
  LastPopBumperHit = 0;
  ScoreOverrideStatus = 0;

  // Ball is ready → Start the game
  GameReady = true;
  RPU_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, TriggeredSwitches);             
  RPU_SetDisableFlippers(false);
  RPU_SetLampState(BALL_IN_PLAY, 1);

  return MACHINE_STATE_INIT_NEW_BALL;
}

//=======================================================
int InitNewBall(bool curStateChanged, byte playerNum, int ballNum) {

  // If we're coming into this mode for the first time
  // then we have to do everything to set up the new ball
  if (curStateChanged) {
    SamePlayerShootsAgain = false;
    BallFirstSwitchHitTime = 0;
    DropTargetsScoreSpecial = false;

    RPU_SetDisableFlippers(false);
    RPU_EnableSolenoidStack();
    RPU_SetDisplayCredits(Credits, true);
    SetPlayerLamps(playerNum + 1, 4);

    RPU_SetDisplayBallInPlay(ballNum);
    RPU_SetLampState(BALL_IN_PLAY, 1);
    RPU_SetLampState(TILT, 0);

    Bonus = 0;
    BonusX = 1;
    BonusXPotential = InitialBonusXPotential; // Set to DIP initial setting
    BallSaveUsed = false;
    SkillShotRunning = true;
    BallTimeInTrough = 0;
    NumTiltWarnings = 0;
    LastTiltWarningTime = 0;
    PopBumperPhase = 0;

    // Initialize game-specific start-of-ball lights & variables
    GameMode = GAME_MODE_SKILL_SHOT;
    GameModeStartTime = CurrentTime;
    GameModeEndTime = 0;
    LastModeShotTime = 0;
    ABLaneState = 0x11;
    ABMaxedOut[CurrentPlayer] = false;
    leftBumperLit = false;  // Reset to unlit
    rightBumperLit = false; // Reset to unlit
    LeftDropsSweptCount = 0;
    Full8DropsSweptCount = 0;
    LastTargetScoresSpecial = false;
    
    // Start appropriate mode music
    PlaySoundEffect(SOUND_EFFECT_PLAYER_UP);

    if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
      RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100);
    }

    // Reset drop targets
    RPU_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 20);
    RPU_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 150);
    ResetLeftDropTargetStatusTime = CurrentTime + 250;
    ResetRightDropTargetStatusTime = CurrentTime + 250;
  }

  // We should only consider the ball initialized when
  // the ball is no longer triggering the SW_OUTHOLE
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    return MACHINE_STATE_INIT_NEW_BALL;
  } else {
    return MACHINE_STATE_NORMAL_GAMEPLAY;
  }

}

//=======================================================
void AddToBonus(byte bonusAddition) {
  Bonus += bonusAddition;
  if (Bonus > MAX_DISPLAY_BONUS) Bonus = MAX_DISPLAY_BONUS;
}


int LastReportedValue = 0;
boolean PlayerUpLightBlinking = false;
// unsigned long LastTimeSlingOrLaneHit = 0;

//=======================================================
// This function manages all timers, flags, and lights
int ManageGameMode() {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  // If the playfield hasn't been validated yet, flash score and player up num
  if (BallFirstSwitchHitTime == 0) {
    if (!PlayerUpLightBlinking) {
      SetPlayerLamps((CurrentPlayer + 1), 4, 250);
      PlayerUpLightBlinking = true;
    }
  } else {
    if (PlayerUpLightBlinking) {
      SetPlayerLamps((CurrentPlayer + 1), 4);
      PlayerUpLightBlinking = false;
    }
  }

  if (ResetLeftDropTargetStatusTime && CurrentTime>ResetLeftDropTargetStatusTime) {
    LeftDropTargetStatus = CheckSequentialSwitches(SW_LEFT_DROP_TARGET_4, 4);
//    LeftDTBankHitOrder = 0;
    ResetLeftDropTargetStatusTime = 0;
  }

  if (ResetRightDropTargetStatusTime && CurrentTime>ResetRightDropTargetStatusTime) {
    RightDropTargetStatus = CheckSequentialSwitches(SW_RIGHT_DROP_TARGET_4, 4);
//    RightDTBankHitOrder = 0;
    ResetRightDropTargetStatusTime = 0;
  }

  //=======================================================
  // GAME MODE TRANSITIONS 
  //=======================================================
  switch (GameMode) {
    
    case GAME_MODE_SKILL_SHOT:
      if (GameModeEndTime == 0) {
        // First time we're in this mode, initialize the timer
        GameModeEndTime = GameModeStartTime + (1000 * SKILL_SHOT_DURATION);
      }
      if (BallFirstSwitchHitTime != 0 || CurrentTime > GameModeEndTime) {
        // Something has been hit, OR the timer expired, so exit skill shot
        GameMode = GAME_MODE_QUALIFY_SELECT;
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
      }
    break;

    case GAME_MODE_QUALIFY_SELECT:
      // --- FLOW DEFINITION: Triggered ONLY after the 5th mode completes ---
      if (ModeCompletionStatus[CurrentPlayer] == 0x1F) {
        ProspectiveGameMode = GAME_MODE_WIZARD; // Sets the preview pointer to Wizard!
        
      }
      // To get from qualify to select, the player has to hit both A&B within 10 seconds
      if (LastAHit && LastBHit && ((CurrentTime-LastAHit)/1000)<AB_TIME_TO_QUALIFY_MODE && ((CurrentTime-LastBHit)/1000)<AB_TIME_TO_QUALIFY_MODE) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        GameMode = GAME_MODE_SELECT_MODE;
      }
    break;

    case GAME_MODE_SELECT_MODE:
      if (GameModeEndTime==0) {
        // This mode doesn't have an end
        GameModeEndTime = CurrentTime;
        ProspectiveGameMode = GetNextUnfinishedMode(GAME_MODE_AB_LANES-1);
      }
    break;

    case GAME_MODE_AB_LANES:
      if (GameModeEndTime==0) {
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;
        // LastABHitTime = CurrentTime; 
        LastABReportedValue = -1; 
      }
      { int abGoalsLeft = ABLaneGoal[CurrentPlayer];
        if (abGoalsLeft >= 0) {
          if (LastABReportedValue != abGoalsLeft) {
            // Push the fresh remaining target number to the display once
            RPU_SetDisplayCredits(abGoalsLeft, true, true); 
            CurrentlyShowingABHits = true;
            LastABReportedValue = abGoalsLeft;
          } else {
            // Tell the system to actively hold and flash/refresh this temporary override text
            RPU_SetDisplayFlashCredits(CurrentTime, 100); 
          }
        }
          CurrentlyShowingABHits = true;
        }

      if (CurrentTime>GameModeEndTime || !ABLaneGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;

        CurrentlyShowingABHits = false;
        LastABReportedValue = -1;

        if (ABLaneGoal[CurrentPlayer]<=(NUM_ORBITS_IN_AB_GOAL/2)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_AB_LANES;
          PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
          CurrentScores[CurrentPlayer] += 5000;
        }
         else if (CurrentTime > GameModeEndTime) {
          PlaySoundEffect(SOUND_EFFECT_TIMEOUT); // Fires the timeout failure chime
        }
        RPU_SetDisplayCredits(Credits, true, true);
        // --- Clear AB Status
        LastAHit = 0;
        LastBHit = 0;
      }
    break;

    case GAME_MODE_LEFT_DROP_TARGETS:
      if (GameModeEndTime==0) {
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;        
        RPU_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 100);
        LastABReportedValue = -1;
      }
      {
        int leftDropsLeft = LeftTargetGoal[CurrentPlayer];
        if (leftDropsLeft >= 0) {
          if (LastABReportedValue != leftDropsLeft) {
            RPU_SetDisplayCredits(leftDropsLeft, true, true); 
            LastABReportedValue = leftDropsLeft;
          } else {
            RPU_SetDisplayFlashCredits(CurrentTime, 100); 
          }
        }
      }
      if (CurrentTime>GameModeEndTime || !LeftTargetGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;

        if (LeftTargetGoal[CurrentPlayer]<=(NUM_LEFT_TARGETS_GOAL/2)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_LEFT_DROPS;
          PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
          CurrentScores[CurrentPlayer] += 5000;
        }
        else if (CurrentTime > GameModeEndTime) {
          PlaySoundEffect(SOUND_EFFECT_TIMEOUT); // Fires the timeout failure chime
        }
        // --- RESET DROPS AT END OF MODE ---
        RPU_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 100);
        RPU_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 100 + 150); // Safe 150ms stagger
        LeftDropTargetStatus = 0;   // Reset internal memory registers
        RightDropTargetStatus = 0;
        RPU_SetDisplayCredits(Credits, true, true);
        // --- Clear AB Status
        LastAHit = 0;
        LastBHit = 0; 
      }
    break;

    case GAME_MODE_RIGHT_DROP_TARGETS:
      if (GameModeEndTime==0) {
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;        
        RPU_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 100);
        LastABReportedValue = -1;
      }
      {
        int rightDropsLeft = RightTargetGoal[CurrentPlayer];
        if (rightDropsLeft >= 0) {
          if (LastABReportedValue != rightDropsLeft) {
            RPU_SetDisplayCredits(rightDropsLeft, true, true); 
            LastABReportedValue = rightDropsLeft;
          } else {
            RPU_SetDisplayFlashCredits(CurrentTime, 100); 
          }
        }
      }
      if (CurrentTime>GameModeEndTime || !RightTargetGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;
        if (RightTargetGoal[CurrentPlayer]<=(NUM_RIGHT_TARGETS_GOAL/2)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_RIGHT_DROPS;
          PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
          CurrentScores[CurrentPlayer] += 5000;
        }
        else if (CurrentTime > GameModeEndTime) {
          PlaySoundEffect(SOUND_EFFECT_TIMEOUT); // Fires the timeout failure chime
        }
        // --- RESET DROPS AT END OF MODE ---
        RPU_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 100);
        RPU_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 100 + 150); // Safe 150ms stagger
        LeftDropTargetStatus = 0;   // Reset internal memory registers
        RightDropTargetStatus = 0;
        RPU_SetDisplayCredits(Credits, true, true);
        // --- Clear AB Status
        LastAHit = 0;
        LastBHit = 0; 
      }
    break;

    case GAME_MODE_POP_BUMPERS:
      if (GameModeEndTime==0) {
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;
        LastABReportedValue = -1;
      }
      {
        int bumpersLeft = PopBumperGoal[CurrentPlayer];
        if (bumpersLeft >= 0) {
          if (LastABReportedValue != bumpersLeft) {
            RPU_SetDisplayCredits(bumpersLeft, true, true); 
            LastABReportedValue = bumpersLeft;
          } else {
            RPU_SetDisplayFlashCredits(CurrentTime, 100); 
          }
        }
      }
      if (CurrentTime>GameModeEndTime || !PopBumperGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;
        if (PopBumperGoal[CurrentPlayer]==0) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_POP_BUMPERS;
          PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
          CurrentScores[CurrentPlayer] += 5000;
        }
        else if (CurrentTime > GameModeEndTime) {
          PlaySoundEffect(SOUND_EFFECT_TIMEOUT); // Fires the timeout failure chime
        }
        RPU_SetDisplayCredits(Credits, true, true);
        // --- Clear AB Status
        LastAHit = 0;
        LastBHit = 0;
      }
    break;

    case GAME_MODE_SLINGS_AND_LANES:
      if (GameModeEndTime==0) {
        // First time we're in this mode
        GameModeEndTime = GameModeStartTime + 1000*MODE_LENGTH_IN_SECONDS;
        LastABReportedValue = -1;
      }
      {
        int slingsLeft = SlingsAndLanesGoal[CurrentPlayer];
        if (slingsLeft >= 0) {
          if (LastABReportedValue != slingsLeft) {
            RPU_SetDisplayCredits(slingsLeft, true, true); 
            LastABReportedValue = slingsLeft;
          } else {
            RPU_SetDisplayFlashCredits(CurrentTime, 100); 
          }
        }
      }
      if (CurrentTime>GameModeEndTime || !SlingsAndLanesGoal[CurrentPlayer]) {
        ShowPlayerScores(0xFF, false, false);
        GameMode = GAME_MODE_QUALIFY_SELECT;

        if (SlingsAndLanesGoal[CurrentPlayer]<=(NUM_SLINGS_AND_INLANES/2)) {
          ModeCompletionStatus[CurrentPlayer] |= MODE_STATUS_BIT_SLINGS_AND_LANES;
          PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
          CurrentScores[CurrentPlayer] += 5000;
        }
        else if (CurrentTime > GameModeEndTime) {
          PlaySoundEffect(SOUND_EFFECT_TIMEOUT); // Fires the timeout failure chime
        }
        // --- Clear AB Status
        RPU_SetDisplayCredits(Credits, true, true);
        LastAHit = 0;
        LastBHit = 0;
      }
    break;

    case GAME_MODE_WIZARD:
    static long lastSecondChecked;
      if (GameModeEndTime == 0) {
        // 1. INITIALIZATION: Runs once when WIZARD Mode begins
        // Scales your service menu setting variable (WizardModeTimeLimit) to milliseconds
        GameModeEndTime = CurrentTime + (1000 * WizardModeTimeLimit);
        
        MachineState = MACHINE_STATE_WIZARD_MODE; 
        PlaySoundEffect(SOUND_EFFECT_MACHINE_START); // NEED TO MAKE A SPECIAL SOUND FOR THIS
        lastSecondChecked = WizardModeTimeLimit; 
      }

      // 2. LIVE COUNTDOWN LOGIC
      long secondsLeft = (GameModeEndTime - CurrentTime) / 1000;
      if (secondsLeft >= 0) {

        for (byte i = 0; i < 4; i++) {
          if (i != CurrentPlayer) {
            OverrideScoreDisplay(i, secondsLeft, false);
          }
        }
        // Every 1 second (1000ms) pacing chime 
        if (secondsLeft != lastSecondChecked) {
          if (secondsLeft < WizardModeTimeLimit-3) {
            PlaySoundEffect(SOUND_EFFECT_WIZARD_TIMER);
          }
          lastSecondChecked = secondsLeft; 
        }
      }

      // 3. CLEANUP & TIMER EXPIRATION
      if (CurrentTime > GameModeEndTime) {
        // Clear all completion bits so the checklist wipes completely for a fresh game loop
        ModeCompletionStatus[CurrentPlayer] = 0x00;
        
        // RESET: Re-prime all target countdowns for the ACTIVE player only!
        PopBumperGoal[CurrentPlayer]      = NUM_POP_BUMPERS_HIT_GOAL;
        ABLaneGoal[CurrentPlayer]         = NUM_ORBITS_IN_AB_GOAL;
        SlingsAndLanesGoal[CurrentPlayer] = NUM_SLINGS_AND_INLANES;
        LeftTargetGoal[CurrentPlayer]     = NUM_LEFT_TARGETS_GOAL;
        RightTargetGoal[CurrentPlayer]    = NUM_RIGHT_TARGETS_GOAL;

        // Reset drop states, orbital history, and lane memories
 //       LastAHit = 0;
 //       LastBHit = 0;

        // Drop the cabinet cleanly back down to standard baseline operations
        ShowPlayerScores(0xFF, false, false); 
        GameMode = GAME_MODE_QUALIFY_SELECT;
        MachineState = MACHINE_STATE_NORMAL_GAMEPLAY;
        PlaySoundEffect(SOUND_EFFECT_TIMEOUT);
      }
    break;
  }
  //=======================================================


  //=======================================================
  // LAMP CYCLE
  //=======================================================
  ShowABLamps(GameMode, ProspectiveGameMode, ABLaneState);
  ShowSamePlayerLamps(GameMode, ProspectiveGameMode);
  ShowBonusLights(GameMode, ProspectiveGameMode, Bonus);
  ShowBonusXLights(GameMode, ProspectiveGameMode, BonusX, LastTimeSlingOrLaneHit);
  ShowOutlanes(GameMode, ProspectiveGameMode, GetLeftOutlane(CurrentPlayer), GetRightOutlane(CurrentPlayer), LastTimeSlingOrLaneHit);
  ShowSaucerLamps(GameMode);
  ShowABRewardLamps(GameMode, ProspectiveGameMode, ABLaneState);
  ShowPopBumperLamps(GameMode, ProspectiveGameMode, 0, LastPopBumperHit);
  ShowDropTargetSpecialLamp(GameMode, LastTargetScoresSpecial);
  //=======================================================



  //=======================================================
  // OUTHOLE PROCESSING
  //=======================================================
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    if (BallTimeInTrough == 0) {
      BallTimeInTrough = CurrentTime;
    } else {
      // Make sure the ball stays on the sensor for at least
      // 0.5 seconds to be sure that it's not bouncing
      if ((CurrentTime - BallTimeInTrough) > 500) {

        // --- WIZARD MODE BALL SAVER ---
        if (GameMode == GAME_MODE_WIZARD) {
          WizardSwitchHit(); // Score bonus points for hitting the drain trough!
          RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100); // Kick it back out!
          BallFirstSwitchHitTime = 0; 
          BallTimeInTrough = 0;       
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY; // Keep gameplay rolling!
        }

        if (BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings) {
          // Nothing hit yet, so return the ball to the player
          RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {
          // if we haven't used the ball save, and we're under the time limit, then save the ball
          if (  !BallSaveUsed &&
                ((CurrentTime - BallFirstSwitchHitTime) / 1000) < ((unsigned long)BallSaveNumSeconds) ) {

            RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100);
            if (BallFirstSwitchHitTime > 0) {
              BallSaveUsed = true;
            }
            BallTimeInTrough = CurrentTime;

            returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
          } else {
            ShowPlayerScores(0xFF, false, false);
            PlayBackgroundSong(SOUND_EFFECT_NONE);
            returnState = MACHINE_STATE_COUNTDOWN_BONUS;
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }
  return returnState;
}
//=======================================================


//=======================================================
// BONUS COUNTDOWN
//=======================================================
unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;

int CountdownBonus(boolean curStateChanged) {

  // If this is the first time through the countdown loop
  if (curStateChanged) {
    RPU_SetLampState(BALL_IN_PLAY, 1, 0, 250);

    CountdownStartTime = CurrentTime;
    ShowBonusOnTree(Bonus);

    LastCountdownReportTime = CountdownStartTime;
    BonusCountDownEndTime = 0xFFFFFFFF;
  }

  if ((CurrentTime - LastCountdownReportTime) > 250) {

    if (Bonus > 0) {

      // Only give sound & score if this isn't a tilt
      if (NumTiltWarnings <= MaxTiltWarnings) {
        CurrentScores[CurrentPlayer] += ((unsigned long)BonusX)*1000;
          if (BonusX == 5) {
              PlaySoundEffect(SOUND_EFFECT_5X_BONUS_COUNT); 
            } else if (BonusX == 3) {
              PlaySoundEffect(SOUND_EFFECT_3X_BONUS_COUNT); 
            } else if (BonusX == 2) {
              PlaySoundEffect(SOUND_EFFECT_2X_BONUS_COUNT); 
            } else {
              PlaySoundEffect(SOUND_EFFECT_BONUS_COUNT);   
            }
      }
      Bonus -= 1;
      ShowBonusOnTree(Bonus);
    } else if (BonusCountDownEndTime == 0xFFFFFFFF) {
      PlaySoundEffect(SOUND_EFFECT_BALL_OVER);
      RPU_SetLampState(BONUS_1, 0);
      BonusCountDownEndTime = CurrentTime + 1000;
    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime > BonusCountDownEndTime) {

  // Reset any lights & variables of goals that weren't completed

    BonusCountDownEndTime = 0xFFFFFFFF;
    return MACHINE_STATE_BALL_OVER;
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}


//=======================================================
void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    if (CurrentScores[count] > highestScore) highestScore = CurrentScores[count];
    highScorePlayerNum = count;
  }

  if (highestScore > HighScore) {
    HighScore = highestScore;
    if (HighScoreReplay) {
      AddCredit(false, 3);
      RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 3);
    }
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < 4; count++) {
      if (count == highScorePlayerNum) {
        RPU_SetDisplay(count, CurrentScores[count], true, 2);
      } else {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 300, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 600, true);
  }
}



//=======================================================
// MATCH SEQUENCE
unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 1500;
    //MatchDigit = random(0, 10);
    MatchDigit = CurrentTime%10;
    NumMatchSpins = 0;
    RPU_SetLampState(MATCH, 1, 0);
    RPU_SetDisableFlippers();
    ScoreMatches = 0;
    RPU_SetLampState(BALL_IN_PLAY, 0);
  }

  if (NumMatchSpins < 40) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      PlaySoundEffect(SOUND_EFFECT_MATCH_SPIN);
      RPU_SetDisplayBallInPlay((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      RPU_SetLampState(MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        RPU_SetLampState(MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddCredit(false, 1);
        RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
        RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
        MatchDelay += 1000;
        NumMatchSpins += 1;
        RPU_SetLampState(MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < 4; count++) {
    if ((ScoreMatches >> count) & 0x01) {
      // If this score matches, we're going to flash the last two digits
      if ( (CurrentTime / 200) % 2 ) {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & 0x0F);
      } else {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | 0x30);
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}

//=======================================================
// MODE ENGINE
byte GetNextUnfinishedMode(byte startMode) {
  byte nextMode;
  if (ModeCompletionStatus[CurrentPlayer]==0x1F) return GAME_MODE_WIZARD;

  for (nextMode=startMode+1; nextMode!=startMode; nextMode++) {
    if (nextMode>GAME_MODE_SLINGS_AND_LANES) nextMode = GAME_MODE_AB_LANES;

    byte bitCheck = (0x01<<(nextMode-GAME_MODE_AB_LANES));
    if (!(ModeCompletionStatus[CurrentPlayer] & bitCheck)) break;
  }
  if (nextMode==startMode) return GAME_MODE_WIZARD;

  return nextMode;
}

//============================================================
//New AddABLaneScore to more closely mimic original game rules
void AddABLaneScore() {
  byte aNibble = ABLaneState & 0x0F;
  byte bNibble = (ABLaneState & 0xF0)>>4;
  
  // RULE 1: If they don't match, it's just a single lane hit.
  // Award 500 points and exit.
  if (aNibble != bNibble) {
    CurrentScores[CurrentPlayer] += 500;
    PlaySoundEffect(SOUND_EFFECT_AB_LANE_1);
    return; 
  }

  // RULE 2: They match! Both lanes are complete.
  // Subtract 1 to look back at the scoring tier they just completed.
  byte completedTier = aNibble - 1;

  // Determine which lane was hit FIRST by comparing existing timestamps.
  // The smaller timestamp means that lane was struck first!
  if (LastAHit < LastBHit) {
    // Lane A was hit FIRST, Lane B completed the set.
    // Completion 1 lights Left Bumper, Completion 2 adds Right Bumper.
    if (completedTier >= 1) leftBumperLit = true;
    if (completedTier >= 2) rightBumperLit = true;
  } else {
    // Lane B was hit FIRST, Lane A completed the set.
    // Completion 1 lights Right Bumper, Completion 2 adds Left Bumper.
    if (completedTier >= 1) rightBumperLit = true;
    if (completedTier >= 2) leftBumperLit = true;
  }
  
  switch (completedTier) {
    case 1:
      CurrentScores[CurrentPlayer] += 1000;
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
    break;
    case 2:
      CurrentScores[CurrentPlayer] += 2000;
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
    break;
    case 3:
      CurrentScores[CurrentPlayer] += 3000;
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
    break;
    case 4:
      CurrentScores[CurrentPlayer] += 4000;
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
    break;
    case 5: 
      CurrentScores[CurrentPlayer] += 5000;
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_2);
      // If the player has already beaten Special, trap them in an endless 5K loop
      // Resetting the memory back to 0x55 here ensures they stay in case 5 forever!
      if (ABMaxedOut[CurrentPlayer]) {
        ABLaneState = 0x55;
      }
        
    break;
    case 6: // Extra Ball lamp
      SamePlayerShootsAgain = true;
      PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL);
    break;
    case 7: // Special lamp
      AddCredit(true, 1);
      RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
      PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
      ABMaxedOut[CurrentPlayer] = true; // Mark that this player completed the ladder!
      ABLaneState = 0x55;

    break;
    default:
      CurrentScores[CurrentPlayer] += 500; // Safety fallback
      PlaySoundEffect(SOUND_EFFECT_AB_LANE_1);

       ABLaneState = 0x55;
  }
}


void AddABLaneState(boolean bLaneHit) {
  
  if (GameMode != GAME_MODE_QUALIFY_SELECT && GameMode != GAME_MODE_SELECT_MODE && GameMode != GAME_MODE_SKILL_SHOT) {
  return; // Only collect AB loop bonus accumulations during normal play. Exit the function instantly without changing the lane nibbles if in a mode!
  }
  
  byte aNibble = ABLaneState & 0x0F;
  byte bNibble = (ABLaneState & 0xF0)>>4;

  if (!bLaneHit) {
    if (aNibble<=bNibble) aNibble += 1;  
  } else {
    if (bNibble<=aNibble) bNibble += 1;
  }
  if (aNibble>15) aNibble = 15;
  if (bNibble>15) bNibble = 15;
  ABLaneState = (bNibble<<4) | aNibble;
}

byte CheckSequentialSwitches(byte startingSwitch, byte numSwitches) {
  byte returnSwitches = 0; 
  for (byte count=0; count<numSwitches; count++) {
    returnSwitches |= (RPU_ReadSingleSwitchState(startingSwitch+count)<<count);
  }
  return returnSwitches;
}

//=======================================================
void HandleLeftDropTargetHit(byte switchHit) {

  byte currentStatus = CheckSequentialSwitches(SW_LEFT_DROP_TARGET_4, 4);  
  boolean frenzyReset = false;
  boolean awardGiven = false;
  boolean soundPlayed = false;

  byte targetBit = (1<<(switchHit-SW_LEFT_DROP_TARGET_4));
  // If this is a legit switch hit (not a repeat)
  if ( (targetBit & LeftDropTargetStatus)==0 ) {

/*
    if (LeftDTBankHitOrder==0 && ((currentStatus&0x32)==0x20)) {
      LeftDTBankHitOrder = 1;
    } else if (LeftDTBankHitOrder==1) {
      if ((currentStatus&0x32)==0x22) LeftDTBankHitOrder = 2;
      else LeftDTBankHitOrder = 0;
    } else if (LeftDTBankHitOrder==2 && switchHit==SW_DROP_TARGET_3) {
      StartScoreAnimation(15000);
      PlaySoundEffect(SOUND_EFFECT_DROP_SEQUENCE_SKILL);
      soundPlayed = true;
      awardGiven = true;
      LeftDTBankHitOrder = 0;
    }
*/

/*
    if (GameMode&GAME_MODE_FLAG_DROP_TARGET_FRENZY) {
      PlaySoundEffect(SOUND_EFFECT_7K_BONUS);
      soundPlayed = true;
      StartScoreAnimation(7000);
      frenzyReset = true;
    }
*/    

    // Default scoring for a drop target
    if (!awardGiven) {
      CurrentScores[CurrentPlayer] += 500;
    }

    LeftDropTargetStatus |= targetBit;
  }

  // If targets need to be reset
  boolean bankDown = false;

  // Rule A: If we are running inside either dedicated drop target mission mode, 
  // allow the left bank to reset on its own isolated 4-target completion.
  if (GameMode == GAME_MODE_LEFT_DROP_TARGETS || GameMode == GAME_MODE_RIGHT_DROP_TARGETS) {
    bankDown = (currentStatus == 0x0F);
  } 

  // Rule B: Normal Gameplay. Both left and right banks must be completely down (0x0F) 
  // to trigger a dual reset.
  else {
    boolean leftBankIsDown = (currentStatus == 0x0F);
    boolean rightBankIsDown = (RightDropTargetStatus == 0x0F); // Cross-check right bitmask state
    
    if (leftBankIsDown && rightBankIsDown) {
      bankDown = true;
    }
  }

  if (bankDown || frenzyReset) {
    if (ResetLeftDropTargetStatusTime==0) {
      unsigned long extraDelay = 0;
      if (frenzyReset) {
        extraDelay = 2000;
      } else {

 // --- 8-TARGET SWEEP REWARDS FOR NORMAL PLAY (LEFT SIDE) ---
      if (GameMode != GAME_MODE_LEFT_DROP_TARGETS && GameMode != GAME_MODE_RIGHT_DROP_TARGETS) {
        Full8DropsSweptCount += 1;
        soundPlayed = true; // Blocks basic target chimes from repeating
        
        if (Full8DropsSweptCount == 1) {
          // 1st Complete Sweep: Award 50,000 points and prime Special light rule
          CurrentScores[CurrentPlayer] += 50000;
          PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL);
          LastTargetScoresSpecial = true; 
        } 
        else if (Full8DropsSweptCount == 2) {
          // 2nd+ Complete Sweep: Check for Tournament Constraints
          if (TournamentScoring) {
            // TOURNAMENT MODE ACTIVE: Converts the Special reward into an additional 50,000 points
            CurrentScores[CurrentPlayer] += 50000;
            PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL);
          } else {
            // CASUAL MODE ACTIVE: Collect Special Reward (Loud knocker coil fire and free game credit)
            AddCredit(true, 1);
            RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
            PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
          }
          LastTargetScoresSpecial = false; 
        }
        else {
          PlaySoundEffect(SOUND_EFFECT_SKILL_SHOT); 
        }
      }
      }
      RPU_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 500 + extraDelay);
      if (GameMode != GAME_MODE_LEFT_DROP_TARGETS && GameMode != GAME_MODE_RIGHT_DROP_TARGETS) {
        RPU_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 500 + 150 + extraDelay);
        // Reset the right tracking register back to 0 so the right switch file stays in sync
        RightDropTargetStatus = 0; 
      }
      ResetLeftDropTargetStatusTime = CurrentTime + 850 + extraDelay;
    }
  }
  if (!soundPlayed) {
    PlaySoundEffect(SOUND_EFFECT_DROP_TARGET);
  }

}

//========================================================
void HandleRightDropTargetHit(byte switchHit) {

  byte currentStatus = CheckSequentialSwitches(SW_RIGHT_DROP_TARGET_4, 4);  
  boolean frenzyReset = false;
  boolean awardGiven = false;
  boolean soundPlayed = false;

  byte targetBit = (1<<(switchHit-SW_RIGHT_DROP_TARGET_4));
  // If this is a legit switch hit (not a repeat)
  if ( (targetBit & RightDropTargetStatus)==0 ) {

    // Default scoring for a drop target
    if (!awardGiven) {
      CurrentScores[CurrentPlayer] += 500;
    }

    RightDropTargetStatus |= targetBit;
  }
  // If targets need to be reset
  boolean bankDown = false;

  // Rule A: If running inside either dedicated drop target mission mode, 
  // allow the right bank to reset on its own isolated 4-target completion.
  if (GameMode == GAME_MODE_LEFT_DROP_TARGETS || GameMode == GAME_MODE_RIGHT_DROP_TARGETS) {
    bankDown = (currentStatus == 0x0F);
  } 

  // Rule B: Normal Gameplay. Both left and right banks must be completely down (0x0F) 
  // to trigger a dual reset.
  else {
    boolean rightBankIsDown = (currentStatus == 0x0F);
    boolean leftBankIsDown = (LeftDropTargetStatus == 0x0F); // Cross-check left bitmask state
    
    if (leftBankIsDown && rightBankIsDown) {
      bankDown = true;
    }
  }

  // If targets need to be reset
  if (bankDown || frenzyReset) {
    if (ResetRightDropTargetStatusTime==0) {
      unsigned long extraDelay = 0;
      if (frenzyReset) {
        extraDelay = 2000;
      } else {

// --- 8-TARGET SWEEP REWARDS FOR NORMAL PLAY (RIGHT SIDE) ---
      if (GameMode != GAME_MODE_LEFT_DROP_TARGETS && GameMode != GAME_MODE_RIGHT_DROP_TARGETS) {
        Full8DropsSweptCount += 1;
        soundPlayed = true; // Blocks basic target chimes from repeating
        
        if (Full8DropsSweptCount == 1) {
          // 1st Complete Sweep: Award 50,000 points and prime Special light rule
          CurrentScores[CurrentPlayer] += 50000;
          PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL);
          LastTargetScoresSpecial = true; 
        } 
        else if (Full8DropsSweptCount == 2) {
          // 2nd Complete Sweep: Check for Tournament Constraints
          if (TournamentScoring) {
            // TOURNAMENT MODE ACTIVE: Converts the Special reward into an additional 50,000 points
            CurrentScores[CurrentPlayer] += 50000;
            PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL);
          } else {
            // CASUAL MODE ACTIVE: Collect Special Reward (Loud knocker coil fire and free game credit)
            AddCredit(true, 1);
            RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
            PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
          }
          LastTargetScoresSpecial = false; 
        }
        else {
          PlaySoundEffect(SOUND_EFFECT_SKILL_SHOT); 
        }
      }
      }
      RPU_PushToTimedSolenoidStack(SOL_RIGHT_DROP_TARGETS, 15, CurrentTime + 500 + extraDelay);
      // SAFE STAGGER FIX: Shift the Left Bank reset coil execution window by an extra 150ms
      // Spreads out the power spike safely
      if (GameMode != GAME_MODE_LEFT_DROP_TARGETS && GameMode != GAME_MODE_RIGHT_DROP_TARGETS) {
        RPU_PushToTimedSolenoidStack(SOL_LEFT_DROP_TARGETS, 15, CurrentTime + 500 + 150 + extraDelay);
        // Reset the left tracking register back to 0 so the left switch file stays in sync
        LeftDropTargetStatus = 0; 
      }

      ResetRightDropTargetStatusTime = CurrentTime + 850 + extraDelay;
    }
  }

  if (!soundPlayed) {
    PlaySoundEffect(SOUND_EFFECT_DROP_TARGET);
  }
}



void WizardSwitchHit() {
  CurrentScores[CurrentPlayer] += WizardSwitchReward;
  PlaySoundEffect(SOUND_EFFECT_WIZARD_SCORE);
}

//============================================================
int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
//  byte bonusAtTop = Bonus;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];
/*
  if (curStateChanged) {
    if (DEBUG_MESSAGES) {
      Serial.write("State changed in Game Play Mode\n\r");
    }
  }
*/
  // Very first time into gameplay loop
  if (curState == MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay();
  } else if (curState == MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged, CurrentPlayer, CurrentBallInPlay);
  } else if (curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = ManageGameMode();  
  } else if (curState == MACHINE_STATE_COUNTDOWN_BONUS) {
    returnState = CountdownBonus(curStateChanged);
  } else if (curState == MACHINE_STATE_BALL_OVER) {
    if (SamePlayerShootsAgain) {
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {
      CurrentPlayer += 1;
      if (CurrentPlayer >= CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay += 1;
      }

      if (CurrentBallInPlay > BallsPerGame) {
        CheckHighScores();
        PlaySoundEffect(SOUND_EFFECT_GAME_OVER);
        SetPlayerLamps(0);
        for (int count = 0; count < 4; count++) {
          if (count<CurrentNumPlayers) RPU_SetDisplay(count, CurrentScores[count], true, 2);
          else RPU_SetDisplayBlank(count, 0x00);
        }

        returnState = MACHINE_STATE_MATCH_MODE;
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState == MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);
  }

  byte switchHit;

  if (NumTiltWarnings <= MaxTiltWarnings) {
    while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
      
      switch (switchHit) {
        case SW_SLAM:
            RPU_DisableSolenoidStack();
            RPU_SetDisableFlippers(true);
            RPU_TurnOffAllLamps();
            RPU_SetLampState(GAME_OVER, 1);
            delay(1000);
            return MACHINE_STATE_ATTRACT;
        break;
        case SW_TILT:
          // This should be debounced
          if ((CurrentTime - LastTiltWarningTime) > TILT_WARNING_DEBOUNCE_TIME) {
            LastTiltWarningTime = CurrentTime;
            NumTiltWarnings += 1;
            if (NumTiltWarnings > MaxTiltWarnings) {
              RPU_DisableSolenoidStack();
              RPU_SetDisableFlippers(true);
              RPU_TurnOffAllLamps();
              RPU_SetLampState(TILT, 1);
            }
            PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
          }
        break;
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LAMPS;
          SetLastSelfTestChangedTime(CurrentTime);
        break;

        case SW_LEFT_INLANE:
        case SW_RIGHT_INLANE:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;

          // 1. Wizard Mode
          if (GameMode == GAME_MODE_WIZARD) {
            WizardSwitchHit();
          } 
          // 2. Active Slings & Lanes Mode
          else if (GameMode == GAME_MODE_SLINGS_AND_LANES) {
            CurrentScores[CurrentPlayer] += 3000; // Aligned 3,000-point mission reward!
            PlaySoundEffect(SOUND_EFFECT_BIG_INLANE); // 
            
            // Handle Countdown Objective Tracking & Display Update
            if (SlingsAndLanesGoal[CurrentPlayer]) {
              SlingsAndLanesGoal[CurrentPlayer] -= 1;
              LastModeShotTime = CurrentTime;
            }
          } 
          // 3. Standard operational gameplay fallback (500 Points)
          else {
            CurrentScores[CurrentPlayer] += 500;
            PlaySoundEffect(SOUND_EFFECT_INLANE);
          }

          LastTimeSlingOrLaneHit = CurrentTime;
          AddToBonus(1);
        break;

        case SW_LEFT_A_LANE:
        case SW_TOP_A_LANE:
          LastAHit = CurrentTime;
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          
          // 1. ACTIVE MODE GATE: If a specialized mode is active, handle scoring independently
          if (GameMode != GAME_MODE_QUALIFY_SELECT && GameMode != GAME_MODE_SELECT_MODE && GameMode != GAME_MODE_SKILL_SHOT) {

            // If the active mode is specifically the AB Lanes mission, score high mission points
            if (GameMode == GAME_MODE_AB_LANES) {
              CurrentScores[CurrentPlayer] += 3000; // Escalated mission reward
              PlaySoundEffect(SOUND_EFFECT_AB_LANE_3);
              
              if (ABLaneGoal[CurrentPlayer]) {
                ABLaneGoal[CurrentPlayer] -= 1;
                LastModeShotTime = CurrentTime;
              }
            } 
            // For any other mode (like Drop Targets or Pop Bumpers), just award a basic 500-point hit
            else {
              CurrentScores[CurrentPlayer] += 500;
              PlaySoundEffect(SOUND_EFFECT_AB_LANE_1);
            }
            
          } 
          // 2. STANDARD GAMEPLAY: Only run your progressive math functions when in normal play
          else {
            AddABLaneState(false);
            AddABLaneScore();
          }
          AddToBonus(1);
        break;

        case SW_TOP_B_LANE:
        case SW_RIGHT_B_LANE:
          LastBHit = CurrentTime;
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          
          // 1. ACTIVE MODE GATE: If a specialized mode is active, handle scoring independently
          if (GameMode != GAME_MODE_QUALIFY_SELECT && GameMode != GAME_MODE_SELECT_MODE && GameMode != GAME_MODE_SKILL_SHOT) {
            
            if (GameMode == GAME_MODE_AB_LANES) {
              CurrentScores[CurrentPlayer] += 3000; 
              PlaySoundEffect(SOUND_EFFECT_AB_LANE_3);
              if (ABLaneGoal[CurrentPlayer]) {
                ABLaneGoal[CurrentPlayer] -= 1;
                LastModeShotTime = CurrentTime;
              }
            } 
            else {
              CurrentScores[CurrentPlayer] += 500;
              PlaySoundEffect(SOUND_EFFECT_AB_LANE_1);
            }     
          } 
          // 2. STANDARD GAMEPLAY: Only run progressive math functions when in normal play
          else {
            AddABLaneState(true);
            AddABLaneScore();
          }
          AddToBonus(1);
        break;

        case SW_LEFT_OUTLANE:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;

          if (GameMode != GAME_MODE_WIZARD) {
            // Determine which rule applies to this physical switch right now
            boolean leftIsUpgraded = (outlanesSwapped == false) ? (BonusXPotential == 5) : (BonusX == 5);

            if (leftIsUpgraded) {
              CurrentScores[CurrentPlayer] += 50000;
              PlaySoundEffect(SOUND_EFFECT_OUTLANE_LIT); 
            } else {
              CurrentScores[CurrentPlayer] += 1000; 
              PlaySoundEffect(SOUND_EFFECT_OUTLANE_UNLIT); 
            }
          } else {
            WizardSwitchHit();
          }
          LastTimeSlingOrLaneHit = CurrentTime;
        break;

        case SW_RIGHT_OUTLANE:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;

          if (GameMode != GAME_MODE_WIZARD) {
            // Determine which rule applies to this physical switch right now
            boolean rightIsUpgraded = (outlanesSwapped == false) ? (BonusX == 5) : (BonusXPotential == 5);

            if (rightIsUpgraded) {
              CurrentScores[CurrentPlayer] += 50000;
              PlaySoundEffect(SOUND_EFFECT_OUTLANE_LIT); 
            } else {
              CurrentScores[CurrentPlayer] += 1000; 
              PlaySoundEffect(SOUND_EFFECT_OUTLANE_UNLIT); 
            }
          } else {
            WizardSwitchHit();
          }
          LastTimeSlingOrLaneHit = CurrentTime;
        break;

        case SW_10_PTS:
          if (GameMode != GAME_MODE_WIZARD) {
            CurrentScores[CurrentPlayer] += 10;
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
            PlaySoundEffect(SOUND_EFFECT_10PT_SWITCH);
          } else {
            WizardSwitchHit();
          }
          break;


        case SW_SAUCER:
          // DEBOUNCE GUARD: Ignore the hit if it occurs within 0.4 second of the last scored hit
          if ((CurrentTime - LastSaucerScoreTime) < 400) {
            break; // Exit the case immediately without scoring, playing sounds, or adding bonus
          }
          LastSaucerScoreTime = CurrentTime;

          // ------------------------------------------------------------------
          // 1. WIZARD MODE INITIATION
          // ------------------------------------------------------------------
          if (ModeCompletionStatus[CurrentPlayer] == 0x1F && GameMode != GAME_MODE_WIZARD) {
            GameMode = GAME_MODE_WIZARD;
            GameModeStartTime = CurrentTime;
            GameModeEndTime = 0; 
            RPU_PushToTimedSolenoidStack(SOL_SAUCER, 12, CurrentTime + 2000); // 2-second hold for intro
            if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
            AddToBonus(3);
            break;
          } 
          // 2. WIZARD MODE HIT and KICK
          else if (GameMode == GAME_MODE_WIZARD) {
            WizardSwitchHit(); // Scores Wizard Mode Points
            RPU_PushToTimedSolenoidStack(SOL_SAUCER, 12, CurrentTime + 200); 
            if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
            break;
          }

          // Mark this as a valid, scored hit
          LastSaucerScoreTime = CurrentTime;
          if (GameMode==GAME_MODE_SKILL_SHOT &&  SkillShotRunning == true) {
            PlaySoundEffect(SOUND_EFFECT_SKILL_SHOT);
            CurrentScores[CurrentPlayer] += (SkillShotAwardsLevel != 0) * 10000;
            RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 1000); 
          } else if (GameMode==GAME_MODE_SELECT_MODE) {
            GameMode = ProspectiveGameMode;
            if (GameMode<GAME_MODE_AB_LANES || GameMode>GAME_MODE_SLINGS_AND_LANES) GameMode = GAME_MODE_AB_LANES;
            GameModeStartTime = CurrentTime;
            GameModeEndTime = 0;
            PlaySoundEffect(SOUND_EFFECT_PLAYER_UP); // Mode Active Sound
            RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 1000); 
          } else {
/*            if (DEBUG_MESSAGES) {
              Serial.write("Generic Saucer hit\n\r");
            }
*/            
            if (BonusXPotential == 1) BonusX = 1; // Base 1X scoring
              else if (BonusXPotential == 2) BonusX = 2; // Upgrades to 2X
              else if (BonusXPotential == 3) BonusX = 3; // Upgrades to 3X
              else if (BonusXPotential == 5) BonusX = 5; // Upgrades and locks at 5X
            if (BonusXPotential == 1) BonusXPotential = 2;
              else if (BonusXPotential == 2) BonusXPotential = 3;
              else if (BonusXPotential == 3) BonusXPotential = 5;
              else                           BonusXPotential = 5;
                  PlaySoundEffect(SOUND_EFFECT_SAUCER);
                  CurrentScores[CurrentPlayer] += 3000;
                  RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 500); 
          }
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          AddToBonus(3);
        break;
        
        case SW_LEFT_DROP_TARGET_1:
        case SW_LEFT_DROP_TARGET_2:
        case SW_LEFT_DROP_TARGET_3:
        case SW_LEFT_DROP_TARGET_4:
          HandleLeftDropTargetHit(switchHit);
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (GameMode == GAME_MODE_WIZARD) CurrentScores[CurrentPlayer] += WizardSwitchReward;
          if (GameMode==GAME_MODE_LEFT_DROP_TARGETS && LeftTargetGoal[CurrentPlayer]) {
            LeftTargetGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
            AddToBonus(1);
          }
          AddToBonus(1);
        break;

        case SW_RIGHT_DROP_TARGET_1:
        case SW_RIGHT_DROP_TARGET_2:
        case SW_RIGHT_DROP_TARGET_3:
        case SW_RIGHT_DROP_TARGET_4:
          HandleRightDropTargetHit(switchHit);
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (GameMode == GAME_MODE_WIZARD) CurrentScores[CurrentPlayer] += WizardSwitchReward;
          if (GameMode==GAME_MODE_RIGHT_DROP_TARGETS && RightTargetGoal[CurrentPlayer]) {
            RightTargetGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
            AddToBonus(1);
          }
          AddToBonus(1);
        break;

        //Dual Mode PopBumpers === Bumper 1 is bottom left, 4 is bottom right
        case SW_BUMPER_1:
          LastPopBumperHit = CurrentTime;
          PopBumperPhase += 1;
          if ((PopBumperPhase % 4) == 0) {
            ProspectiveGameMode = GetNextUnfinishedMode(ProspectiveGameMode);
            GameModeStartTime = CurrentTime;
          }

          // 1. SCORING GATEWAY
          if (GameMode == GAME_MODE_WIZARD) {            
            WizardSwitchHit();
          }

          else if (GameMode == GAME_MODE_POP_BUMPERS) {
            // High point reward per hit during the dedicated mode
            CurrentScores[CurrentPlayer] += 3000; 
            PlaySoundEffect(SOUND_EFFECT_BUMPER_LIT);
            
            // Handle Countdown Objective Tracking
            if (PopBumperGoal[CurrentPlayer] > 0) {
              PopBumperGoal[CurrentPlayer] -= 1;
              LastModeShotTime = CurrentTime;
            }            
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          }

          else if (leftBumperLit) {
            CurrentScores[CurrentPlayer] += 1000;
            PlaySoundEffect(SOUND_EFFECT_BUMPER_LIT);
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          }

          else {
            CurrentScores[CurrentPlayer] += 10;
            PlaySoundEffect(SOUND_EFFECT_BUMPER_10);
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          }
        break;

        case SW_BUMPER_4:
          LastPopBumperHit = CurrentTime;
          PopBumperPhase += 1;
          if ((PopBumperPhase % 4) == 0) {
            ProspectiveGameMode = GetNextUnfinishedMode(ProspectiveGameMode);
            GameModeStartTime = CurrentTime;
          }

          // 1. SCORING GATEWAY
          if (GameMode == GAME_MODE_WIZARD) {            
            WizardSwitchHit();
          }

          else if (GameMode == GAME_MODE_POP_BUMPERS) {
            // High point reward per hit during the dedicated mode
            CurrentScores[CurrentPlayer] += 3000; 
            PlaySoundEffect(SOUND_EFFECT_BUMPER_LIT);
            
            // Handle Countdown Objective Tracking
            if (PopBumperGoal[CurrentPlayer] > 0) {
              PopBumperGoal[CurrentPlayer] -= 1;
              LastModeShotTime = CurrentTime;
            }            
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          }

          else if (rightBumperLit) {
            CurrentScores[CurrentPlayer] += 1000;
            PlaySoundEffect(SOUND_EFFECT_BUMPER_LIT);
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          }

          else {
            CurrentScores[CurrentPlayer] += 10;
            PlaySoundEffect(SOUND_EFFECT_BUMPER_10);
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          }
          break;

        // Regular PopBumpers 
        case SW_BUMPER_2:
        case SW_BUMPER_3:
       
          LastPopBumperHit = CurrentTime;
          PopBumperPhase += 1;
          if ((PopBumperPhase%4)==0) {
            ProspectiveGameMode = GetNextUnfinishedMode(ProspectiveGameMode);
            GameModeStartTime = CurrentTime;
          }
          if (GameMode != GAME_MODE_WIZARD) {
            CurrentScores[CurrentPlayer] += 100;
            PlaySoundEffect(SOUND_EFFECT_BUMPER);
            outlanesSwapped = !outlanesSwapped;
            if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          } else {
            WizardSwitchHit();
          }
          if (GameMode==GAME_MODE_POP_BUMPERS && PopBumperGoal[CurrentPlayer]) {
            PopBumperGoal[CurrentPlayer] -= 1;
            LastModeShotTime = CurrentTime;
          }
        break;

        case SW_RIGHT_SLING:
        case SW_LEFT_SLING:
          if ((CurrentTime - LastTimeSlingOrLaneHit) < 150) {
            break; // Exit the case statement immediately
          }
          // 1. Wizard Mode
          if (GameMode == GAME_MODE_WIZARD) {
            WizardSwitchHit();
          } 
          // 2. Active Slings & Lanes Mode
          else if (GameMode == GAME_MODE_SLINGS_AND_LANES) {
            CurrentScores[CurrentPlayer] += 3000; // Escalated mission reward!
            PlaySoundEffect(SOUND_EFFECT_BIG_SLING_SHOT); 
            
            // Objective Tracking & Display Update
            if (SlingsAndLanesGoal[CurrentPlayer]) {
              SlingsAndLanesGoal[CurrentPlayer] -= 1;
              LastModeShotTime = CurrentTime;
            }
          } 
          // 3. Standard operational gameplay
          else {
            CurrentScores[CurrentPlayer] += 10;
            PlaySoundEffect(SOUND_EFFECT_SLING_SHOT);
            if (leftBumperLit != rightBumperLit) {
              leftBumperLit = !leftBumperLit;
              rightBumperLit = !rightBumperLit;
            }
          }

          if (PlayfieldValidation < 2 && BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          LastTimeSlingOrLaneHit = CurrentTime;
        break;

        case SW_COIN_1:
        case SW_COIN_2:
        case SW_COIN_3:
          AddCoinToAudit(switchHit);
          AddCredit(true, 1);
        break;
        case SW_CREDIT_RESET:
          if (CurrentBallInPlay < 2) {
            // If we haven't finished the first ball, we can add players
            AddPlayer();
          } else {
            // If the first ball is over, pressing start again resets the game
            if (Credits >= 1 || FreePlayMode) {
              if (!FreePlayMode) {
                Credits -= 1;
                RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
                RPU_SetDisplayCredits(Credits);
              }
              returnState = MACHINE_STATE_INIT_GAMEPLAY;
            }
          }
/*          if (DEBUG_MESSAGES) {
            Serial.write("Start game button pressed\n\r");
          }
*/          
        break;
      }
    }
  } else {
    // We're tilted, so just wait for outhole
    while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
      switch (switchHit) {
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LAMPS;
          SetLastSelfTestChangedTime(CurrentTime);
          break;
        case SW_COIN_1:
        case SW_COIN_2:
        case SW_COIN_3:
          AddCoinToAudit(switchHit);
          AddCredit(true, 1);
          break;
      }
    }
  }
  
  if (!ScrollingScores && CurrentScores[CurrentPlayer] > 999999) {
    CurrentScores[CurrentPlayer] -= 999999;
  } else if (CurrentScores[CurrentPlayer]>999999999) {
    CurrentScores[CurrentPlayer] -= 999999999;    
  }
  
  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
  
    if (!TournamentScoring) {
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (((ScoreAwardReplay >> awardCount) & 0x01) == 0x01) {
            AddCredit(false, 1);
            RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
            RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
          } else {
            SamePlayerShootsAgain = true;
            PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL);
          }
        }
      }
    }
  
    LastTimeScoreChanged = CurrentTime;
  }
  
  ShowPlayerScores(CurrentPlayer, (BallFirstSwitchHitTime==0)?true:false, (BallFirstSwitchHitTime>0 && ((CurrentTime-LastTimeScoreChanged)>2000))?true:false);
  
  return returnState;
}


void loop() {

  RPU_DataRead(0);
  CurrentTime = millis();
  int newMachineState = MachineState;

  if (MachineState < 0) {
    newMachineState = RunSelfTest(MachineState, MachineStateChanged);
  } else if (MachineState == MACHINE_STATE_ATTRACT) {
    newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  } else if (MachineState == MACHINE_STATE_WAIT_FOR_BALL) {
    newMachineState = RunWaitForBallMode(MachineState, MachineStateChanged);
  } else {
    newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  }

  if (newMachineState != MachineState) {
    MachineState = newMachineState;
    MachineStateChanged = true;
  } else {
    MachineStateChanged = false;
  }

  RPU_ApplyFlashToLamps(CurrentTime);
  RPU_UpdateTimedSolenoidStack(CurrentTime);

} 
