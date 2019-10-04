/*
 *   IRC - Internet Relay Chat, src/modules/undccdeny.c
 *   (C) 2004 The UnrealIRCd Team
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

CMD_FUNC(cmd_undccdeny);

#define MSG_UNDCCDENY 	"UNDCCDENY"	

ModuleHeader MOD_HEADER
  = {
	"undccdeny",
	"5.0",
	"command /undccdeny", 
	"UnrealIRCd Team",
	"unrealircd-5",
    };

MOD_INIT()
{
	CommandAdd(modinfo->handle, MSG_UNDCCDENY, cmd_undccdeny, MAXPARA, CMD_USER);
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

/* Remove a temporary dccdeny line
 * parv[1] - file/mask
 */
CMD_FUNC(cmd_undccdeny)
{
	ConfigItem_deny_dcc *d;

	if (!MyUser(client))
		return;

	if (!ValidatePermissionsForPath("server-ban:dccdeny",client,NULL,NULL,NULL))
	{
		sendnumeric(client, ERR_NOPRIVILEGES);
		return;
	}

	if ((parc < 2) || BadPtr(parv[1]))
	{
		sendnumeric(client, ERR_NEEDMOREPARAMS, "UNDCCDENY");
		return;
	}

	if ((d = Find_deny_dcc(parv[1])) && d->flag.type2 == CONF_BAN_TYPE_TEMPORARY)
	{
		sendto_ops("%s removed a temp dccdeny for %s", client->name, parv[1]);
		DCCdeny_del(d);
		return;
	} else
	{
		sendnotice(client, "*** Unable to find a temp dccdeny matching %s", parv[1]);
	}
}
