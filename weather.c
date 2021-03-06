/* ************************************************************************
*  file: weather.c , Weather and time module              Part of DIKUMUD *
*  Usage: Performing the clock and the weather                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"

/* uses */

extern struct time_info_data time_info;
extern struct weather_data weather_info;

/* In this part. */

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);

/* Here comes the code */

void weather_and_time(int mode)
{
	another_hour(mode);
	if(mode)
		weather_change();
}



void another_hour(int mode)
{
	time_info.hours++;

	if (mode) {
		switch (time_info.hours) {
			case 5 :
			{
				weather_info.sunlight = SUN_RISE;
				send_to_outdoor("The sun rises in the east.\n\r");
				break;
			}
			case 6 :
			{
				weather_info.sunlight = SUN_LIGHT;
				send_to_outdoor("The day has begun.\n\r");
				break;	
			}
			case 21 :
			{
				weather_info.sunlight = SUN_SET;
				send_to_outdoor(
				"The sun slowly disappears in the west.\n\r");
				break;
			}
			case 22 :
			{
				weather_info.sunlight = SUN_DARK;
				send_to_outdoor("The night has begun.\n\r");
				break;
			}
			default : break;
		}
	}

	if (time_info.hours > 23)  /* Changed by HHS due to bug ???*/
	{
		time_info.hours -= 24;
		time_info.day++;

		if (time_info.day>34)
		{
			time_info.day = 0;
			time_info.month++;

			if(time_info.month>16)
			{
				time_info.month = 0;
				time_info.year++;
			}
		}
	}
}

void weather_change(void)
{
	int diff, change;
	if((time_info.month>=9)&&(time_info.month<=16))
		diff=(weather_info.pressure>985 ? -2 : 2);
	else
		diff=(weather_info.pressure>1015? -2 : 2);

	weather_info.change += (dice(1,4)*diff+dice(2,6)-dice(2,6));

	weather_info.chang./technoC3.readme                                                                                      644   31746     107         4324  5326500607   7172                                                                                                                                                                                                                                                                                                                                                                      technoC3.tar.Z  is  Technopolis TechMud Version C.3 (based on Dikumud)

You have here version C.3 of technopolis.  Enhancements can been seen when
you use the command 'news' when in game.

NOTE: read the README files in directories mudsource, and mudsource/lib/
IT IS IMPORTANT THAT YOU DO SO.

Graphics!  Also, terminator now hunt killers and thieves.  Also look for
hospitals the operate on you to put in chips into your head.

Since Hunt command is in high demand, I put out this release with it.

Made it so when you see multiple items in room they show up as (?) ITEM.
Removed a LOT of bad code from Version B.  Replicants are here. You can
clone people with the spell.  Added a lot of commands.

It is different from other dikumuds in that it is based on a cyberpunk
theme.  The setting is in the future, where there are many worlds, and
you must enter starships to travel to them.  Things different in it are..
a communication system based on beepers, cellular phones, Cybercomms, etc.

There are portals and teleporters you must enter to beam to starships and
beam down.  There are Cybercops and Terminators that hunt down Thieves and
Killers.  You can get operated on and substitute your body with bionic parts 
for increased strength, dexterity, freedom from thirst, etc.

Some of the command list differ from basic Diku...

call gsplit gmana sacrifice push prompt portal god newbie award gsay
pardon ban report (look up/watch) aggressive wimpy dip quiet title
godinvis assist... etc.

and some others...
(a