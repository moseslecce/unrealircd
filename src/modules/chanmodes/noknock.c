/*
 * Disallow knocks UnrealIRCd Module (Channel Mode +K)
 * (C) Copyright 2014 Travis McArthur (Heero) and the UnrealIRCd team
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

ModuleHeader MOD_HEADER(noknock)
  = {
	"chanmodes/noknock",
	"4.2",
	"Channel Mode +K",
	"UnrealIRCd Team",
	"unrealircd-5",
    };

Cmode_t EXTCMODE_NOKNOCK;

#define IsNoKnock(chptr)    (chptr->mode.extmode & EXTCMODE_NOKNOCK)

int noknock_check (Client *sptr, Channel *chptr);
int noknock_mode_allow(Client *cptr, Channel *chptr, char mode, char *para, int checkt, int what);
int noknock_mode_del (Channel *chptr, int modeChar);

MOD_TEST(noknock)
{
	return MOD_SUCCESS;
}

MOD_INIT(noknock)
{
CmodeInfo req;

	memset(&req, 0, sizeof(req));
	req.paracount = 0;
	req.flag = 'K';
	req.is_ok = noknock_mode_allow;
	CmodeAdd(modinfo->handle, req, &EXTCMODE_NOKNOCK);
	
	HookAdd(modinfo->handle, HOOKTYPE_PRE_KNOCK, 0, noknock_check);
	HookAdd(modinfo->handle, HOOKTYPE_MODECHAR_DEL, 0, noknock_mode_del);

	
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

MOD_LOAD(noctcp)
{
	return MOD_SUCCESS;
}

MOD_UNLOAD(noctcp)
{
	return MOD_SUCCESS;
}


int noknock_check (Client *sptr, Channel *chptr)
{
	if (MyClient(sptr) && IsNoKnock(chptr))
	{
		sendnumeric(sptr, ERR_CANNOTKNOCK, chptr->chname, "No knocks are allowed! (+K)");
		return HOOK_DENY;
	}

	return HOOK_CONTINUE;
}

int noknock_mode_del (Channel *chptr, int modeChar)
{
	// Remove noknock when we're removing invite only
	if (modeChar == 'i')
		chptr->mode.extmode &= ~EXTCMODE_NOKNOCK;

	return 0;
}

int noknock_mode_allow(Client *cptr, Channel *chptr, char mode, char *para, int checkt, int what)
{
	if (!(chptr->mode.mode & MODE_INVITEONLY))
	{
		if (checkt == EXCHK_ACCESS_ERR)
		{
			sendnumeric(cptr, ERR_CANNOTCHANGECHANMODE, 'K', "+i must be set");
		}
		return EX_DENY;
	}

	return extcmode_default_requirehalfop(cptr,chptr,mode,para,checkt,what);
}
