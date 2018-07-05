#define HUNT_FILE "hunt.table"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char wizlist[MAX_STRING_LENGTH];
extern char *dirs[]; 
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern char *room_bits[];
extern int top_of_world;

typedef struct HUNTTYPE {
  int direct;
  int dist;
} HUNTTYPE;



int direct(int x,int y)
{
  struct HUNTTYPE *direct;
  char buf[50];
  register int i;
  FILE *ptr;
  direct=malloc(sizeof(HUNTTYPE));
  if (!(ptr=fopen(HUNT_FILE,"r")))
    {
      log("Error reading HUNT_FILE");
      return;
    }
  fseek(ptr,sizeof(HUNTTYPE)*(x*(top_of_world+1))+y*sizeof(HUNTTYPE),0);
  fread(direct,sizeof(HUNTTYPE),1,ptr);
  fclose(ptr);
  i=direct->direct;
  free(direct);
  return(i);
  
}

int dist(int x,int y)
{
  struct HUNTTYPE *dist;
  char buf[50];
  register int i;
  FILE *ptr;
  dist=malloc(sizeof(HUNTTYPE));
  if (!(ptr=fopen(HUNT_FILE,"r")))
    {
      log("Error reading HUNT_FILE");
      return;
    }
  fseek(ptr,sizeof(HUNTTYPE)*(x*(top_of_world+1))+y*sizeof(HUNTTYPE),0);
  fread(dist,sizeof(HUNTTYPE),1,ptr);
  fclose(ptr);
  i=dist->dist;
  free(dist);
  return(i);
  
}


void huntstep(struct char_data *ch, int wasroom)
{
  char buf[MAX_STRING_LENGTH];
  char *exits[] =
    {	
      "North",
      "East",
      "South",
      "West",
      "Up",
      "Down"
      };
  int temp;

  if (!ch->specials.hunting)
    {
      REMOVE_BIT(ch->specials.act,PLR_HUNT);
      return;
    }

  if (dist(ch->in_room,ch->specials.hunting->in_room)==-1)
    {
if (IS_SET(ch->specials.act,PLR_GRAPHICS))
  sprintf(buf,"%c%c%c%cYou seem to have lost the scent of your victim.%c%c%c\n\r",27,91,55,109,27,91,109);
else
  sprintf(buf,"You seem to have lost the scent of your victim.\n\r");
      send_to_char(buf,ch);
      REMOVE_BIT(ch->specials.act,PLR_HUNT);
      ch->specials.hunting=NULL;
      return;
    }


  if (dist(ch->in_room,ch->specials.hunting->in_room)==0)
    {
      if (IS_SET(ch->specials.act,PLR_GRAPHICS))
      sprintf(buf,"%c%c%c%cYou have succeeded in hunting down your victim.%c%c%c\n\r",27,91,55,109,27,91,109);
else       sprintf(buf,"You have succeeded in hunting down your victim.\n\r");
      send_to_char(buf,ch);
      REMOVE_BIT(ch->specials.act,PLR_HUNT);
      ch->specials.hunting=NULL;
      return;
    }

  for (temp=0;temp<=5;++temp)
    if (world[ch->in_room].dir_option[temp])
      if (world[ch->in_room].dir_option[temp]->to_room ==
	  direct(ch->in_room,ch->specials.hunting->in_room))
	{
	  if (dist(ch->in_room,ch->specials.hunting->in_room)>
	      dist(wasroom,ch->specials.hunting->in_room))
	    {
      if (IS_SET(ch->specials.act,PLR_GRAPHICS))
      sprintf(buf,"%c%c%c%cYou have seem to have lost the scent.%c%c%c\n\r",27,91,55,109,27,91,109);
else      sprintf(buf,"You have seem to have lost the scent.\n\r");

	      REMOVE_BIT(ch->specials.act,PLR_HUNT);
	      ch->specials.hunting=NULL;
	      send_to_char(buf,ch);
	      return;
	    }
      if (IS_SET(ch->specials.act,PLR_GRAPHICS))
      sprintf(buf,"%c%c%c%cThe scent seems to be coming from direction %s.%c%c%c\n\r",27,91,55,109,exits[temp],27,91,109);
else      sprintf(buf,"The scent seems to be coming from direction %s.\n\r",exits[temp]);

	  send_to_char(buf,ch);


	  break;
	}
  return;
}

void huntmove(struct char_data *hunter, struct char_data *victim)
{

  int temp;

  char direction[50];
  void do_move(struct char_data *ch, char *argument, int cmd);


  for (temp=0;temp<=5;++temp)
    if (world[hunter->in_room].dir_option[temp])
      if (world[hunter->in_room].dir_option[temp]->to_room ==
	  direct(hunter->in_room,victim->in_room))
	{

	  switch(temp) {
	  case 0:
	    strcpy(direction,"north");
	    break;
	  case 1:
	    strcpy(direction,"east");
	    break;
	  case 2:
	    strcpy(direction,"south");
	    break;
	  case 3:
	    strcpy(direction,"west");
	    break;
	  case 4:
	    strcpy(direction,"up");
	    break;
	  case 5:
	    strcpy(direction,"down");
	    break;
	  }

	  do_move(hunter,direction,temp+1);	  

	}


}


