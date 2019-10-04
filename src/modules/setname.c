/*
 *   IRC - Internet Relay Chat, src/modules/setname.c
 *   (c) 1999-2001 Dominick Meglio (codemastr) <codemastr@unrealircd.com>
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers. 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

CMD_FUNC(cmd_setname);

#define MSG_SETNAME 	"SETNAME"	/* setname */

ModuleHeader MOD_HEADER
  = {
	"setname",	/* Name of module */
	"5.0", /* Version */
	"command /setname", /* Short description of module */
	"UnrealIRCd Team",
	"unrealircd-5",
    };

MOD_INIT()
{
	CommandAdd(modinfo->handle, MSG_SETNAME, cmd_setname, 1, CMD_USER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

MOD_LOAD()
{
	return MOD_SUCCESS;
}

MOD_UNLOAD()
{
	return MOD_SUCCESS;
}

/* cmd_setname - 12/05/1999 - Stskeeps
 * :prefix SETNAME :gecos
 * parv[1] - gecos
 * D: This will set your gecos to be <x> (like (/setname :The lonely wanderer))
   yes it is experimental but anyways ;P
    FREEDOM TO THE USERS! ;) 
*/ 
CMD_FUNC(cmd_setname)
{
	int xx;
	char tmpinfo[REALLEN + 1];
	char spamfilter_user[NICKLEN + USERLEN + HOSTLEN + REALLEN + 64];
	ConfigItem_ban *bconf;

	if ((parc < 2) || BadPtr(parv[1]))
	{
		sendnumeric(sptr, ERR_NEEDMOREPARAMS, "SETNAME");
		return;
	}

	if (strlen(parv[1]) > REALLEN)
	{
		if (MyConnect(sptr))
		{
			sendnotice(sptr, "*** /SetName Error: \"Real names\" may maximum be %i characters of length",
				REALLEN);
		}
		return;
	}

	if (MyUser(sptr))
	{
		/* set temp info for spamfilter check*/
		strcpy(tmpinfo, sptr->info);
		/* set the new name before we check, but don't send to servers unless it is ok */
		strcpy(sptr->info, parv[1]);
		spamfilter_build_user_string(spamfilter_user, sptr->name, sptr);
		if (!match_spamfilter(sptr, spamfilter_user, SPAMF_USER, NULL, 0, NULL))
		{
			/* Was rejected by spamfilter, restore the realname */
			strcpy(sptr->info, tmpinfo);
			return;
		}

		/* Check for realname bans here too */
		if (!ValidatePermissionsForPath("immune:server-ban:ban-realname",sptr,NULL,NULL,NULL) &&
		    ((bconf = Find_ban(NULL, sptr->info, CONF_BAN_REALNAME))))
		{
			return banned_client(sptr, "realname", bconf->reason?bconf->reason:"", 0, 0);
		}
	} else {
		/* remote user */
		strcpy(sptr->info, parv[1]);
	}

	sendto_server(sptr, 0, 0, NULL, ":%s SETNAME :%s", sptr->name, parv[1]);

	if (MyConnect(sptr))
	{
		sendnotice(sptr, "Your \"real name\" is now set to be %s - you have to set it manually to undo it",
		           parv[1]);
	}
}
