/*
 * Block CTCP UnrealIRCd Module (Channel Mode +C)
 * (C) Copyright 2000-.. Bram Matthys (Syzop) and the UnrealIRCd team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

CMD_FUNC(noctcp);

ModuleHeader MOD_HEADER
  = {
	"chanmodes/noctcp",
	"4.2",
	"Channel Mode +C",
	"UnrealIRCd Team",
	"unrealircd-5",
    };

Cmode_t EXTCMODE_NOCTCP;

#define IsNoCTCP(chptr)    (chptr->mode.extmode & EXTCMODE_NOCTCP)

char *noctcp_prechanmsg(Client *client, Channel *chptr, MessageTag *mtags, char *text, int notice);

MOD_TEST()
{
	return MOD_SUCCESS;
}

MOD_INIT()
{
CmodeInfo req;

	memset(&req, 0, sizeof(req));
	req.paracount = 0;
	req.flag = 'C';
	req.is_ok = extcmode_default_requirehalfop;
	CmodeAdd(modinfo->handle, req, &EXTCMODE_NOCTCP);
	
	HookAddPChar(modinfo->handle, HOOKTYPE_PRE_CHANMSG, 0, noctcp_prechanmsg);
	
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

static int IsACTCP(char *s)
{
	if (!s)
		return 0;

	if ((*s == '\001') && strncmp(&s[1], "ACTION ", 7))
		return 1;

	return 0;
}

char *noctcp_prechanmsg(Client *client, Channel *chptr, MessageTag *mtags, char *text, int notice)
{
	if (MyUser(client) && IsNoCTCP(chptr) && IsACTCP(text))
	{
		if (!notice)
		{
			sendnumeric(client, ERR_CANNOTSENDTOCHAN, chptr->chname,
				   "CTCPs are not permitted in this channel", chptr->chname);
		}

		return NULL;
	}
	return text;
}