/* nice and clean eh? PHL */
void do_hunt(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct char_data *i,*bestchar;
  struct obj_data *k,*bestobj;



  struct descriptor_data *d;
  int bestdist;
  int temp;
  bool found;
  int x;
  char *exits[] =
    {	
      "North",
      "East",
      "South",
      "West",
      "Up",
      "Down"
      };



  one_argument(argument, name);
  
  if (!*name) 
    {
      send_to_char("What do you want to hunt for?\n\r", ch);
      return;

    }  
  if (GET_LEVEL(ch)>20)
    if (is_number(name))
      {
	x=atoi(name);
	if (x<0||x>top_of_world)
	  {
	    send_to_char("Real room number out of range.\n\r",ch);
	    return;
	  }
	sprintf(buf,"From(R%d)(V%d) To(%d)(V%d).. Go(R%d)(V%d) Dist(%d)\n\r",
		ch->in_room,world[ch->in_room].number,
		x,world[x].number,
		direct(ch->in_room,x),world[direct(ch->in_room,x)].number,
		dist(ch->in_room,x));
	send_to_char(buf,ch);
	return;
      }
  
  *buf = '\0';
  found = FALSE;
  bestdist = MAXROOM+1;
  for (i = character_list; i; i = i->next)
    if (isname(name, i->player.name) && CAN_SEE(ch, i) )
      {
	if ((i->in_room != NOWHERE) && 
	    ((GET_LEVEL(ch)>20) ||
	     (dist(ch->in_room,i->in_room)<=(GET_LEVEL(ch)*4))))
	  {
	    
	    if ((IS_SET(i->specials.act,PLR_GODINVIS))&&
		(GET_LEVEL(ch)<GET_LEVEL(i))) continue;
	    if (dist(ch->in_room,i->in_room) != -1)
	      if (dist(ch->in_room,i->in_room) < bestdist)
		{
		  found=TRUE;
		  bestchar = i;
		  bestdist = dist(ch->in_room,i->in_room);
		}
	  }
      }
  if (found)
    {
      if (!bestdist)
	{
	  send_to_char("Right here in this room!\n\r",ch);
	  REMOVE_BIT(ch->specials.act,PLR_HUNT);
	  ch->specials.hunting=NULL;
	  return;
	}
      for (temp=0;temp<=5;++temp)
	if (world[ch->in_room].dir_option[temp])
	  if (world[ch->in_room].dir_option[temp]->to_room ==
	      direct(ch->in_room,bestchar->in_room))
	  {
      if (IS_SET(ch->specials.act,PLR_GRAPHICS))
      sprintf(buf,"%c%c%c%cThe scent seems to be coming from direction %s.%c%c%c\n\r",27,91,55,109,exits[temp],27,91,109);
else      sprintf(buf,"The scent seems to be coming from direction %s.\n\r",exits[temp]);

	    send_to_char(buf,ch);
	    ch->specials.hunting=bestchar;
	    SET_BIT(ch->specials.act,PLR_HUNT);
	    break;
	  }
      return;
    }

  if (!found)
      {
	for (k = object_list; k; k = k->next)
	  if (isname(name, k->name) && CAN_SEE_OBJ(ch, k) && 
	      (k->in_room != NOWHERE)) 
	    if ((GET_LEVEL(ch) > 20) ||
		(dist(ch->in_room,k->in_room)<=(GET_LEVEL(ch)*4)))
	      if (dist(ch->in_room,k->in_room) != -1)
		if (dist(ch->in_room,k->in_room) < bestdist)
		  {
		    found=TRUE;
		    bestobj = k;
		    bestdist = dist(ch->in_room,k->in_room);
		  }
      }
  if (found)
    {
      if (!bestdist)
	{
	  send_to_char("Right here in this room!\n\r",ch);
	  REMOVE_BIT(ch->specials.act,PLR_HUNT);
	  ch->specials.hunting=NULL;
	  return;
	}
      for (temp=0;temp<=5;++temp)
	if (world[ch->in_room].dir_option[temp])
	  if (world[ch->in_room].dir_option[temp]->to_room ==
	      direct(ch->in_room,bestobj->in_room))
	  {
if (IS_SET(ch->specials.act,PLR_GRAPHICS))
      sprintf(buf,"%c%c%c%cYour ESP (with help from gods) tells you the object is %s.%c%c%c\n\r",27,91,55,109,exits[temp],27,91,109);
else      sprintf(buf,"Your ESP (with help from gods) tells you the object is %s.\n\r",exits[temp]);

	    send_to_char(buf,ch);
	  REMOVE_BIT(ch->specials.act,PLR_HUNT);
	  ch->specials.hunting=NULL;
	    break;
	  }
      return;
    }
  
  if (!found)
    {
      send_to_char("You couldn't find any scent.\n\r", ch);
      REMOVE_BIT(ch->specials.act,PLR_HUNT);
      ch->specials.hunting=NULL;
    }
}








  

