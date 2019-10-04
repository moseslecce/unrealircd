/*
 * Extended ban that affects nick-changes only (+b ~n)
 * (C) Copyright 2003-.. Bram Matthys (Syzop) and the UnrealIRCd team
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

ModuleHeader MOD_HEADER
= {
	"extbans/nickchange",
	"4.2",
	"ExtBan ~n - prevent nick-changes only",
	"UnrealIRCd Team",
	"unrealircd-5",
};

/* Forward declarations */
int extban_nickchange_is_banned(Client *client, Channel *chptr, char *banin, int type, char **msg, char **errmsg);

/** Called upon module init */
MOD_INIT()
{
	ExtbanInfo req;
	
	req.flag = 'n';
	req.is_ok = extban_is_ok_nuh_extban;
	req.conv_param = extban_conv_param_nuh_or_extban;
	req.is_banned = extban_nickchange_is_banned;
	req.options = EXTBOPT_ACTMODIFIER;
	if (!ExtbanAdd(modinfo->handle, req))
	{
		config_error("could not register extended ban type");
		return MOD_FAILED;
	}

	MARK_AS_OFFICIAL_MODULE(modinfo);
	
	return MOD_SUCCESS;
}

/** Called upon module load */
MOD_LOAD()
{
	return MOD_SUCCESS;
}

/** Called upon unload */
MOD_UNLOAD()
{
	return MOD_SUCCESS;
}

/** This ban that affects nick-changes only */
int extban_nickchange_is_banned(Client *client, Channel *chptr, char *banin, int type, char **msg, char **errmsg)
{
	char *sub_ban;

	if (type != BANCHK_NICK)
		return 0;

	if (has_voice(client, chptr))
		return 0;

	sub_ban = banin + 3;

	return ban_check_mask(client, chptr, sub_ban, type, msg, errmsg, 0);
}

