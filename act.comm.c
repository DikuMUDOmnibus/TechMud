/* ************************************************************************
*  file: act.comm.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Communication.                                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;


void do_say(struct char_data *ch, char *argument, int cmd)
{
	int i;
	static char buf[MAX_STRING_LENGTH];

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
	else
	{
		sprintf(buf,"$n says '%s'", argument + i);
		act(buf,FALSE,ch,0,0,TO_ROOM);
		sprintf(buf,"You say '%s'\n\r", argument + i);
		send_to_char(buf, ch);
	}
}


void do_title(struct char_data *ch, char *argument, int cmd)
{

  char buf[MAX_STRING_LENGTH];
  if (IS_SET(ch->specials.act,PLR_THIEF)||
      IS_SET(ch->specials.act,PLR_KILLER))
    {
      send_to_char("Sorry, you are either a killer, thief, or both.\n\r",ch);
      return;
    }
	
  for (; *argument == ' '; argument++);
  
  if (!*(argument ))
    send_to_char("Yes, but WHAT do you want it to be?\n\r", ch);
  else
    {
      free(GET_TITLE(ch));
      GET_TITLE(ch)=(char *)malloc(sizeof(buf));
      sprintf(buf,"%s\0",argument);
      strcpy(GET_TITLE(ch),buf);
      
      
      send_to_char("Ok, changed\n\r", ch);
    }
}

void do_report(struct char_data *ch, char *argument, int cmd)
{

	static char buf[MAX_STRING_LENGTH];


		sprintf(buf,"$n reports %d/%d Health, %d/%d Mana, %d/%d Energy.\n\r", GET_HIT(ch),GET_MAX_HIT(ch),GET_MANA(ch),GET_MAX_MANA(ch),GET_MOVE(ch),GET_MAX_MOVE(ch));
		act(buf,FALSE,ch,0,0,TO_ROOM);
		sprintf(buf,"You report %d/%d Health, %d/%d Mana, %d/%d Energy.\n\r", GET_HIT(ch),GET_MAX_HIT(ch),GET_MANA(ch),GET_MAX_MANA(ch),GET_MOVE(ch),GET_MAX_MOVE(ch));
		send_to_char(buf, ch);

}




void do_shout(struct char_data *ch, char *argument, int cmd)
{
  static char buf1[MAX_STRING_LENGTH];
  struct descriptor_data *i;
  
  if (!IS_NPC(ch))
    {
      if (IS_SET(ch->specials.act, PLR_NOSHOUT))
	{
	  send_to_char("You can't shout!!\n\r", ch);
	  return;
	}
      

      if (GET_LEVEL(ch) < 2)
	{
	  send_to_char("You are too lowly.  Get some exp!\n\r", ch);
	  return;
	}
    }

  if (GET_MOVE(ch) < 2)
    {
      send_to_char("You are too exhausted.\n\r", ch);
      return;
    }
  
  GET_MOVE(ch) -= 1;
  
  for (; *argument == ' '; argument++);
  
  if (!(*argument))
    send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
  else
    {

      sprintf(buf1, "$n shouts '%s'", argument);
      
      for (i = descriptor_list; i; i = i->next)
      	if (i->character != ch && !i->connected &&
	    !IS_SET(i->character->specials.act, PLR_QUIET) &&
	    ((world[i->character->in_room].zone == world[ch->in_room].zone)||
	     (GET_LEVEL(i->character)>=21)|| (GET_LEVEL(ch)>=21)) )
	  act(buf1, 0, ch, 0, i->character, TO_VICT);
      
      if (GET_LEVEL(ch)>=21) sprintf(buf1,"You shout globally '%s'\n\r",argument);
      else sprintf(buf1,"You shout locally '%s'\n\r",argument);
      
      send_to_char(buf1, ch);
    }
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	char name[100], message[MAX_STRING_LENGTH],
		buf[MAX_STRING_LENGTH];

	if (IS_SET(ch->specials.act, PLR_NOTELL))
	{
		send_to_char("Your message didn't get through!!\n\r", ch);
		return;
	}

	half_chop(argument,name,message);



	if(!*name || !*message)
		send_to_char("Who do you wish to tell what??\n\r", ch);
	else if (!(vict = get_char_vis(ch, name)))
		send_to_char("You don't see anyone by that name..\n\r", ch);
	else if (ch == vict)
		send_to_char("You try to tell yourself something.\n\r", ch);
     else if ((GET_LEVEL(ch)<21)&&(GET_LEVEL(vict)<21))
     send_to_char("Being mortal, you can only mentally tell to immortals.\n\r",ch);
	else if ((IS_SET(vict->specials.act,PLR_GODINVIS))&&
		 (GET_LEVEL(ch)<GET_LEVEL(vict)))
	  {
		send_to_char("You don't see anyone by that name..\n\r", ch);
	  }
	else if ((GET_POS(vict) == POSITION_SLEEPING) ||
	         IS_SET(vict->specials.act, PLR_QUIET))
	{
		act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR);
	}
	else
	{
		sprintf(buf,"%s tells you '%s'\n\r",
		  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
		send_to_char(buf, vict);
		sprintf(buf,"You tell %s '%s'\n\r",GET_NAME(vict),message);
		send_to_char(buf, ch);

	}
}


int obj_type_in_room_vis(struct char_data *ch, int thetype)
{
  struct obj_data *theobject =0, *nextobject;
  for (theobject = world[ch->in_room].contents;theobject; theobject = nextobject)
    {
      nextobject = theobject->next_content;
      if (!CAN_SEE_OBJ(ch, theobject)) continue;
      if (CAN_WEAR(theobject,ITEM_TAKE)) continue;
      if (theobject->obj_flags.type_flag == thetype) return 1;
    }
  return 0;
  
}

int obj_type_in_carry(struct char_data *ch, int thetype)
{
  struct obj_data *theobject =0, *nextobject;
  for (theobject = ch->carrying;theobject; theobject = nextobject)
    {
      nextobject = theobject->next_content;
      if (theobject->obj_flags.type_flag == thetype) return 1;
    }
  return 0;
  
}

int obj_type_in_equipment(struct char_data *ch, int thetype)
{
  struct obj_data *theobject =0;
  int position;
  for (position=0;position<MAX_WEAR;++position)
    {
      if(!(theobject = ch->equipment[position])) continue;
      if (theobject->obj_flags.type_flag == thetype) return 1;
    }
  return 0;
  
}


void do_call(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *thephone =0;
  
  char name[100], message[MAX_STRING_LENGTH],
  buf[MAX_STRING_LENGTH];
  
  half_chop(argument,name,message);
  
  if (obj_type_in_room_vis(ch,ITEM_PHONE)==0)
    if (obj_type_in_equipment(ch,ITEM_PHONE)==0)
      if (obj_type_in_carry(ch,ITEM_PHONE)==0)
	{
	  send_to_char("No visible public communication devices in room, nor in possession.\n\r", ch);
	  return;
	}
      else
	{
	  send_to_char("Hold or wear your communication device.\n\r", ch);
	  return;
	}
  
  if(!*name)
    {
      send_to_char("Who do you wish to call??\n\r", ch);
      return;
    }
  if(!*message)
    {
      send_to_char("What do you want to say??\n\r", ch);
      return;
    }
  if (!(vict = get_char_vis(ch, name)))
    {
      send_to_char("You don't see anyone by that name..\n\r", ch);
      return;
    }
  if (ch == vict)
    {
      send_to_char("Why call yourself?\n\r", ch);
      return;
    }
  
  if ((IS_SET(vict->specials.act,PLR_GODINVIS))&&
      (GET_LEVEL(ch)<GET_LEVEL(vict)))
    {
      send_to_char("You don't see anyone by that name..\n\r", ch);
      return;
    }

  if (obj_type_in_room_vis(vict,ITEM_PHONE)==0)
    if (obj_type_in_equipment(vict,ITEM_PHONE)==0)
      if (obj_type_in_carry(vict,ITEM_PHONE)==0)
	{
	  if ((obj_type_in_carry(vict,ITEM_BEEPER)==1)||
	      (obj_type_in_equipment(vict,ITEM_BEEPER)==1))
	    {
	      if (IS_SET(vict->specials.act,PLR_QUIET))
		{
		  send_to_char("That person turned off his devices...\n\r",ch);
		  return;
		}
		
	      send_to_char("You send your number to that person's communication device...\n\r",ch);
	      sprintf(buf,"Your communication device beeps... It displays %s's number.\n\r",GET_NAME(ch));
	      send_to_char(buf,vict);
	      return;
	    }
	  send_to_char("That person doesn't have access to a communication device.\n\r",ch);
	  
	  return;
	}
      else
	{
	  if (IS_SET(vict->specials.act,PLR_QUIET))
	    {
	      send_to_char("That person turned off his devices...\n\r",ch);
	      return;
	    }
	  send_to_char("You ring that person's communication device.\n\r",ch);
	  send_to_char("Your communication device rings.\n\r",vict);
	  if ((obj_type_in_carry(vict,ITEM_BEEPER)==1)||
	      (obj_type_in_equipment(vict,ITEM_BEEPER)==1))
	    {
	      send_to_char("You also send your number to that person's other device...\n\r",ch);
	      sprintf(buf,"Another communication device beeps... It displays %s's number.\n\r",GET_NAME(ch));
	      send_to_char(buf,vict);

	    }
	  return;
	}
  if (IS_SET(vict->specials.act,PLR_QUIET))
    {
      send_to_char("That person turned off his devices...\n\r",ch);
      return;
    }

  sprintf(buf,"%s calls, '%s'\n\r",
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);

  if ((obj_type_in_carry(vict,ITEM_BEEPER)==1)||
      (obj_type_in_equipment(vict,ITEM_BEEPER)==1))
    {
      send_to_char("You send your number first to one of that person's communication device...\n\r",ch);
      sprintf(buf,"Your communication device beeps... It displays %s's number.\n\r",GET_NAME(ch));
      send_to_char(buf,vict);
      sprintf(buf,"%s also calls you, '%s'\n\r",
	      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
      
    }

  

  send_to_char(buf, vict);
  sprintf(buf,"You call %s, '%s'\n\r",GET_NAME(vict),message);
  send_to_char(buf, ch);
  
}


void do_whisper(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	char name[100], message[MAX_STRING_LENGTH],
		buf[MAX_STRING_LENGTH];

	half_chop(argument,name,message);

	if(!*name || !*message)
		send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
	else if (!(vict = get_char_room_vis(ch, name)))
		send_to_char("No-one by that name here..\n\r", ch);
	else if (vict == ch)
	{
		act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
		send_to_char(
			"You can't seem to get your mouth close enough to your ear...\n\r",
			 ch);
	}
	else
	{
		sprintf(buf,"$n whispers to you, '%s'",message);
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		send_to_char("Ok.\n\r", ch);
		act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	}
}


void do_ask(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	char name[100], message[MAX_STRING_LENGTH],
		buf[MAX_STRING_LENGTH];

	half_chop(argument,name,message);

	if(!*name || !*message)
		send_to_char("Who do you want to ask something.. and what??\n\r", ch);
	else if (!(vict = get_char_room_vis(ch, name)))
		send_to_char("No-one by that name here..\n\r", ch);
	else if (vict == ch)
	{
		act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
		send_to_char("You think about it for a while...\n\r", ch);
	}
	else
	{
		sprintf(buf,"$n asks you '%s'",message);
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		send_to_char("Ok.\n\r", ch);
		act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
	}
}


void do_gmana(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	int amount;
	char name[100],	buf[MAX_STRING_LENGTH];

	one_argument(argument,name);


if (GET_MANA(ch)<1)
  {
    send_to_char("But you have no mana to give out!\n\r",ch);
    return;
  }


	if(!*name)
		send_to_char("Who do you want to give your mana to??\n\r", ch);
	else if (!(vict = get_char_room_vis(ch, name)))
		send_to_char("No-one by that name here..\n\r", ch);
	else if (vict == ch)
	{
		act("$n tries to give $mself mana. Weird.",FALSE,ch,0,0,TO_ROOM);
		send_to_char("To yourself? It's yours already!\n\r", ch);
	}
	else if (IS_NPC(vict))
	  {
	    send_to_char("You were rejected of your offer.\n\r",ch);
	    
	  }
	else
	{
	  amount = GET_MAX_MANA(vict)-GET_MANA(vict);
	  if (GET_MANA(ch)>=amount)
	    {
	      GET_MANA(vict)+=amount;
	      GET_MANA(ch)-=amount;
	    }
	  else
	    {
	      GET_MANA(vict)+=GET_MANA(ch);
	      GET_MANA(ch)=0;
	    }

		act("You receive mana from $n!", FALSE, ch, 0, vict, TO_VICT);
		send_to_char("Ok.\n\r", ch);
		act("$n transfers mana to $N.",FALSE,ch,0,vict,TO_NOTVICT);
	  
	}
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

void do_write(struct char_data *ch, char *argument, int cmd)
{
	struct obj_data *paper = 0, *pen = 0;
	char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
		buf[MAX_STRING_LENGTH];

	argument_interpreter(argument, papername, penname);

	if (!ch->desc)
		return;

	if (!*papername)  /* nothing was delivered */
	{   
		send_to_char(
			"Write? with what? ON what? what are you trying to do??\n\r", ch);
		return;
	}
	if (*penname) /* there were two arguments */
	{
		if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
		{
			sprintf(buf, "You have no %s.\n\r", papername);
			send_to_char(buf, ch);
			return;
		}
		if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
		{
			sprintf(buf, "You have no %s.\n\r", penname);
			send_to_char(buf, ch);
			return;
		}
	}
	else  /* there was one arg.let's see what we can find */
	{			
		if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
		{
			sprintf(buf, "There is no %s in your inventory.\n\r", papername);
			send_to_char(buf, ch);
			return;
		}
		if (paper->obj_flags.type_flag == ITEM_PEN)  /* oops, a pen.. */
		{
			pen = paper;
			paper = 0;
		}
		else if (paper->obj_flags.type_flag != ITEM_NOTE)
		{
			send_to_char("That thing has nothing to do with writing.\n\r", ch);
			return;
		}

		/* one object was found. Now for the other one. */
		if (!ch->equipment[HOLD])
		{
			sprintf(buf, "You can't write with a %s alone.\n\r", papername);
			send_to_char(buf, ch);
			return;
		}
		if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD]))
		{
			send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
			return;
		}
		
		if (pen)
			paper = ch->equipment[HOLD];
		else
			pen = ch->equipment[HOLD];
	}
			
	/* ok.. now let's see what kind of stuff we've found */
	if (pen->obj_flags.type_flag != ITEM_PEN)
	{
		act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
	}
	else if (paper->obj_flags.type_flag != ITEM_NOTE)
	{
		act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
	}
	else if (paper->action_description)
		send_to_char("There's something written on it already.\n\r", ch);
	else
	{
		/* we can write - hooray! */
				
		send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r",
			ch);
		act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
		ch->desc->str = &paper->action_description;
		ch->desc->max_str = MAX_NOTE_LENGTH;
	}
}
