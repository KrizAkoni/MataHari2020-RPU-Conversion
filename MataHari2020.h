/**************************************************************************
 *     This file is part of MataHari2020.
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

    Updated by John Kriz (KrizAkoni@gmail.com) to new RPU code base.
    V0.2 13JUL2026
 */

#define NUMBER_OF_LAMPS        60

// Lamp Numbers (defines for lamps) where RPU ID = ((Decoder Output Pin times 4)+Strobe Chip Number)
#define BONUS_1                 0 //Q14
#define BONUS_2                 1 //Q29
#define BONUS_3                 2 //Q36
#define BONUS_4                 3 //Q57
#define BONUS_5                 4 //Q12
#define BONUS_6                 5 //Q27
#define BONUS_7                 6 //Q38
#define BONUS_8                 7 //Q50
#define BONUS_9                 8 //Q13
#define BONUS_10                9 //Q28
#define BONUS_20                10 //Q44
#define LAST_TARGET_SCORES_SPECIAL  11 //Q51
//#define LAMP_                 12 //Q8
//#define LAMP_                 13 //35
#define B_LANE                  14 //Q49
#define A_LANE                  15 //Q54
//#define LAMP_                 16 //Q9
//#define LAMP_                 17 //Q34
//#define LAMP_                 18 //Q48
//#define LAMP_                 19 //Q55
#define AB_SCORES_1000          20 //Q10
#define AB_SCORES_2000          21 //Q22
#define AB_SCORES_3000          22 //Q37
#define AB_SCORES_4000          23 //Q60
#define AB_SCORES_5000          24 //Q11
#define AB_SCORES_EB            25 //Q26
#define AB_SCORES_SPECIAL       26 //Q32
//#define LAMP_                 27 //Q59
//#define LAMP_                 28 //Q4
//#define LAMP_                 29 //Q25
#define RIGHT_OUTLANE_50        30 //Q20
#define LEFT_OUTLANE_50         31 //Q58
//#define LAMP_                 32 //Q1
//#define LAMP_                 33 //Q24
#define POP_BUMPER_2            34 //Q42
#define POP_BUMPER_1            35 //Q56
//#define LAMP_                 36 //Q2
#define BONUS_5X_POTENTIAL      37 //Q17
#define BONUS_3X_POTENTIAL      38 //Q41
#define BONUS_2X_POTENTIAL      39 //Q46
#define SAME_PLAYER_SHOOTS_AGAIN    40 //Q3
#define MATCH                   41 //Q23
#define SHOOT_AGAIN             42 //Q40
#define APRON_CREDIT            43 //Q52
//#define LAMP_                 44 //Q7
#define BONUS_5X                45 //Q21
#define BONUS_3X                46 //Q39
#define BONUS_2X                47 //Q53
#define BALL_IN_PLAY            48 //Q16
#define HIGH_SCORE_TO_DATE      49 //Q15
#define GAME_OVER               50 //Q33
#define TILT                    51 //Q47
#define PLAYER_1                52 //Q5
#define PLAYER_2                53 //Q18
#define PLAYER_3                54 //Q30
#define PLAYER_4                55 //Q43
#define PLAYER_1_UP             56 //Q6
#define PLAYER_2_UP             57 //Q19
#define PLAYER_3_UP             58 //Q31
#define PLAYER_4_UP             59 //Q45


#define NUM_OF_SWITCHES     27

// Defines for switches
#define SW_CREDIT_RESET   5
#define SW_TILT           6
#define SW_OUTHOLE        7
#define SW_COIN_3         8
#define SW_COIN_2         9
#define SW_COIN_1         10
#define SW_SLAM           15

#define SW_RIGHT_DROP_TARGET_4  16
#define SW_RIGHT_DROP_TARGET_3  17
#define SW_RIGHT_DROP_TARGET_2  18
#define SW_RIGHT_DROP_TARGET_1  19

#define SW_LEFT_DROP_TARGET_4   20
#define SW_LEFT_DROP_TARGET_3   21
#define SW_LEFT_DROP_TARGET_2   22
#define SW_LEFT_DROP_TARGET_1   23

#define SW_RIGHT_INLANE         24
#define SW_LEFT_INLANE          25
#define SW_10_PTS               26
#define SW_RIGHT_B_LANE         27
#define SW_LEFT_A_LANE          28
#define SW_TOP_B_LANE           29
#define SW_TOP_A_LANE           30
#define SW_SAUCER               31
#define SW_RIGHT_OUTLANE        32
#define SW_LEFT_OUTLANE         33
#define SW_RIGHT_SLING          34
#define SW_LEFT_SLING           35
#define SW_BUMPER_4             36
#define SW_BUMPER_3             38
#define SW_BUMPER_2             39
#define SW_BUMPER_1             37

// Defines for solenoids
#define SOL_SAUCER          0
#define SOL_CHIME_100       1
#define SOL_CHIME_1000      2
#define SOL_CHIME_10000     3
#define SOL_CHIME_10        4
#define SOL_KNOCKER         5
#define SOL_OUTHOLE         6
#define SOL_BUMPER_1        7
#define SOL_BUMPER_2        8
#define SOL_BUMPER_3        9
#define SOL_BUMPER_4        10
#define SOL_LEFT_SLING      11
#define SOL_LEFT_DROP_TARGETS   12
#define SOL_RIGHT_SLING     13
#define SOL_RIGHT_DROP_TARGETS  14  

// SWITCHES_WITH_TRIGGERS are for switches that will automatically
// activate a solenoid (like in the case of a chime that rings on a rollover)
// but SWITCHES_WITH_TRIGGERS are fully debounced before being activated
#define NUM_SWITCHES_WITH_TRIGGERS         6

// PRIORITY_SWITCHES_WITH_TRIGGERS are switches that trigger immediately
// (like for pop bumpers or slings) - they are not debounced completely
#define NUM_PRIORITY_SWITCHES_WITH_TRIGGERS 6

// Define automatic solenoid triggers (switch, solenoid, number of 1/120ths of a second to fire)
struct PlayfieldAndCabinetSwitch TriggeredSwitches[] = {
  { SW_LEFT_SLING, SOL_LEFT_SLING, 4 },
  { SW_RIGHT_SLING, SOL_RIGHT_SLING, 4 },
  { SW_BUMPER_1, SOL_BUMPER_1, 4 },
  { SW_BUMPER_2, SOL_BUMPER_2, 4 },
  { SW_BUMPER_3, SOL_BUMPER_3, 4 },
  { SW_BUMPER_4, SOL_BUMPER_4, 4 },
};
