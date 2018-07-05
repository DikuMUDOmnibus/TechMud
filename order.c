

void do_order(struct char_data *ch, char *argument, int cmd)
{
	char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool found = FALSE;
	int org_room;
	struct char_data *victim;
	struct follow_type *k;

	half_chop(argument, name, message);

	if(isdweeb(ch)) { return; }
	if(GET_EXP(ch) < 1500) {
		send_to_char("Due to abuse, you must have at least 1500 experience to use this command.\n\r",ch);
		return;
	}
	if (!*name || !*message)
		send_to_char("Order who to do what?\n\r", ch);
	else if (!(victim = get_char_room_vis(ch, name)) &&
	         str_cmp("follower", name) && str_cmp("followers", name))
			send_to_char("That person isn't here.\n\r", ch);
	else if (ch == victim)
		send_to_char("You obviously suffer from schitzophrenia.\n\r", ch);

	else {
		if (IS_AFFECTED(ch, AFF_CHARM)) {
			send_to_char("Your superior would not aprove of you giving orders.\n\r",ch);
			return;
		}

		if (victim) {
			sprintf(buf, "$N orders you to '%s'", message);
			act(buf, FALSE, victim, 0, ch, TO_CHAR);
			act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

			if ( (victim->master!=ch) || !IS_AFFECTED(victim, AFF_CHARM) )
				act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
			else {
				send_to_char("Ok.\n\r", ch);
				command_interpreter(victim, message);
			}
		} else {  /* This is order "followers" */
			sprintf(buf, "$n issues the order '%s'.", message);
			act(buf, FALSE, ch, 0, victim, TO_ROOM);

			org_room = ch->in_room;

			for (k = ch->followers; k; k = k->next) {
				if (org_room == k->follower->in_room)
					if (IS_AFFECTED(k->follower, AFF_CHARM)) {
						found = TRUE;
						command_interpreter(k->follower, message);
					}
			}
			if (found)
				send_to_char("Ok.\n\r", ch);
			else
				send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);
		}
	}
}

