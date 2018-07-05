#define HUNT_FILE "hunt.table"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"

typedef struct HUNTTYPE {
  int direct;
  int dist;
} HUNTTYPE;

extern struct room_data *world;
extern int top_of_world;
void do_hunt(struct char_data *a,char *b,int c)
{
}
void huntstep(struct char_data *a,int c)
{
}
void huntmove(struct char_data *a,struct char_data *b)
{
}




int main(int argc,char **argv)
{
  
  if (chdir(DFLT_DIR)<0)
    {
      perror("Cant open the directory");
      return(0);
    }
  log("booting zones");
  boot_zones();
  log("booting world");
  boot_world();
  log("renumbering world");
  renum_world();
  log("doing setup");
  huntsetup();
  log("writing to file");
  writehuntfile();
}

int direct[MAXROOM+1][MAXROOM+1];
int dist[MAXROOM+1][MAXROOM+1];

void writehuntfile()
{
  struct HUNTTYPE *cell;
  char buf[50];
  register int x,y;
  FILE *ptr;
  cell = malloc(sizeof(HUNTTYPE));
  if(!(  ptr=fopen(HUNT_FILE,"w")))
    {
      log("Error writing to HUNT_FILE");
      return;
    }
  
  for(x=0;x<=top_of_world;++x)
    for(y=0;y<=top_of_world;++y)
      {
	cell->direct=direct[x][y];
	cell->dist=dist[x][y];
	fwrite(cell,sizeof(HUNTTYPE),1,ptr);
      }

  fclose(ptr);
}

void inithunt(void)
{
  register int x,y;
 char buf[100];
  sprintf(buf,"init:tow(%d)",top_of_world);
  log(buf);
  for (x=0;x<=top_of_world;++x)
    for (y=0;y<=top_of_world;++y)
      {
	direct[x][y]=-1;
	dist[x][y]=-1;
	if (x==y)
	  {
	    direct[x][y]=-2;
	    dist[x][y]=0;
	  }
      }
  sprintf(buf,"init:beginassign",top_of_world);
  log(buf);

  for(x=0;x<=top_of_world;++x)
    for(y=0;y<=5;++y)
      if(world[x].dir_option[y])
	if (world[x].dir_option[y]->to_room != NOWHERE)
	  if (world[x].dir_option[y]->to_room != x)
	    {
	      direct[x][world[x].dir_option[y]->to_room]=
		world[x].dir_option[y]->to_room;
	      dist[x][world[x].dir_option[y]->to_room]=1;
	    }
}

/* This is a weird, but working world table needed by do_hunt to find the
path to any object or monster.  Note that it is my pseudo algorithm for
finding the shortest path between two rooms.  (If a better one exists
please insert here :)  PHL
*/

void huntsetup(void)
{
  register int counter=0,tempx,minpath;
  register int top_of_worl = top_of_world;
  char buf[100];
  register int from,to,x;
  inithunt();
  sprintf(buf,"setup:begin",top_of_world);
  log(buf);
  do
    {
      for (from=0;from<=top_of_worl;++from)
	for (to=0;to<=top_of_worl;++to)
	  if (direct[from][to]==-1)
	    {
	      tempx=-1;
	      minpath=top_of_worl+2;
	      for (x=0;x<=top_of_worl;++x)
		if ((direct[x][to]>=0)&&(direct[from][x]>=0))
		  if (dist[x][to]+dist[from][x]<minpath)
		    {
		      minpath=dist[x][to]+dist[from][x];
		      tempx=direct[from][x];
		    }
	      if (tempx!=-1)
		{
		  direct[from][to]=tempx;
		  dist[from][to]=minpath;
		}
	      
	    }

      sprintf(buf,"setup:(%d)",counter);
      log(buf);
    } while (counter++ <=10);

}
