#include <strings.h>
#include <stdio.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"

#define ATM_FILE "atm.accounts"    /* File for bank */
#define BAN_FILE "sites.banned"    /* File for banned sites */
typedef struct ATMtAccount {
   char name[20];
   long balance;
} ATMtAccount;

extern struct room_data *world;
extern struct zone_data *zone_table;

int real_object(int i);
struct char_data *get_char_vis(struct char_data *ch,char *name);


void ATMwithdraw(struct char_data *ch, char *arg, int cmd);
void ATMdeposit(struct char_data *ch, char *arg, int cmd);
void ATMbalance(struct char_data *ch, char *arg, int cmd);

/**********************************************************************/

int ATMisPresent(struct char_data *ch) 
{
   struct obj_data *tmp;
   bool found = FALSE;


   for (tmp = world[ch->in_room].contents;
        tmp != NULL && !found;
        tmp = tmp->next_content) {


      if ((tmp->item_number == real_object(3036)) && (CAN_SEE_OBJ(ch, tmp))) {
         found = TRUE;
      } /* if */
   } /* for */

   return found;

} /* ATMisPresent */

/*******************************************************************/

void do_deposit(struct char_data *ch, char *argument, int cmd) 
{

   if (ATMisPresent(ch)) {
      ATMdeposit(ch, argument, cmd);
   }
   else {
      send_to_char("There's no ATM here!\n\r", ch);
   } /* if */

} /* do_deposit */

/*******************************************************************/

void do_balance(struct char_data *ch, char *argument, int cmd)
{

   if (ATMisPresent(ch)) {
      ATMbalance(ch, argument, cmd);
   }
   else {
      send_to_char("There's no ATM here!\n\r", ch);
   } /* if */

} /* do_balance */

/*******************************************************************/

void do_withdraw(struct char_data *ch, char *argument, int cmd)
{

   if (ATMisPresent(ch)) {
      ATMwithdraw(ch, argument, cmd);
   }
   else {
      send_to_char("There's no ATM here!\n\r", ch);
   } /* if */

} /* do_withdraw */

/*****************************************************************/

int checkban(char *theaddress)
{
  FILE *fptr;
  char buf[MAX_STRING_LENGTH];
  char tmpadd[MAX_STRING_LENGTH];
  if ((fptr = fopen(BAN_FILE,"r"))==NULL) 
    {
      log("Error opening BAN_FILE!");
      exit(0);
    }
  strcpy(tmpadd,theaddress);
  strcat(tmpadd,"\n");

  while(!feof(fptr))
    {
      fgets(buf,100,fptr);
      if (!strcmp(buf,tmpadd)) 
	{
	  fclose(fptr);
	  return (1);
	}
      
    }
  fclose(fptr);
  return (0);
}



void do_ban(struct char_data *ch, char *argument, int cmd) 
{
  FILE *fptr;
  char buf[MAX_STRING_LENGTH];
  char theaddress[MAX_STRING_LENGTH];
  char storage[50][50];

  bool found=FALSE;
  int i=0;

  one_argument(argument,theaddress);

  if (!*theaddress)
    {
      if ((fptr = fopen(BAN_FILE,"r"))==NULL) 
	{
	  log("Error opening BAN_FILE!");
	  exit(0);
	}
      send_to_char("Banned Sites...\n\r",ch);
      while(!feof(fptr))
	{
	  fgets(buf,100,fptr);
	  send_to_char(buf,ch);
	}
      return;
    }
  

  if ((fptr = fopen(BAN_FILE,"r"))==NULL) 
    {
      log("Error opening BAN_FILE!");
      exit(0);
    }
  strcat(theaddress,"\n");
  while(!feof(fptr))
    {
      fgets(buf,100,fptr);
      if (!strcmp(buf,theaddress)) 
	{
	  found=TRUE;
	  continue;
	}
      strcpy(storage[i++],buf);
    }
  fclose(fptr);

  if (!found)
    {
      if ((fptr = fopen(BAN_FILE,"a"))==NULL) 
	{
	  log("Error opening BAN_FILE for append!");
	  exit(0);
	}
      sprintf(buf,"Banned Site: %s\r",theaddress);
      send_to_char(buf,ch);

      fputs(theaddress,fptr);

      fclose(fptr);
      return;
    }

  if ((fptr = fopen(BAN_FILE,"w"))==NULL) 
    {
      log("Error rewriting to BAN_FILE!");
      exit(0);
    }  
  while (i--)
    {
      fputs(storage[i],fptr);
    }
  sprintf(buf,"Allowed Site: %s\r",theaddress);
  send_to_char(buf,ch);
  fclose(fptr);

}

int ATMgetUser(char *name, ATMtAccount *acc)
{
   FILE *fp;
   int offset = 0;   


   if ((fp = fopen(ATM_FILE, "r")) == NULL) {
      log("Error opening ATM_FILE!");
      exit(0);
   } /* if */

   while (!feof(fp)) {
      fread(acc, sizeof(ATMtAccount), 1, fp);
      if ((str_cmp(acc->name, name) == 0) && (!feof(fp))) {
         /* Found the player, return this record */
         offset = ftell(fp) - (int)sizeof(ATMtAccount);
	 fclose(fp);
         return offset;
      } /* if */
   } /* while */

   /*
    * If the person is not in the bank file, then create an entry.
    */
   strcpy(acc->name, name);
   acc->balance = 0;

   fseek(fp, 0, 2);   /* Seek to end of file */
   offset = ftell(fp);
   fwrite(acc, sizeof(ATMtAccount), 1, fp);
   fclose(fp);

   return offset;
  
} /* ATMgetUser */                    

