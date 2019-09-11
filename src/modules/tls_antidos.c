/*
 * SSL/TLS Anti DoS module
 * This protects against SSL renegotiation attacks while still allowing us
 * to leave renegotiation on with all it's security benefits.
 *
 * (C) Copyright 2015- Bram Matthys and the UnrealIRCd team.
 * 
 * License: GPLv2
 */

#include "unrealircd.h"

ModuleHeader MOD_HEADER(tls_antidos)
  = {
	"tls_antidos",
	"5.0",
	"TLS Renegotiation DoS protection",
	"UnrealIRCd Team",
	"unrealircd-5",
    };

#define HANDSHAKE_LIMIT_COUNT 3
#define HANDSHAKE_LIMIT_SECS 300

typedef struct SAD SAD;
struct SAD {
	Client *acptr; /**< client */
	time_t ts; /**< time */
	int n; /**< number of times */
};

int tls_antidos_index = 0; /* slot# we acquire from OpenSSL. Hmm.. looks awfully similar to our moddata system ;) */

/* Forward declaration */
int tls_antidos_handshake(Client *acptr);

void tls_antidos_free(void *parent, void *ptr, CRYPTO_EX_DATA *ad, int idx, long argl, void *argp);

MOD_INIT(tls_antidos)
{
	HookAdd(modinfo->handle, HOOKTYPE_HANDSHAKE, 0, tls_antidos_handshake);

	MARK_AS_OFFICIAL_MODULE(modinfo);
	
	ModuleSetOptions(modinfo->handle, MOD_OPT_PERM, 1);
	/* Note that we cannot be MOD_OPT_PERM_RELOADABLE as we use OpenSSL functions to register
	 * an index and callback function.
	 */
	
	tls_antidos_index = SSL_get_ex_new_index(0, "tls_antidos", NULL, NULL, tls_antidos_free);
	
	return MOD_SUCCESS;
}

MOD_LOAD(tls_antidos)
{
	return MOD_SUCCESS;
}

MOD_UNLOAD(tls_antidos)
{
	return MOD_SUCCESS;
}

/** Called upon handshake (and other events) */
void ssl_info_callback(const SSL *ssl, int where, int ret)
{
	if (where & SSL_CB_HANDSHAKE_START)
	{
		SAD *e = SSL_get_ex_data(ssl, tls_antidos_index);
		Client *acptr = e->acptr;
		
		if (IsServer(acptr) || IsDeadSocket(acptr))
			return; /* if it's a server, or already pending to be killed off then we don't care */

		if (e->ts < TStime() - HANDSHAKE_LIMIT_SECS)
		{
			e->ts = TStime();
			e->n = 1;
		} else {
			e->n++;
			if (e->n >= HANDSHAKE_LIMIT_COUNT)
			{
				ircd_log(LOG_ERROR, "TLS Handshake flood detected from %s -- killed", get_client_name(acptr, TRUE));
				sendto_realops("TLS Handshake flood detected from %s -- killed", get_client_name(acptr, TRUE));
				dead_link(acptr, "TLS Handshake flood detected");
			}
		}
	}
}

/** Called when a client has just connected to us.
 * This function is called quite quickly after accept(),
 * in any case very likely before any data has been received.
 */
int tls_antidos_handshake(Client *acptr)
{
	if (acptr->local->ssl)
	{
		SAD *sad = MyMallocEx(sizeof(SAD));
		sad->acptr = acptr;
		SSL_set_info_callback(acptr->local->ssl, ssl_info_callback);
		SSL_set_ex_data(acptr->local->ssl, tls_antidos_index, sad);
	}
	return 0;
}

/** Called by OpenSSL when the SSL structure is freed (so we can free up our custom struct too) */
void tls_antidos_free(void *parent, void *ptr, CRYPTO_EX_DATA *ad, int idx, long argl, void *argp)
{
	MyFree(ptr);
}