/*****************************************************************/

void ATMputUser(int offset, ATMtAccount *acc)
{
   FILE *fp;
   bool found = FALSE;   
   ATMtAccount temp;


   if ((fp = fopen(ATM_FILE, "r+")) == NULL) {
      log("Error opening ATM_FILE!");
      exit(0);
   } /* if */

   fseek(fp, offset, 0);
   fwrite(acc, sizeof(ATMtAccount), 1, fp);   
   fclose(fp);

   return;
  
} /* ATMputUser */                    

/*****************************************************************/


void ATMbalance(struct char_data *ch, char *arg, int cmd)
{
   ATMtAccount acc;
   char buf[MAX_INPUT_LENGTH];
   struct char_data *who;

   if(GET_LEVEL(ch)>=21 && *arg) {
      if(!(who=get_char_vis(ch,arg))) {
         send_to_char("Get whose bank balance?\n\r",ch);
         return;
      }
   } else
      who=ch;

   (void)ATMgetUser(who->player.name, &acc);
   if (acc.balance == 0) {
     sprintf(buf, "%s's balance is a big fat zero!\n\r",who->player.name);
   } 
   else if (acc.balance != 1) {
     sprintf(buf,"%s's balance is %ld coins.\n\r",who->player.name,acc.balance);
   }
   else {
     sprintf(buf,"%s's balance is %ld coin.\n\r",who->player.name,acc.balance);
   } /* if */

   send_to_char (buf, ch);      
 
} /* ATMbalance */                    

/*****************************************************************/

void ATMdeposit(struct char_data *ch, char *arg, int cmd)
{
   ATMtAccount acc;
   char buf[MAX_INPUT_LENGTH];
   char number[MAX_INPUT_LENGTH];
   char logit[MAX_INPUT_LENGTH];
   long amount;
   int offset;

   offset = ATMgetUser(ch->player.name, &acc);


	   sprintf(logit,"WIZINFO: (%s) deposit %s",GET_NAME(ch),arg);
	   log(logit);


   one_argument(arg, number);
   if (!*number) {
      send_to_char("Usage :: DEPOSIT <value>\n\r", ch);
      return;
   } /* if */

   if (strlen(number)>7){ /* Was getting overflow...fix--Swiftest */
      send_to_char("Usage :: DEPOSIT <value>\n\r",ch);
      return;
   }

   if (!(amount = atol(number))) {
      send_to_char("Usage :: DEPOSIT <value>\n\r", ch);
      return;
   } /* if */

   if (amount < 0) {
      send_to_char("The amount to deposit must be non-negative.\n\r",ch);
      return;
   } /* if */

   if (amount > GET_GOLD(ch)) {
      send_to_char("You don't have that much gold.\n\r", ch);
   }
   else {
      acc.balance += amount;
      GET_GOLD(ch) -= amount;
      ATMputUser(offset, &acc);

      if (amount == 1) {
         sprintf(buf, "You deposit 1 coin.\n\r");
      }
      else if (amount == 0) {
         sprintf(buf, "A most unproductive effort.\n\r");   
      }
      else {
         sprintf(buf, "You deposit %ld coins.\n\r", amount);
      } /* if */

      send_to_char(buf, ch);
   } /* if */

} /* ATMdeposit */                    

/*****************************************************************/

void ATMwithdraw(struct char_data *ch, char *arg, int cmd)
{
   ATMtAccount acc;
   char buf[MAX_INPUT_LENGTH];
   char number[MAX_INPUT_LENGTH];
   char logit[MAX_INPUT_LENGTH];
   long amount;
   int offset;

   offset = ATMgetUser(ch->player.name, &acc);


	   sprintf(logit,"WIZINFO: (%s) withdraw %s",GET_NAME(ch),arg);
	   log(logit);


   one_argument(arg, number);
   if (!*number) {
      send_to_char("Usage :: WITHDRAW <value>\n\r", ch);
      return;
   } /* if */

   if (strlen(number)>7){ /* Was getting overflow error--Swiftest */
      send_to_char("Usage :: WITHDRAW <value>\n\r",ch);
      return;
   } /* if */

   if (!(amount = atol(number))) {
      send_to_char("Usage :: WITHDRAW <value>\n\r", ch);
      return;
   } /* if */

   if (amount < 0) {
      send_to_char("The amount to withdraw must be non-negative.\n\r",ch);
      return;
   } /* if */

   if (amount > acc.balance) {
      send_to_char("You don't have that much gold in your account.\n\r", ch);
   }
   else {
      acc.balance -= amount;
      GET_GOLD(ch) += amount;
      ATMputUser(offset, &acc);

      if (amount == 1) {
         sprintf(buf, "You withdraw 1 coin.\n\r");
      }
      else if (amount == 0) {
         sprintf(buf, "A most unproductive effort.\n\r");   
      }
      else {
         sprintf(buf, "You withdraw %ld coins.\n\r", amount);
      } /* if */

      send_to_char(buf, ch);
   } /* if */

} /* ATMwithdraw */                

/*****************************************************************/


