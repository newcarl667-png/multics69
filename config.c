
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>

#include <time.h>


#include "common.h"
#include "convert.h"
#include "tools.h"
#include "debug.h"
#include "ecmdata.h"
#include "sockets.h"


#ifdef CCCAM
#include "msg-cccam.h"
#endif

#include "parser.h"
#include "config.h"
#include "main.h"

void cs_disconnect_cli(struct cs_client_data *cli);
void cs_disconnect_srv(struct cs_server_data *srv);
#ifdef CCCAM_SRV
void cc_disconnect_cli(struct cc_client_data *cli);
#endif
#ifdef MGCAMD_SRV
void mg_disconnect_cli(struct mg_client_data *cli);
#endif


///////////////////////////////////////////////////////////////////////////////
// HOST LIST
///////////////////////////////////////////////////////////////////////////////

struct host_data *add_host( struct config_data *cfg, char *hostname)
{
	struct host_data *host = cfg->host;
	// Search
	while (host) {
		if ( !strcmp(host->name, hostname) ) return host;
		host = host->next;
	}
	// Create
	host = malloc( sizeof(struct host_data) );
	memset( host, 0, sizeof(struct host_data) );
	strcpy( host->name, hostname );
	host->ip = 0;
	host->checkiptime = 0;
	host->next = cfg->host;
	cfg->host = host;
	return host;
}

void free_allhosts( struct config_data *cfg )
{
	while (cfg->host) {
		struct host_data *host = cfg->host;
		cfg->host = host->next;
		free(host);
	}
}

///////////////////////////////////////////////////////////////////////////////
// READ CONFIG
///////////////////////////////////////////////////////////////////////////////

// Default Newcamd DES key 
int cc_build[] = { 2892, 2971, 3094, 3165, 0 };

uint8 defdeskey[14] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14 };

char *cc_version[] = { "2.0.11", "2.1.1", "2.1.2", "2.1.3", "" };

char *uppercase(char *str)
{
	int i;
	for(i=0;;i++) {
		switch(str[i]) {
			case 'a'...'z':
				str[i] = str[i] - ('a'-'A');
				break;
			case 0:
				return str;
		}
	}
}

char currentline[10240];

void init_cccamserver(struct cccamserver_data *cccam)
{
	memset( cccam, 0, sizeof(struct cccamserver_data) );
#ifdef CCCAM_SRV
	cccam->client = NULL;
	cccam->handle = INVALID_SOCKET;
	cccam->port = 0;
#endif
}

void init_mgcamdserver(struct mgcamdserver_data *mgcamd)
{
	memset( mgcamd, 0, sizeof(struct mgcamdserver_data) );
	mgcamd->client = NULL;
	mgcamd->handle = INVALID_SOCKET;
	mgcamd->port = 0;
	mgcamd->dcwtime = 0;
	memcpy( mgcamd->key, defdeskey, 14);
}


void init_config(struct config_data *cfg)
{
	// Init config data
	memset( cfg, 0, sizeof(struct config_data) );

	cfg->clientid = 1;
	cfg->serverid = 1;
	cfg->cardserverid = 0x64; // Start like in CCcam 
	cfg->cachepeerid = 1;

#ifdef HTTP_SRV
	cfg->http.port = 5500;
	cfg->http.handle = INVALID_SOCKET;
	cfg->http.autorefresh = 0;
#endif

	// CACHE
	cfg->cache.port = 0;
	cfg->cache.hits = 0;
	cfg->cache.handle = 0;
	cfg->cache.faccept0onid = 1;

#ifdef CCCAM
	//2.2.1 build 3316
	strcpy(cfg->cccam.version, cc_version[0]);
	sprintf(cfg->cccam.build, "%d", cc_build[0]);
	memcpy(cfg->cccam.nodeid, cccam_nodeid, 8);
	cfg->cccam.dcwtime = 0;
	cfg->cccam.clientid = 1;
	cfg->cccam.serverid = 1;
	cfg->cccam.server = NULL;
	//init_cccamserver( &cfg->cccam ); cfg->cccam.id = cfg->cccamserverid; cfg->cccamserverid++;
#endif

#ifdef FREECCCAM_SRV
	//2.2.1 build 3316
	strcpy(cfg->freecccam.version, cc_version[0]);//"2.0.11");
	sprintf(cfg->freecccam.build, "%d", cc_build[0]);
	memcpy(cfg->freecccam.nodeid, cccam_nodeid, 8);

	cfg->freecccam.server.client = NULL;
	cfg->freecccam.server.handle = INVALID_SOCKET;
	cfg->freecccam.server.port = 0;
	cfg->freecccam.dcwtime = 0;
	cfg->freecccam.maxusers = 0;
#endif

#ifdef MGCAMD_SRV
	cfg->mgcamd.dcwtime = 0;
	cfg->mgcamd.clientid = 1;
	cfg->mgcamd.serverid = 1;
	cfg->cccam.server = NULL;
#endif

#ifdef TESTCHANNEL
	cfg->testchn.sid = 0x2009;
	cfg->testchn.caid = 0x0500;
	cfg->testchn.prov = 0x032830;
#endif

}


void init_cardserver(struct cardserver_data *cs)
{
	memset( cs, 0, sizeof(struct cardserver_data) );

	cs->option.dcw.time = 0;
	cs->option.dcw.timeout = 3000;

	cs->newcamd.port = 8000;

	cs->option.server.max = 0;		// Max cs per ecm ( 0:unlimited )
	cs->option.server.interval = 1000;	// interval between 2 same ecm to diffrent cs
	cs->option.server.timeout = 2000; // timeout for resending ecm to cs
	cs->option.server.timeperecm = 0; // min time to do a request
	cs->option.server.validecmtime = 0; // cardserver max ecm reply time
	//cs->option.retry.cccam = 0; // Max number of retries for CCcam servers

	// Flags
	cs->option.faccept0sid = 1;
	cs->option.faccept0provider = 1;
	cs->option.faccept0caid = 1;
	cs->option.fallowcccam = 1;    // Allow cccam server protocol to decode ecm
	cs->option.fallownewcamd = 1;  // Allow newcamd server protocol to decode ecm
	cs->option.fallowradegast = 1;
	cs->option.fallowcache = 1;

	cs->option.cacheresendreq = 0;
	cs->option.cachesendrep = 1;
	cs->option.cachesendreq = 1;

	//cs->fmaxuphops = 2;     // allowed cards distance to decode ecm
	cs->option.cssendcaid = 1;
	cs->option.cssendprovid = 1;
	cs->option.cssendsid = 1;
	memcpy( cs->newcamd.key, defdeskey, 14);
	cs->option.dcw.check = 0; // default: off
}

void init_server(struct cs_server_data *srv)
{
	memset(srv,0,sizeof(struct cs_server_data) );
	memcpy( srv->key, defdeskey, 14);
	srv->handle = INVALID_SOCKET;
}



struct global_user_data
{
	struct global_user_data *next;
	char user[64];
	char pass[64];
	unsigned short csport[MAX_CSPORTS];
};


void parse_server_data( struct cs_server_data *tsrv )
{
	char str[255];
	int i,j;

	tsrv->sharelimits[0].caid = 0xFFFF;
	parse_spaces();
	if (*iparser=='{') { // Get Ports List & User Info
		iparser++;
		parse_spaces();
		if ( (*iparser>='0')&&(*iparser<='9') ) {
			i = 0;
			while (i<MAX_CSPORTS) {
				if ( parse_int(str)>0 ) {
					tsrv->csport[i] = atoi(str);
					for (j=0; j<i; j++) if ( tsrv->csport[j] && (tsrv->csport[j]==tsrv->csport[i]) ) break;
					if (j>=i) i++; else tsrv->csport[i] = 0;
				}
				else break;
				parse_spaces();
				if (*iparser==',') iparser++;
			}
		}
		else {
			while (1) {
				parse_spaces();
				if (*iparser=='}') break;
				// NAME1=value1; Name2=Value2 ... }
				parse_value(str,"\r\n\t =");
				// '='
				parse_spaces();
				if (*iparser!='=') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(,%d): '=' expected\n",iparser-currentline);
					break;
				}
				iparser++;
				// Value
				// Check for PREDEFINED names
				if (!strcmp(str,"profiles")) {
					i = 0;
					while (i<MAX_CSPORTS) {
						if ( parse_int(str)>0 ) {
							tsrv->csport[i] = atoi(str);
							for (j=0; j<i; j++) if ( tsrv->csport[j] && (tsrv->csport[j]==tsrv->csport[i]) ) break;
							if (j>=i) i++; else tsrv->csport[i] = 0;
						}
						else break;
						parse_spaces();
						if (*iparser==',') iparser++;
					}
				}
				else if (!strcmp(str,"sids")) {
					int count = 0;
					struct sid_chid_data *sids;
					sids = malloc ( sizeof(struct sid_chid_data) * MAX_SIDS );
					memset( sids, 0, sizeof(struct sid_chid_data) * MAX_SIDS );
					tsrv->sids = sids;
					while ( parse_hex(str)>0 ) {
						sids->sid = hex2int(str);
						parse_spaces();
						if (*iparser==':') { // CHID
							iparser++;
							if (parse_hex(str)>0) sids->chid = hex2int(str);
						}
						count++;
						sids++;
						if (count>=MAX_SIDS) {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(,%d): too many sids...\n",iparser-currentline);
							break;
						}
						parse_spaces();
						if (*iparser==',') iparser++;
					}
					sids->sid = 0;
					sids->chid = 0;
				}
				else if (!strcmp(str,"shares")) {
					int count = 0;
					while ( parse_hex(str)>0 ) {
						tsrv->sharelimits[count].caid = hex2int(str);
						parse_spaces();
						if (*iparser==':') { // PROVIDER ID
							iparser++;
							if (parse_hex(str)>0) tsrv->sharelimits[count].provid = hex2int(str); else break; // ERROR
							parse_spaces();
							if (*iparser==':') { // UPHOPS(optional)
								iparser++;
								if (parse_hex(str)>0) tsrv->sharelimits[count].uphops = hex2int(str); else break; // ERROR
							}
						} else break; // ERROR
						count++;
						if (count>=99) {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(,%d): too many share limits...\n",iparser-currentline);
							break;
						}
						parse_spaces();
						if (*iparser==',') iparser++;
					}
					tsrv->sharelimits[count].caid = 0xFFFF;
				}
				else if (!strcmp(str,"priority")) {
					if (parse_bin(str)) tsrv->priority = str[0]=='1';
				}
				parse_spaces();
				if (*iparser==';') iparser++; else break;
			}
		}	
		if (*iparser!='}') debugf(getdbgflag(DBG_CONFIG,0,0)," config(,%d): '}' expected\n",iparser-currentline);
	}
/*
	char line[1024];
	server2string( tsrv, line );
	printf(">> %s\n", line);
*/
}



void cfg_addserver(struct config_data *cfg, struct cs_server_data *srv)
{
	struct cs_server_data *tmp = cfg->server;
	srv->next = NULL;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = srv;
	} else cfg->server = srv;
}

void cfg_addprofile(struct config_data *cfg, struct cardserver_data *cs)
{
	struct cardserver_data *tmp = cfg->cardserver;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = cs;
	} else cfg->cardserver = cs;
	cs->next = NULL;
}

void cs_addnewcamdclient(struct cardserver_data *cs, struct cs_client_data *cli)
{
	struct cs_client_data *tmp = cs->newcamd.client;
	cli->next = NULL;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = cli;
	} else cs->newcamd.client = cli;
}

void cfg_addmgcamdclient(struct mgcamdserver_data *srv, struct mg_client_data *cli)
{
	struct mg_client_data *tmp = srv->client;
	cli->next = NULL;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = cli;
	} else srv->client = cli;
}

void cfg_addmgcamdserver(struct config_data *cfg, struct mgcamdserver_data *srv)
{
	struct mgcamdserver_data *tmp = cfg->mgcamd.server;
	srv->next = NULL;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = srv;
	}
	else cfg->mgcamd.server = srv;
}


void cfg_addcccamclient(struct cccamserver_data *cccam, struct cc_client_data *cli)
{
	struct cc_client_data *tmp = cccam->client;
	cli->next = NULL;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = cli;
	} else cccam->client = cli;
}

void cfg_addcccamserver(struct config_data *cfg, struct cccamserver_data *srv)
{
	struct cccamserver_data *tmp = cfg->cccam.server;
	srv->next = NULL;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = srv;
	}
	else cfg->cccam.server = srv;
}


void cfg_addcachepeer(struct config_data *cfg, struct cachepeer_data *peer)
{
	struct cachepeer_data *tmp = cfg->cache.peer;
	peer->next = NULL;
	if (tmp) {
		while (tmp->next) tmp = tmp->next;
		tmp->next = peer;
	} else cfg->cache.peer = peer;
}



///////////////////////////////////////////////////////////////////////////////
// FILE LIST
///////////////////////////////////////////////////////////////////////////////

struct filename_data *add_filename( struct config_data *cfg, char *name)
{
	struct filename_data *tmp = cfg->files;
	// Search
	while (tmp) {
		if ( !strcmp(tmp->name, name) ) return tmp;
		tmp = tmp->next;
	}
	// Create
	tmp = malloc( sizeof(struct filename_data) );
	memset( tmp, 0, sizeof(struct filename_data) );
	strcpy( tmp->name, name );
	tmp->next = NULL;
	// ADD
	struct filename_data *last = cfg->files;
	if (last) {
		while (last->next) last = last->next;
		last->next = tmp;
	} else cfg->files = tmp;
	// return
	return tmp;
}

void free_filenames( struct config_data *cfg )
{
	while (cfg->files) {
		struct filename_data *tmp = cfg->files;
		cfg->files = tmp->next;
		free(tmp);
	}
}

///////////////////////////////////////////////////////////////////////////////
// INCLUDE FILES
///////////////////////////////////////////////////////////////////////////////

struct includefile_data
{
	struct includefile_data *next;
	char name[512];
	FILE *fd;
	int nbline;
};

struct includefile_data *newfile(char *name)
{
	FILE *fd = fopen(name,"rt");
	if (fd==NULL) {
		debugf(getdbgflag(DBG_CONFIG,0,0)," file not found '%s'\n",name);
		return NULL;
	} 
	struct includefile_data *data = malloc( sizeof(struct includefile_data) );
	data->fd = fd;
	data->next = NULL;
	strcpy(data->name, name);
	data->nbline = 0;
	debugf(getdbgflag(DBG_CONFIG,0,0)," config: parsing file '%s'\n",name);
	return data;
}



int read_config(struct config_data *cfg)
{
	int i;
	char str[255];
	struct cardserver_data *cardserver = NULL; // Must be pointer for local server pointing
	struct cs_client_data *usr;
	struct cs_server_data *srv;
	int err = 0; // no error
	struct cardserver_data defaultcs;

	struct global_user_data *guser=NULL;

	struct cccamserver_data *cccam = NULL;
	struct mgcamdserver_data *mgcamd = NULL;

	struct includefile_data *file = newfile(config_file);
	if (!file) return -1;
	add_filename( cfg, config_file );

	// Init defaultcs
	init_cardserver(&defaultcs);

	while (1) {
		memset(currentline, 0, sizeof(currentline) );
		if ( !fgets(currentline, 10239, file->fd) ) {
			struct includefile_data *tmp = file->next;
			fclose( file->fd );
			free( file );
			file = tmp;
			if (file) continue;
			else break;
		} else file->nbline++;

		while (1) {
			char *pos;
			// Remove Comments
			pos = currentline;
			while (*pos) { 
				if (*pos=='#') {
					*pos=0;
					break;
				}
				pos++;
			}
			// delete from the end '\r' '\n' ' ' '\t'
			pos = currentline + strlen(currentline) - 1 ;
			while ( (*pos=='\r')||(*pos=='\n')||(*pos=='\t')||(*pos==' ') ) { 
				*pos=0; pos--;
				if (pos<=currentline) break;
			}
			if (*pos=='\\') {
				if ( !fgets(pos, 10239-(pos-currentline), file->fd) ) {
					*pos=0;
					break;
				}
			} else break;
		}
		//printf("%s\n", currentline);
		iparser = currentline;
		parse_spaces();

		if (*iparser==0) continue;

		if (*iparser=='[') {
			iparser++;
			parse_value(str, "\r\n]" );
			if (*iparser!=']') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ']' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;

			cardserver = malloc( sizeof(struct cardserver_data) );
			memcpy( cardserver, &defaultcs, sizeof(struct cardserver_data) );
			// Newcamd Port
			cardserver->newcamd.port = defaultcs.newcamd.port;
			if (defaultcs.newcamd.port) defaultcs.newcamd.port++;
			// Name
			strcpy(cardserver->name, str);
			cfg_addprofile(cfg, cardserver);
			continue;
		}

		if ( (*iparser<'A') || ((*iparser>'Z')&&(*iparser<'a')) || (*iparser>'z') ) continue; // each line iparser with a word
		if (!parse_name(str)) continue;
		uppercase(str);
		err = 0; // no error


		if (!strcmp(str,"INCLUDE")) {
			parse_spaces();
			if (*iparser==':') iparser++;
			parse_spaces();
			if ( parse_quotes('"',str) ) {
				struct includefile_data *new = newfile( str );
				if (new) {
					add_filename( cfg, str );
					new->next = file;
					file = new;
					continue;
				}
			} 
		}

		else if (!strcmp(str,"N")) {
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			struct cs_server_data tsrv;
			memset(&tsrv,0,sizeof(struct cs_server_data) );
			tsrv.type = TYPE_NEWCAMD;
			parse_str(str);
			tsrv.host = add_host(cfg, str);
			parse_int(str);
			tsrv.port = atoi( str );
			int lastport = 0;
			// Check for CWS
			parse_spaces();
			if (*iparser==':') {
				iparser++;
				if ( parse_int(str) ) lastport = atoi( str );
			} else lastport = tsrv.port;
			parse_str(tsrv.user);
			parse_str(tsrv.pass);
			for(i=0; i<14; i++)
				if ( parse_hex(str)!=2 ) {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Error reading DES-KEY\n",file->nbline,iparser-currentline);
					err++;
					break;
				} 
				else {
					tsrv.key[i] = hex2int(str);
				}
			if (err) {
				continue;
			}

			if (lastport) parse_server_data( &tsrv );

			tsrv.handle = INVALID_SOCKET;

			// Create Servers
			for (i=tsrv.port; i<=lastport; i++) {
				struct cs_server_data *srv = malloc( sizeof(struct cs_server_data) );
				memcpy(srv,&tsrv,sizeof(struct cs_server_data) );
				srv->port = i;
				pthread_mutex_init( &srv->lock, NULL );
				cfg_addserver(cfg, srv);
			}
		}

#ifdef RADEGAST_CLI
		//R: host port caid providers
		else if (!strcmp(str,"R")) {
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			srv = malloc( sizeof(struct cs_server_data) );
			memset(srv,0,sizeof(struct cs_server_data) );
			srv->type = TYPE_RADEGAST;
			parse_str(str);
			srv->host = add_host(cfg, str);
			parse_int(str);
			srv->port = atoi( str );
			// Card
			struct cs_card_data *card = malloc( sizeof(struct cs_card_data) );
			memset(card, 0, sizeof(struct cs_card_data) );
			srv->card = card;
			parse_hex(str);
			card->caid = hex2int( str );
			card->nbprov = 0;
			card->uphops = 1;
			for(i=0;i<CARD_MAXPROV;i++) {
				if ( parse_hex(str)>0 ) {
					card->prov[i] = hex2int( str );
					card->nbprov++;
				} else break;
				parse_spaces();
				if (*iparser==',') iparser++;
			}
			srv->handle = INVALID_SOCKET;
			pthread_mutex_init( &srv->lock, NULL );
			cfg_addserver(cfg, srv);
		}
#endif
		else if (!strcmp(str,"NEWCAMD")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"CLIENTID")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_hex(str);
				cfg->newcamdclientid = hex2int(str);
			}
		}

#ifdef HTTP_SRV
		else if (!strcmp(str,"HTTP")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"PORT")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_int(str);
				//debugf(getdbgflag(DBG_CONFIG,0,0),"http port:%s\n", str);
				parse_spaces();
				if (!cardserver) cfg->http.port = atoi(str);
				else debugf(getdbgflag(DBG_CONFIG,0,0)," config: skip http port, defined within profile\n");
			}
			else if (!strcmp(str,"USER")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_str(cfg->http.user);
			}
			else if (!strcmp(str,"PASS")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_str(cfg->http.pass);
			}
			else if (!strcmp(str,"EDITOR")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"OFF")) cfg->http.noeditor = 1;
				else if (!strcmp(str,"ON")) cfg->http.noeditor = 0;
			}
			else if (!strcmp(str,"RESTART")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"OFF")) cfg->http.norestart = 1;
				else if (!strcmp(str,"ON")) cfg->http.norestart = 0;
			}
			else if (!strcmp(str,"AUTOREFRESH")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_int(str);
				uppercase(str);
				if (!strcmp(str,"OFF")) cfg->http.autorefresh = 0;
				else {
					cfg->http.autorefresh = atoi(str);
					if (cfg->http.autorefresh<0) cfg->http.autorefresh = 0;
					else if (cfg->http.autorefresh>600) cfg->http.autorefresh = 600;
				}
			}
		}
#endif

		else if (!strcmp(str,"BAD-DCW")) {
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			struct dcw_data *dcw = malloc( sizeof(struct dcw_data) );
			memset(dcw,0,sizeof(struct dcw_data) );
			int error = 0;
			for(i=0; i<16; i++)
				if ( parse_hex(str)!=2 ) {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Error reading BAD-DCW\n",file->nbline,iparser-currentline);
					error++;
					break;
				} 
				else {
					dcw->dcw[i] = hex2int(str);
				}
			if (error)
				free(dcw);
			else {
				dcw->next = cfg->bad_dcw;
				cfg->bad_dcw = dcw;
			}
		}


#ifdef TESTCHANNEL
		else if (!strcmp(str,"TESTCHANNEL")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"CAID")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_hex(str);
				cfg->testchn.caid = hex2int(str);
			}
			else if (!strcmp(str,"PROVIDER")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_hex(str);
				cfg->testchn.prov = hex2int(str);
			}
			else if (!strcmp(str,"SID")) {
				parse_spaces();
				if (*iparser!=':') {
					printf("':' expected\n");
					continue;
				} else iparser++;
				parse_hex(str);
				cfg->testchn.sid = hex2int(str);
			}
		}
#endif


		else if (!strcmp(str,"FILE")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"CHANNELINFO")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_spaces();
				if ( parse_quotes('"',str) ) strcpy( cfg->channelinfo_file, str );
			}
			else if (!strcmp(str,"PROVIDERINFO")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_spaces();
				if ( parse_quotes('"',str) ) strcpy( cfg->providers_file, str );
			}
			else if (!strcmp(str,"IP2COUNTRY")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_spaces();
				if ( parse_quotes('"',str) ) strcpy( cfg->ip2country_file, str );
			}
			else if (!strcmp(str,"STYLESHEET")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_spaces();
				if ( parse_quotes('"',str) ) strcpy( cfg->stylesheet_file, str );
			}
		}
///////////////////////////////////////////////////////////////////////////////
// DEFAULT
///////////////////////////////////////////////////////////////////////////////


		else if (!strcmp(str,"DEFAULT")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"KEY")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				for(i=0; i<14; i++) if ( parse_hex(str)!=2 ) break; else defaultcs.newcamd.key[i] = hex2int(str);
			}

			else if (!strcmp(str,"DCW")) {
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"TIMEOUT")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue; 
					} else iparser++;
					parse_int(str);
					parse_spaces();
					defaultcs.option.dcw.timeout = atoi(str);
					if (defaultcs.option.dcw.timeout<300) defaultcs.option.dcw.timeout=300;
					else if (defaultcs.option.dcw.timeout>9999) defaultcs.option.dcw.timeout=9999;
				}
				else if (!strcmp(str,"MAXFAILED")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					parse_spaces();
					defaultcs.option.maxfailedecm = atoi(str);
					if (defaultcs.option.maxfailedecm<0) defaultcs.option.maxfailedecm=0;
					else if (defaultcs.option.maxfailedecm>100) defaultcs.option.maxfailedecm=100;
				}
				else if ( (!strcmp(str,"TIME"))||(!strcmp(str,"MINTIME")) ) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					parse_spaces();
					defaultcs.option.dcw.time = atoi(str);
					if (defaultcs.option.dcw.time<0) defaultcs.option.dcw.time=0;
					else if (defaultcs.option.dcw.time>500) defaultcs.option.dcw.time=500;
				}
#ifdef CHECK_NEXTDCW
				else if (!strcmp(str,"CHECK")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) defaultcs.option.dcw.check = str[0]=='1';
				}
#endif
			}
			else if ( !strcmp(str,"SERVER") ) {
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"TIMEOUT")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.server.timeout = atoi(str);
					if ( (defaultcs.option.server.timeout<300)||(defaultcs.option.server.timeout>10000) ) defaultcs.option.server.timeout=300;
				}
				else if (!strcmp(str,"MAX")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.server.max = atoi(str);
					if (defaultcs.option.server.max<0) defaultcs.option.server.max=0;
					else if (defaultcs.option.server.max>10) defaultcs.option.server.max=10;
				}
				else if (!strcmp(str,"INTERVAL")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.server.interval = atoi(str);
					if (defaultcs.option.server.interval<100) defaultcs.option.server.interval=100;
					else if (defaultcs.option.server.interval>3000) defaultcs.option.server.interval=3000;
				}
				else if (!strcmp(str,"VALIDECMTIME")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.server.validecmtime = atoi(str);
					if (defaultcs.option.server.validecmtime>5000) defaultcs.option.server.validecmtime=5000;
				}
				else if (!strcmp(str,"FIRST")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.server.first = atoi(str);
					if (defaultcs.option.server.first<0) defaultcs.option.server.first = 0;
					else if (defaultcs.option.server.first>5) defaultcs.option.server.first = 5;
				}
				else debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): profile variable expected\n",file->nbline,iparser-currentline);
			}
			else if ( !strcmp(str,"RETRY") ) {
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"NEWCAMD")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.retry.newcamd = atoi(str);
					if (defaultcs.option.retry.newcamd<0) defaultcs.option.retry.newcamd=0;
					else if (defaultcs.option.retry.newcamd>3) defaultcs.option.retry.newcamd=3;
				}
				else if (!strcmp(str,"CCCAM")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.retry.cccam = atoi(str);
					if (defaultcs.option.retry.cccam<0) defaultcs.option.retry.cccam=0;
					else if (defaultcs.option.retry.cccam>10) defaultcs.option.retry.cccam=10;
				}
				else debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): profile variable expected\n",file->nbline,iparser-currentline);
			}
			else if ( !strcmp(str,"CACHE") ) {
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"TIMEOUT")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					parse_int(str);
					defaultcs.option.cachetimeout = atoi(str);
					if (defaultcs.option.cachetimeout<0) defaultcs.option.cachetimeout = 0;
					else if (defaultcs.option.cachetimeout>5000) defaultcs.option.cachetimeout = 5000;
				}
				else if (!strcmp(str,"SENDREQ")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue; 
					} else iparser++;
					parse_name(str);
					uppercase(str);
					if (!strcmp(str,"NO")) defaultcs.option.cachesendreq = 0;
					else if (!strcmp(str,"YES")) defaultcs.option.cachesendreq = 1;
				}
			}
			else if ( !strcmp(str,"ACCEPT") ) {
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"NULL")) {
					parse_name(str);
					uppercase(str);
					if (!strcmp(str,"SID")) {
						if (!cardserver) {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip ACCEPT, undefined profile\n",file->nbline,iparser-currentline);
							continue;
						}
						parse_spaces();
						if (*iparser!=':') {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
							continue;
						} else iparser++;
						if (parse_bin(str)) defaultcs.option.faccept0sid = str[0]=='1';
					}
					else if (!strcmp(str,"CAID")) {
						if (!cardserver) {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip ACCEPT, undefined profile\n",file->nbline,iparser-currentline);
							continue;
						}
						parse_spaces();
						if (*iparser!=':') {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
							continue;
						} else iparser++;
						if (parse_bin(str)) defaultcs.option.faccept0caid = str[0]=='1';
					}
					else if (!strcmp(str,"PROVIDER")) {
						if (!cardserver) {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip ACCEPT, undefined profile\n",file->nbline,iparser-currentline);
							continue;
						}
						parse_spaces();
						if (*iparser!=':') {
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
							continue;
						} else iparser++;
						if (parse_bin(str)) defaultcs.option.faccept0provider = str[0]=='1';
					}
				}
			}
			else if (!strcmp(str,"DISABLE")) {
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"CCCAM")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) defaultcs.option.fallowcccam = str[0]=='0';
				}
				else if (!strcmp(str,"NEWCAMD")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) defaultcs.option.fallownewcamd = str[0]=='0';
				}
				else if (!strcmp(str,"RADEGAST")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) defaultcs.option.fallowradegast = str[0]=='0';
				}
				else if (!strcmp(str,"CACHE")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) defaultcs.option.fallowcache = str[0]=='0';
				}
			}
		}

///////////////////////////////////////////////////////////////////////////////


#ifdef CCCAM_CLI
		else if (!strcmp(str,"C")) {
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			srv = malloc( sizeof(struct cs_server_data) );
			memset(srv,0,sizeof(struct cs_server_data) );
			srv->type = TYPE_CCCAM;
			parse_str(str);
			srv->host = add_host(cfg, str);
			parse_int(str);
			srv->port = atoi( str );
			parse_str(&srv->user[0]);
			parse_str(&srv->pass[0]);
			/// if (parse_int(str)) srv->uphops = atoi( str ); else srv->uphops=1; removed
			parse_server_data( srv );
			srv->handle = INVALID_SOCKET;
			pthread_mutex_init( &srv->lock, NULL );
			cfg_addserver(cfg, srv);
		}
#endif

#ifdef CCCAM_SRV
		else if (!strcmp(str,"F")) {
			if (!cccam) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip F line, undefined CCcam Server\n",file->nbline,iparser-currentline);
				continue;
			}
			// F : user pass <downhops> <dcwtime> <uphops>
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			struct cc_client_data *cli = malloc( sizeof(struct cc_client_data) );
			memset(cli, 0, sizeof(struct cc_client_data) );
			// init Default
			cli->dcwtime = cfg->cccam.dcwtime;
			cli->dnhops = 0;
			cli->uphops = 0;
			// <user> <pass> <dcwtime> <downhops> <uphops> { <CardserverPort>,... }
			parse_str(cli->user);
			parse_str(cli->pass);
			// TODO: Check for same user 
			if (parse_int(str)) {
				cli->dnhops = atoi(str);
				if (parse_int(str)) {
					cli->dcwtime = atoi(str);
					if (cli->dcwtime<0) cli->dcwtime=0;
					else if (cli->dcwtime>500) cli->dcwtime=500;
					if (parse_int(str)) {
						cli->uphops = atoi(str);
					}
				}
			}

			cli->sharelimits[0].caid = 0xFFFF;
			parse_spaces();
			if (*iparser=='{') { // Get Ports List & User Info
				iparser++;
				parse_spaces();
				if ( (*iparser>='0')&&(*iparser<='9') ) {
					i = 0;
					while (i<MAX_CSPORTS) {
						if ( parse_int(str)>0 ) {
							// check for uphops
							if (i==0) {
								parse_spaces();
								if (*iparser==':') {
									iparser++;
									cli->uphops = atoi(str);
									continue;
								}
							}
							// check for port
							cli->csport[i] = atoi(str);
							int j;
							for (j=0; j<i; j++) if ( cli->csport[j] && (cli->csport[j]==cli->csport[i]) ) break;
							if (j>=i) i++; else cli->csport[i] = 0;
						}
						else break;
						parse_spaces();
						if (*iparser==',') iparser++;
					}
				}
				else {
					while (1) {
						parse_spaces();
						if (*iparser=='}') break;
						// NAME1=value1; Name2=Value2 ... }
						parse_value(str,"\r\n\t =");
						//printf(" NAME: '%s'\n", str);
						// '='
						parse_spaces();
						if (*iparser!='=') break;
						iparser++;
						// Value
						// Check for PREDEFINED names
						if (!strcmp(str,"profiles")) {
							i = 0;
							while (i<MAX_CSPORTS) {
								if ( parse_int(str)>0 ) {
									// check for port
									int n = cli->csport[i] = atoi(str);
									int j;
									for (j=0; j<i; j++) if ( cli->csport[j] && (cli->csport[j]==n) ) break;
									if (j>=i) { 
										cli->csport[i] = n;
										i++;
									}
								}
								else break;
								parse_spaces();
								if (*iparser==',') iparser++;
							}
						}
						else if (!strcmp(str,"shares")) {
							int count = 0;
							while ( parse_hex(str)>0 ) {
								cli->sharelimits[count].caid = hex2int(str);
								parse_spaces();
								if (*iparser==':') { // PROVIDER ID
									iparser++;
									if (parse_hex(str)>0) cli->sharelimits[count].provid = hex2int(str); else break; // ERROR
									parse_spaces();
									if (*iparser==':') { // UPHOPS(optional)
										iparser++;
										if (parse_hex(str)>0) cli->sharelimits[count].uphops = hex2int(str); else break; // ERROR
									}
								} else break; // ERROR
								count++;
								if (count>=99) {
									debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): too many share limits...\n",file->nbline,iparser-currentline);
									break;
								}
								parse_spaces();
								if (*iparser==',') iparser++;
							}
							cli->sharelimits[count].caid = 0xFFFF;
						}
						else {
							struct client_info_data *info = malloc( sizeof(struct client_info_data) );
							strcpy(info->name, str);
							parse_spaces();
							parse_value(str,"\r\n;}");
							for(i=strlen(str)-1; ( (str[i]==' ')||(str[i]=='\t') ) ; i--) str[i] = 0; // Remove spaces
							//printf(" VALUE: '%s'\n", str);
							strcpy(info->value, str);
							info->next = cli->info;
							cli->info = info;
						}
						parse_spaces();
						if (*iparser==';') iparser++; else break;
					}
					// Set Info Data
					struct client_info_data *info = cli->info;
					while (info) {
						strcpy(str,info->name);
						uppercase(str);
						if (!strcmp(str,"NAME")) cli->realname = info->value;
						else if ( !strcmp(str,"ENDDATE") || !strcmp(str,"EXPIRE") ) {
							if ( (info->value[4]=='-')&&(info->value[7]=='-') ) strptime(  info->value, "%Y-%m-%d %H", &cli->enddate);
							else if ( (info->value[2]=='-')&&(info->value[5]=='-') ) strptime(  info->value, "%d-%m-%Y %H", &cli->enddate);
 						}
						else if (!strcmp(str,"HOST")) {
							cli->host = add_host(cfg,info->value);
 						}
						info = info->next;
					}
				}
				if (*iparser!='}') debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): '}' expected\n",file->nbline,iparser-currentline);
			}

			cli->handle = -1;
			cfg_addcccamclient(cccam,cli);
		}
#endif

#ifdef CCCAM
		else if (!strcmp(str,"CCCAM")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"VERSION")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_str(str);
				for(i=0; cc_version[i]; i++)
					if ( !strcmp(cc_version[i],str) ) {
						strcpy(cfg->cccam.version,cc_version[i]);
						sprintf(cfg->cccam.build, "%d", cc_build[i]);
						break;
					}
			}

			else if (!strcmp(str,"NODEID")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if ( parse_hex(str)==16 ) hex2array( str, cfg->cccam.nodeid );
			}

#ifdef CCCAM_SRV
			else if (!strcmp(str,"PORT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);

				// Create New
				cccam = malloc( sizeof(struct cccamserver_data) );
				init_cccamserver(cccam);
				cccam->id = cfg->cccam.serverid++;
				cccam->port = atoi(str);
				cccam->next = NULL;
				// Add to config
				cfg_addcccamserver(cfg, cccam);
			}
			else if (!strcmp(str,"PROFILES")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				for(i=0;i<MAX_CSPORTS;i++) {
					if ( parse_int(str)>0 ) {
						cfg->cccam.csport[i] = atoi(str);
					}
					else break;
					parse_spaces();
					if (*iparser==',') iparser++;
				}
			}
			else if (!strcmp(str,"DCWTIME")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cfg->cccam.dcwtime = atoi(str);
			}
#endif
		}
#endif

#ifdef FREECCCAM_SRV
		else if (!strcmp(str,"FREECCCAM")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"PORT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cfg->freecccam.server.port = atoi(str);
			}
			else if (!strcmp(str,"MAXUSERS")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cfg->freecccam.maxusers = atoi(str);
			}
			else if (!strcmp(str,"DCWMINTIME")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cfg->freecccam.dcwtime = atoi(str);
				if (cfg->freecccam.dcwtime<0) cfg->freecccam.dcwtime=0;
				else if (cfg->freecccam.dcwtime>500) cfg->freecccam.dcwtime=500;

			}
			else if (!strcmp(str,"USERNAME")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_str(cfg->freecccam.user);
			}
			else if (!strcmp(str,"PASSWORD")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_str(cfg->freecccam.pass);
			}
			else if (!strcmp(str,"PROFILES")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				for(i=0;i<MAX_CSPORTS;i++) {
					if ( parse_int(str)>0 ) {
						cfg->freecccam.csport[i] = atoi(str);
					}
					else break;
					parse_spaces();
					if (*iparser==',') iparser++;
				}
			}
		}
#endif

#ifdef MGCAMD_SRV
		else if ( !strcmp(str,"MG")||!strcmp(str,"MGUSER") ) {
			if (!mgcamd) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip MGUSER, undefined Mgcamd Server\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			struct mg_client_data *cli = malloc( sizeof(struct mg_client_data) );
			memset(cli, 0, sizeof(struct mg_client_data) );
			// init Default
			cli->dcwtime = 0;

			// <user> <pass> <dcwtime> { <CardserverPort>,... }
			parse_str(cli->user);
			parse_str(cli->pass);
			// TODO: Check for same user 
			if (parse_int(str)) {
				cli->dcwtime = atoi(str);
				if (cli->dcwtime<0) cli->dcwtime=0;
				else if (cli->dcwtime>500) cli->dcwtime=500;
			}

			cli->sharelimits[0].caid = 0xFFFF;
			parse_spaces();
			if (*iparser=='{') { // Get Ports List & User Info
				iparser++;
				parse_spaces();
				if ( (*iparser>='0')&&(*iparser<='9') ) {
					i = 0;
					while (i<MAX_CSPORTS) {
						if ( parse_int(str)>0 ) {
							// check for port
							cli->csport[i] = atoi(str);
							int j;
							for (j=0; j<i; j++) if ( cli->csport[j] && (cli->csport[j]==cli->csport[i]) ) break;
							if (j>=i) i++; else cli->csport[i] = 0;
						}
						else break;
						parse_spaces();
						if (*iparser==',') iparser++;
					}
				}
				else {
					while (1) {
						parse_spaces();
						if (*iparser=='}') break;
						// NAME1=value1; Name2=Value2 ... }
						parse_value(str,"\r\n\t =");
						//printf(" NAME: '%s'\n", str);
						// '='
						parse_spaces();
						if (*iparser!='=') break;
						iparser++;
						// Value
						// Check for PREDEFINED names
						if (!strcmp(str,"profiles")) {
							i = 0;
							while (i<MAX_CSPORTS) {
								if ( parse_int(str)>0 ) {
									// check for port
									int n = cli->csport[i] = atoi(str);
									int j;
									for (j=0; j<i; j++) if ( cli->csport[j] && (cli->csport[j]==n) ) break;
									if (j>=i) { 
										cli->csport[i] = n;
										i++;
									}
								}
								else break;
								parse_spaces();
								if (*iparser==',') iparser++;
							}
						}
						else if (!strcmp(str,"shares")) {
							int count = 0;
							while ( parse_hex(str)>0 ) {
								cli->sharelimits[count].caid = hex2int(str);
								parse_spaces();
								if (*iparser==':') { // PROVIDER ID
									iparser++;
									if (parse_hex(str)>0) cli->sharelimits[count].provid = hex2int(str); else break; // ERROR
									parse_spaces();
									if (*iparser==':') { // UPHOPS(optional)
										iparser++;
										if (parse_hex(str)>0) cli->sharelimits[count].uphops = hex2int(str); else break; // ERROR
									}
								} else break; // ERROR
								count++;
								if (count>=99) {
									debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): too many share limits...\n",file->nbline,iparser-currentline);
									break;
								}
								parse_spaces();
								if (*iparser==',') iparser++;
							}
							cli->sharelimits[count].caid = 0xFFFF;
						}
						else {
							struct client_info_data *info = malloc( sizeof(struct client_info_data) );
							strcpy(info->name, str);
							parse_spaces();
							parse_value(str,"\r\n;}");
							for(i=strlen(str)-1; ( (str[i]==' ')||(str[i]=='\t') ) ; i--) str[i] = 0; // Remove spaces
							//printf(" VALUE: '%s'\n", str);
							strcpy(info->value, str);
							info->next = cli->info;
							cli->info = info;
						}
						parse_spaces();
						if (*iparser==';') iparser++; else break;
					}
					// Set Info Data
					struct client_info_data *info = cli->info;
					while (info) {
						strcpy(str,info->name);
						uppercase(str);
						if (!strcmp(str,"NAME")) cli->realname = info->value;
						else if ( !strcmp(str,"ENDDATE") || !strcmp(str,"EXPIRE") ) {
							if (info->value[4]=='-') strptime(  info->value, "%Y-%m-%d %H", &cli->enddate);
							else strptime(  info->value, "%d-%m-%Y:%H", &cli->enddate);
 						}
						else if (!strcmp(str,"HOST")) cli->host = add_host(cfg,info->value);
						info = info->next;
					}
				}	
				if (*iparser!='}') debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): '}' expected\n",file->nbline,iparser-currentline);
			}

			cli->handle = -1;
			cfg_addmgcamdclient(mgcamd,cli);
		}
		else if (!strcmp(str,"MGCAMD")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"PORT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				// Create New
				mgcamd = malloc( sizeof(struct mgcamdserver_data) );
				init_mgcamdserver(mgcamd);
				mgcamd->id = cfg->mgcamd.serverid++;
				mgcamd->port = atoi(str);
				mgcamd->next = NULL;
				// Add to config
				cfg_addmgcamdserver(cfg, mgcamd);
			}
			else if (!strcmp(str,"KEY")) {
				if (!mgcamd) {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip MGCAMD KEY, undefined Mgcamd Server\n",file->nbline,iparser-currentline);
					continue;
				}
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				for(i=0; i<14; i++)
					if ( parse_hex(str)!=2 ) {
						memcpy( mgcamd->key, defdeskey, 14);
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Error reading DES-KEY\n",file->nbline,iparser-currentline);
						break;
					} 
					else {
						mgcamd->key[i] = hex2int(str);
					}
			}
			else if (!strcmp(str,"PROFILES")) {
				if (!mgcamd) {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip MGCAMD PROFILES, undefined Mgcamd Server\n",file->nbline,iparser-currentline);
					continue;
				}
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				for(i=0;i<MAX_CSPORTS;i++) {
					if ( parse_int(str)>0 ) {
						mgcamd->csport[i] = atoi(str);
					}
					else break;
					parse_spaces();
					if (*iparser==',') iparser++;
				}
			}
		}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

		else if (!strcmp(str,"CACHE")) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"PORT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				parse_spaces();
				cfg->cache.port = atoi(str);
			}
			else if (!strcmp(str,"PEER")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				// must check for reuse of same user
				struct cachepeer_data *peer = malloc( sizeof(struct cachepeer_data) );
				memset(peer,0,sizeof(struct cachepeer_data) );
				peer->fsendrep = 0;
				parse_name(str);
				peer->host = add_host(cfg, str);
				parse_spaces();
				if ( *(iparser)==':' ) iparser++;
				parse_int(str);
				peer->port = atoi(str);
				if (parse_bin(str)) {
					peer->fblock0onid = str[0]=='1';
					if (parse_int(str)) {
						peer->fsendrep = str[0]=='1';
					}
				}
				parse_spaces();
				if (*iparser=='{') { // Get Ports List
					iparser++;
					for(i=0;i<MAX_CSPORTS;i++) {
						if ( parse_int(str)>0 ) {
							peer->csport[i] = atoi(str);
						}
						else break;
						parse_spaces();
						if (*iparser==',') iparser++;
					}
					peer->csport[i] = 0;
					parse_spaces();
					if (*iparser!='}') debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): '}' expected\n",file->nbline,iparser-currentline);
				}
				peer->reqnb=0;
				peer->reqnb=0;
				peer->hitnb=0;
				cfg_addcachepeer(cfg, peer);
			}
			else if (!strcmp(str,"TIMEOUT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				if (cardserver) {
					cardserver->option.cachetimeout = atoi(str);
					if (cardserver->option.cachetimeout<0) cardserver->option.cachetimeout = 0;
					else if (cardserver->option.cachetimeout>5000) cardserver->option.cachetimeout = 5000;
				}
				else {
					defaultcs.option.cachetimeout = atoi(str);
					if (defaultcs.option.cachetimeout<0) defaultcs.option.cachetimeout = 0;
					else if (defaultcs.option.cachetimeout>5000) defaultcs.option.cachetimeout=5000;
				}
			}
			else if (!strcmp(str,"TRACKER")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue; 
				} else iparser++;
				if (parse_bin(str)) cfg->cache.tracker = str[0]=='1';
			}
			else if (!strcmp(str,"SENDREQ")) {
				if (!cardserver) {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip CACHE RESENDREQ, undefined profile\n",file->nbline,iparser-currentline);
					continue;
				}
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue; 
				} else iparser++;
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"NO")) cardserver->option.cachesendreq = 0;
				else if (!strcmp(str,"YES")) cardserver->option.cachesendreq = 1;
			}
		}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

		else if (!strcmp(str,"CAID")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip CAID, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			parse_hex(str);
			cardserver->card.caid = hex2int( str );
			//debugf(getdbgflag(DBG_CONFIG,0,0)," *caid %04X\n",cardserver->card.caid);
		}
		else if (!strcmp(str,"PROVIDERS")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip PROVIDERS, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			cardserver->card.nbprov = 0;
			for(i=0;i<16;i++) {
				if ( parse_hex(str)>0 ) {
					cardserver->card.prov[i] = hex2int( str );
					cardserver->card.nbprov++;
				}
				else break;
				parse_spaces();
				if (*iparser==',') iparser++;
			}
			//for(i=0;i<cardserver->card.nbprov;i++) debugf(getdbgflag(DBG_CONFIG,0,0)," *provider %d = %06X\n",i,cardserver->card.prov[i]); 
		}

		else if (!strcmp(str,"USER")) {
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;

			if (cardserver) {
				// must check for reuse of same user
				usr = malloc( sizeof(struct cs_client_data) );
				memset(usr,0,sizeof(struct cs_client_data) );
				cs_addnewcamdclient(cardserver, usr);
				usr->handle = INVALID_SOCKET;
				usr->flags |= FLAG_DEFCONFIG;
				parse_str(&usr->user[0]);
				parse_str(&usr->pass[0]);
				// options
				usr->dcwtime = cardserver->option.dcw.time;
				usr->dcwtimeout = cardserver->option.dcw.timeout;
#ifdef DCWPERIOD
				usr->dcwperiod = cardserver->dcwperiod;
#endif
				// USER: name pass dcwtime dcwtimeout dcwperiod { CAID:PROVIDER1,PROVIDER2,... } 

				//dcwtime
				if (parse_int(str)) {
					usr->dcwtime = atoi( str ); 
					if (usr->dcwtime<0) usr->dcwtime=0;
					else if (usr->dcwtime>500) usr->dcwtime=500;
					//dcwtimeout
					if (parse_int(str)) {
						usr->dcwtimeout = atoi( str ); 
						if (usr->dcwtimeout>10000) usr->dcwtimeout=10000;
						//dcwperiod
#ifdef DCWPERIOD
						if (parse_int(str)) {
							usr->dcwperiod = atoi( str ); 
							if (usr->dcwperiod>5000) usr->dcwperiod=5000;
						}
#endif
					}
				}
				if ( parse_expect('{') ) { // Read Caid Providers
					if (parse_hex(str)) {
						usr->card.caid = hex2int(str);
						if (!parse_expect(':')) {
							usr->card.caid = 0;
							debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
							continue;
						}
						usr->card.nbprov = 0;
						for(i=0;i<16;i++) {
							if ( parse_hex(str)>0 ) {
								usr->card.prov[i] = hex2int( str );
								usr->card.nbprov++;
							}
							else break;
							parse_spaces(); if (*iparser==',') iparser++;
						}
					}
					if (!parse_expect('}')) {
						usr->card.caid = 0;
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): '}' expected\n",file->nbline,iparser-currentline);
						continue;
					}
				}
			}
			else { // global user
				struct global_user_data *gl = malloc( sizeof(struct global_user_data) );
				memset(gl,0,sizeof(struct global_user_data) );
				/// ADD
				struct global_user_data *tmp = guser;
				if (tmp) {
					while (tmp->next) tmp = tmp->next;
					tmp->next = gl;
				} else guser = gl;
				gl->next = NULL;
				//
				parse_str(&gl->user[0]);
				parse_str(&gl->pass[0]);
				// user: username pass { csports }
				parse_spaces();
				if (*iparser=='{') { // Get Ports List
					iparser++;
					for(i=0;i<MAX_CSPORTS;i++) {
						if ( parse_int(str)>0 ) {
							gl->csport[i] = atoi(str);
						}
						else break;
						parse_spaces();
						if (*iparser==',') iparser++;
					}
					gl->csport[i] = 0;
					parse_spaces();
					if (*iparser!='}') debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): '}' expected\n",file->nbline,iparser-currentline);
				}
			}
		}

		else if (!strcmp(str,"DCW")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip DCW, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"TIMEOUT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue; 
				} else iparser++;
				parse_int(str);
				parse_spaces();
				cardserver->option.dcw.timeout = atoi(str);
				if (cardserver->option.dcw.timeout<300) cardserver->option.dcw.timeout=300;
				else if (cardserver->option.dcw.timeout>9999) cardserver->option.dcw.timeout=9999;
			}
			else if (!strcmp(str,"MAXFAILED")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				parse_spaces();
				cardserver->option.maxfailedecm = atoi(str);
				if (cardserver->option.maxfailedecm<0) cardserver->option.maxfailedecm=0;
				else if (cardserver->option.maxfailedecm>100) cardserver->option.maxfailedecm=100;
			}
			else if ( (!strcmp(str,"TIME"))||(!strcmp(str,"MINTIME")) ) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				parse_spaces();
				cardserver->option.dcw.time = atoi(str);
				if (cardserver->option.dcw.time<0) cardserver->option.dcw.time=0;
				else if (cardserver->option.dcw.time>500) cardserver->option.dcw.time=500;
			}
#ifdef DCWPERIOD
			else if (!strcmp(str,"PERIOD")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				parse_spaces();
				cardserver->dcwperiod = atoi(str);
				if (cardserver->dcwperiod>5000) cardserver->dcwperiod=5000;
			}
			else if (!strcmp(str,"ALGORITHM")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				parse_spaces();
				cardserver->dcwalgorithm = atoi(str);
				if (cardserver->dcwalgorithm>2) cardserver->dcwalgorithm=0;
			}
#endif

#ifdef CHECK_NEXTDCW
			else if (!strcmp(str,"CHECK")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if (parse_bin(str)) cardserver->option.dcw.check = str[0]=='1';
			}
#endif
		}

		else if (!strcmp(str,"SERVER")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip SERVER, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"TIMEOUT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->option.server.timeout = atoi(str);
				if ( (cardserver->option.server.timeout<300)||(cardserver->option.server.timeout>10000) ) cardserver->option.server.timeout=300;
			}
			else if (!strcmp(str,"MAX")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->option.server.max = atoi(str);
				if (cardserver->option.server.max<0) cardserver->option.server.max=0;
				else if (cardserver->option.server.max>10) cardserver->option.server.max=10;
			}
			else if (!strcmp(str,"INTERVAL")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->option.server.interval = atoi(str);
				if (cardserver->option.server.interval<100) cardserver->option.server.interval=100;
				else if (cardserver->option.server.interval>3000) cardserver->option.server.interval=3000;
			}
			else if (!strcmp(str,"VALIDECMTIME")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->option.server.validecmtime = atoi(str);
				if (cardserver->option.server.validecmtime>5000) cardserver->option.server.validecmtime=5000;
			}
			else if (!strcmp(str,"FIRST")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->option.server.first = atoi(str);
				if (cardserver->option.server.first<0) cardserver->option.server.first = 0;
				else if (cardserver->option.server.first>5) cardserver->option.server.first = 5;
			}
			else debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): cardserver variable expected\n",file->nbline,iparser-currentline);
		}

		else if ( !strcmp(str,"RETRY") ) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip RETRY, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"NEWCAMD")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->option.retry.newcamd = atoi(str);
				if (cardserver->option.retry.newcamd<0) cardserver->option.retry.newcamd=0;
				else if (cardserver->option.retry.newcamd>3) cardserver->option.retry.newcamd=3;
			}
			else if (!strcmp(str,"CCCAM")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->option.retry.cccam = atoi(str);
				if (cardserver->option.retry.cccam<0) cardserver->option.retry.cccam=0;
				else if (cardserver->option.retry.cccam>10) cardserver->option.retry.cccam=10;
			}
			else debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): cardserver variable expected\n",file->nbline,iparser-currentline);
		}

		else if (!strcmp(str,"PORT")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip PORT, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			parse_spaces();
			if (*iparser=='+') {
				iparser++;
				//cardserver->newcamd.port = defaultcs.newcamd.port;
				//defaultcs.newcamd.port++;
			}
			else {
				parse_int(str);
				//debugf(getdbgflag(DBG_CONFIG,0,0),"port:%s\n", str);
				parse_spaces();
				cardserver->newcamd.port = atoi(str);
				defaultcs.newcamd.port = cardserver->newcamd.port;
				if (defaultcs.newcamd.port) defaultcs.newcamd.port++;
			}			
		}
#ifdef RADEGAST_SRV
		else if (!strcmp(str,"RADEGAST")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip PORT, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"PORT")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				parse_int(str);
				cardserver->rdgd.port = atoi(str);
			}
		}
#endif
		else if (!strcmp(str,"ONID")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip ONID, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			parse_hex(str);
			cardserver->option.onid = hex2int(str);
		}

		else if ( !strcmp(str,"ACCEPT") ) {
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"NULL")) {
				parse_name(str);
				uppercase(str);
				if (!strcmp(str,"SID")) {
					if (!cardserver) {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip ACCEPT, undefined profile\n",file->nbline,iparser-currentline);
						continue;
					}
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) cardserver->option.faccept0sid = str[0]=='1';
				}
				else if (!strcmp(str,"CAID")) {
					if (!cardserver) {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip ACCEPT, undefined profile\n",file->nbline,iparser-currentline);
						continue;
					}
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) cardserver->option.faccept0caid = str[0]=='1';
				}
				else if (!strcmp(str,"PROVIDER")) {
					if (!cardserver) {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip ACCEPT, undefined profile\n",file->nbline,iparser-currentline);
						continue;
					}
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) cardserver->option.faccept0provider = str[0]=='1';
				}

				else if (!strcmp(str,"ONID")) {
					parse_spaces();
					if (*iparser!=':') {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
						continue;
					} else iparser++;
					if (parse_bin(str)) cfg->cache.faccept0onid = str[0]=='1';
				}
			}
		}

		else if (!strcmp(str,"DISABLE")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip DISABLE, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"CCCAM")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if (parse_bin(str)) cardserver->option.fallowcccam = str[0]=='0';
			}
			else if (!strcmp(str,"NEWCAMD")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if (parse_bin(str)) cardserver->option.fallownewcamd = str[0]=='0';
			}
			else if (!strcmp(str,"RADEGAST")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if (parse_bin(str)) cardserver->option.fallowradegast = str[0]=='0';
			}
			else if (!strcmp(str,"CACHE")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if (parse_bin(str)) cardserver->option.fallowcache = str[0]=='0';
			}
		}

		else if (!strcmp(str,"SID")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip SID, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_name(str);
			uppercase(str);
			if (!strcmp(str,"LIST")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if (cardserver->sids) {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): sids already defined!!!\n",file->nbline,iparser-currentline);
					continue;
				}
				int count = 0;
				struct sid_chid_data *sids;
				sids = malloc ( sizeof(struct sid_chid_data) * MAX_SIDS );
				memset( sids, 0, sizeof(struct sid_chid_data) * MAX_SIDS );
				cardserver->sids = sids;
				while ( parse_hex(str)>0 ) {
					sids->sid = hex2int(str);
					parse_spaces();
					if (*iparser==':') { // CHID
						iparser++;
						if (parse_hex(str)>0) sids->chid = hex2int(str);
					}
					//debugf(getdbgflag(DBG_CONFIG,0,0),"sid%d %s\n", count, str);
					count++;
					sids++;
					if (count>=MAX_SIDS) {
						debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): too many sids...\n",file->nbline,iparser-currentline);
						break;
					}
					parse_spaces();
					if (*iparser==',') iparser++;
				}
				sids->sid = 0;
				sids->chid = 0;
			}
	
			else if (!strcmp(str,"DENYLIST")) {
				parse_spaces();
				if (*iparser!=':') {
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
					continue;
				} else iparser++;
				if (parse_bin(str)) cardserver->isdeniedsids = str[0]=='1';
			}
		}

		else if (!strcmp(str,"KEY")) {
			if (!cardserver) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Skip KEY, undefined profile\n",file->nbline,iparser-currentline);
				continue;
			}
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			for(i=0; i<14; i++)
				if ( parse_hex(str)!=2 ) {
					memset( cardserver->newcamd.key, 0, 16 );
					debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Error reading DES-KEY\n",file->nbline,iparser-currentline);
					break;
				} 
				else {
					cardserver->newcamd.key[i] = hex2int(str);
				}
		}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

		else if (!strcmp(str,"TRACE")) {
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): ':' expected\n",file->nbline,iparser-currentline);
				continue;
			} else iparser++;
			
			if ( parse_bin(str)!=1 ) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): Error reading Trace\n",file->nbline,iparser-currentline);
				break;
			} 
			if (str[0]=='0') flag_debugtrace = 0;
			else if (str[0]=='1') {
				parse_str(trace.host);
				parse_int(str);
				trace.port = atoi(str);
				trace.ip = hostname2ip(trace.host);
				memset( &trace.addr, 0, sizeof(trace.addr) );
				trace.addr.sin_family = AF_INET;
				trace.addr.sin_addr.s_addr = trace.ip;
				trace.addr.sin_port = htons(trace.port);
				flag_debugtrace = 0;
				if (trace.port && trace.ip) {
					if (trace.sock<=0) trace.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
					if (trace.sock>0) flag_debugtrace = 1;
				}
			}
			else {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): expected 0/1\n",file->nbline,iparser-currentline);
				break;
			}
		}
	}

#ifdef FREECCCAM_SRV
	//Create clients
	for(i=0; i< cfg->freecccam.maxusers; i++) {
		struct cc_client_data *cli = malloc( sizeof(struct cc_client_data) );
		memset(cli, 0, sizeof(struct cc_client_data) );
		// init Default
		cli->handle = INVALID_SOCKET;
		cli->dcwtime = cfg->freecccam.dcwtime;
		cli->dnhops = 0;
		cli->uphops = 0;
		cli->next = cfg->freecccam.server.client;
		cfg->freecccam.server.client = cli;
	}
#endif

// ADD GLOBAL USERS TO PROFILES
	struct global_user_data *gl = guser;
	while(gl) {
		struct cardserver_data *cs = cfg->cardserver;
		while(cs) {
			int i;
			for(i=0; i<MAX_CSPORTS; i++ ) {
				if (!gl->csport[i]) break;
				if (gl->csport[i]==cs->newcamd.port) {
					i=0;
					break;
				}
			}
			if (i==0) { // ADD TO PROFILE
				// must check for reuse of same user
				struct cs_client_data *usr = malloc( sizeof(struct cs_client_data) );
				memset(usr,0,sizeof(struct cs_client_data) );
				cs_addnewcamdclient(cs, usr);
				usr->handle = INVALID_SOCKET;
				usr->flags |= FLAG_DEFCONFIG;
				strcpy(usr->user, gl->user);
				strcpy(usr->pass, gl->pass);
				// options
				usr->dcwtime = cs->option.dcw.time;
				usr->dcwtimeout = cs->option.dcw.timeout;
#ifdef DCWPERIOD
				usr->dcwperiod = cs->dcwperiod;
#endif
				// Setup global id
				 //usr->gid = cfg-
			}
			cs = cs->next;
		}
		struct global_user_data *oldgl = gl;
		gl = gl->next;
		free( oldgl );
	}

	read_chinfo(cfg);
	read_providers(cfg);
	read_ip2country(cfg);

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int read_chinfo( struct config_data *cfg )
{
	if (!cfg->channelinfo_file[0]) return 0;

	FILE *fhandle;
	char str[128];

	uint16 caid,sid;
	uint32 prov;
	int chncount = 0;

	int nbline = 0;
	// Open Config file
	fhandle = fopen(cfg->channelinfo_file,"rt");
	if (fhandle==0) {
		debugf(getdbgflag(DBG_CONFIG,0,0)," config: file not found '%s'\n",cfg->channelinfo_file);
		return -1;
	} else debugf(getdbgflag(DBG_CONFIG,0,0)," config: parsing file '%s'\n",cfg->channelinfo_file);

	while (!feof(fhandle))
	{
		if ( !fgets(currentline, 10239, fhandle) ) break;
		iparser = &currentline[0];
		nbline++;
		parse_spaces();

		if ( ((*iparser>='0')&&(*iparser<='9')) || ((*iparser>='a')&&(*iparser<='f')) || ((*iparser>='A')&&(*iparser<='f')) ) {
			parse_hex(str);
			caid = hex2int(str);
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): caid ':' expected\n",nbline,iparser-currentline);
				continue;
			} else iparser++;
			parse_hex(str);
			prov = hex2int(str);
			parse_spaces();
			if (*iparser!=':') {
				debugf(getdbgflag(DBG_CONFIG,0,0)," config(%d,%d): provid ':' expected\n",nbline,iparser-currentline);
				continue;
			} else iparser++;
			parse_hex(str);
			sid = hex2int(str);
			parse_spaces();
			if (*iparser=='"') {
				iparser++;
				char *end = iparser;
				while ( (*end!='"')&&(*end!='\n')&&(*end!='\r')&&(*end!=0) ) end++;
				if (end-iparser) {
					*end = 0;
					//strcpy(str,iparser);
					//debugf(getdbgflag(DBG_CONFIG,0,0),"%04x:%06x:%04x '%s'\n",caid,prov,sid,iparser);
					struct chninfo_data *chn = malloc( sizeof(struct chninfo_data) + strlen(iparser) + 1 );
					chn->sid = sid;
					chn->caid = caid;
					chn->prov = prov;
					strcpy( chn->name, iparser );
					chn->next = cfg->chninfo;
					cfg->chninfo = chn;
					chncount++;
				}
			}
		}
	}
	fclose(fhandle);
	debugf(getdbgflag(DBG_CONFIG,0,0)," config: reading %d channels.\n", chncount);
	return 0;
}

void free_chinfo( struct config_data *cfg )
{
	if (cfg->chninfo) {
		struct chninfo_data *current = cfg->chninfo;
		cfg->chninfo = NULL;
		usleep(100000);
		while (current) {
			struct chninfo_data *next = current->next;
			free(current);
			current = next;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

int read_ip2country( struct config_data *cfg )
{
	if (!cfg->ip2country_file[0]) return 0;
	FILE *fhandle;
	int linecount = 0;

	int nbline = 0;
	// Open Config file
	fhandle = fopen(cfg->ip2country_file,"rt");
	if (fhandle==0) {
		debugf(getdbgflag(DBG_CONFIG,0,0)," config: file not found '%s'\n",cfg->ip2country_file);
		return -1;
	} else debugf(getdbgflag(DBG_CONFIG,0,0)," config: parsing file '%s'\n",cfg->ip2country_file);

	while (!feof(fhandle)) 	{
		if ( !fgets(currentline, 10239, fhandle) ) break;
		iparser = &currentline[0];
		nbline++;

		char value[20][256];
		int nbvalues = 0;
		int icode = -1;
		while (*iparser=='"') {
			iparser++;
			// Copy IP address
			char *p = value[nbvalues];
			while ( (*iparser!='"')&&(*iparser!='\n')&&(*iparser!='\r')&&(*iparser!=0) ) {
				*p = *iparser;
				iparser++;
				p++;
			}
			*p = 0;
			if (*iparser!='"') break; // break;
			iparser++;
			if (strlen(value[nbvalues])==2) icode = nbvalues;
//			printf("(%d) '%s'\n",nbvalues+1,value[nbvalues]);
			nbvalues++;
			// Comma
			parse_spaces();
			if (*iparser!=',') break;
			iparser++;
			parse_spaces();
		}
		if ( (nbvalues<3)||(icode==-1) ) continue;

		// ADD TO DB
		struct ip2country_data *data = malloc( sizeof(struct ip2country_data) );
		memset( data, 0, sizeof(struct ip2country_data) );
		unsigned int ip4[4];
		if (sscanf(value[0],"%d.%d.%d.%d", &ip4[0], &ip4[1], &ip4[2], &ip4[3])==4) {
			data->ipstart = (ip4[0]<<24)|(ip4[1]<<16)|(ip4[2]<<8)|(ip4[3]);
			if (sscanf(value[1],"%d.%d.%d.%d", &ip4[0], &ip4[1], &ip4[2], &ip4[3])==4) {
				data->ipend = (ip4[0]<<24)|(ip4[1]<<16)|(ip4[2]<<8)|(ip4[3]);
				strcpy(data->code, value[icode]);
				data->next = cfg->ip2country;
				cfg->ip2country = data;
				linecount++;
				continue; // OK
			}
		}
		//ERROR
		free( data );
	}
	fclose(fhandle);
	debugf(getdbgflag(DBG_CONFIG,0,0)," config: reading %d valid ip2country entries.\n", linecount);
	return 0;
}


void free_ip2country( struct config_data *cfg )
{
	if (cfg->ip2country) {
		struct ip2country_data *current = cfg->ip2country;
		cfg->ip2country = NULL;
		usleep(100000);
		while (current) {
			struct ip2country_data *next = current->next;
			free(current);
			current = next;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

int read_providers( struct config_data *cfg )
{
	if (!cfg->providers_file[0]) return 0;

	FILE *fhandle;
	char str[128];

	uint32 caprovid;
	int provcount = 0;

	int nbline = 0;
	// Open Config file
	fhandle = fopen(cfg->providers_file,"rt");
	if (fhandle==0) {
		debugf(getdbgflag(DBG_CONFIG,0,0)," config: file not found '%s'\n",cfg->providers_file);
		return -1;
	} else debugf(getdbgflag(DBG_CONFIG,0,0)," config: parsing file '%s'\n",cfg->providers_file);

	while (!feof(fhandle))
	{
		if ( !fgets(currentline, 10239, fhandle) ) break;
		iparser = &currentline[0];
		nbline++;
		parse_spaces();

		if ( parse_hex(str)==8 ) {
			caprovid = hex2int(str);
			parse_spaces();
			if (*iparser=='"') {
				iparser++;
				char *end = iparser;
				while ( (*end!='"')&&(*end!='\n')&&(*end!='\r')&&(*end!=0) ) end++;
				if (end-iparser) {
					*end = 0;
					//strcpy(str,iparser);
					//debugf(getdbgflag(DBG_CONFIG,0,0),"%04x:%06x:%04x '%s'\n",caid,prov,sid,iparser);
					struct providers_data *prov = malloc( sizeof(struct providers_data) + strlen(iparser) + 1 );
					prov->caprovid = caprovid;
					strcpy( prov->name, iparser );
					prov->next = cfg->providers;
					cfg->providers = prov;
					provcount++;
				}
			}
		}
	}
	fclose(fhandle);
	debugf(getdbgflag(DBG_CONFIG,0,0)," config: reading %d providers.\n", provcount);
	return 0;
}

void free_providers( struct config_data *cfg )
{
	if (cfg->providers) {
		struct providers_data *current = cfg->providers;
		cfg->providers = NULL;
		usleep(100000);
		while (current) {
			struct providers_data *next = current->next;
			free(current);
			current = next;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int cardcmp( struct cs_card_data *card1, struct cs_card_data *card2)
{
	if (card1->caid!=card2->caid) return 1;
	if (card1->nbprov!=card2->nbprov) return 1;
	int i;
	for(i=0; i<card1->nbprov; i++) if (card1->prov[i]!=card2->prov[i]) return 1;
	return 0;
}



void remove_cccam_clients(struct cccamserver_data *srv)
{
	while (srv->client) {
		struct cc_client_data *cli = srv->client;
		srv->client = cli->next;
		// FREE 
		if (cli->handle>0) close(cli->handle);
		while (cli->info) {
			struct client_info_data *info = cli->info;
			cli->info = info->next;
			free(info);
		}
		free( cli );
	}
}


void remove_mgcamd_clients(struct mgcamdserver_data *srv)
{
	while (srv->client) {
		struct mg_client_data *cli = srv->client;
		srv->client = cli->next;
		// FREE 
		if (cli->handle>0) close(cli->handle);
		while (cli->info) {
			struct client_info_data *info = cli->info;
			cli->info = info->next;
			free(info);
		}
		free( cli );
	}
}


void remove_newcamd_clients(struct cardserver_data *cs)
{
	while (cs->newcamd.client) {
		struct cs_client_data *cli = cs->newcamd.client;
		cs->newcamd.client = cli->next;
		// FREE 
		if (cli->handle>0) close(cli->handle);
		while (cli->info) {
			struct client_info_data *info = cli->info;
			cli->info = info->next;
			free(info);
		}
		free( cli );
	}
}

//////////////////////////// CCCAM

void update_cccam_clients(struct cccamserver_data *srv, struct cccamserver_data *newsrv )
{
	// set remove flag to old deleted clients & update reused one
	struct cc_client_data *cli = srv->client;
	while (cli) {
		struct cc_client_data *newcli = newsrv->client;
		while (newcli) {
			if ( !(newcli->flags&FLAG_REMOVE) )
			if ( !(cli->flags&FLAG_REMOVE) )
			if ( !strcmp(cli->user, newcli->user) ) break;
			newcli = newcli->next;
		}
		if (newcli) {
			newcli->flags |= FLAG_REMOVE;
			// Update CCcam Client Data
			// PASS
			if ( strcmp(cli->pass, newcli->pass) ) {
				cli->flags |= FLAG_DISCONNECT;
				strcpy(cli->pass, newcli->pass);
			}
			// dnhops
			if (cli->dnhops!=newcli->dnhops) {
				cli->flags |= FLAG_DISCONNECT;
				cli->dnhops = newcli->dnhops;
			}
			// dcwtime
			cli->dcwtime = newcli->dcwtime;
			// csport
			if ( memcmp(cli->csport,newcli->csport,sizeof(cli->csport)) ) {
				cli->flags |= FLAG_DISCONNECT;
				memcpy( cli->csport, newcli->csport, sizeof(cli->csport) );
			}
			// 
			cli->uphops = newcli->uphops;
			cli->shareemus = newcli->shareemus;
			cli->allowemm = newcli->allowemm;
			// Share Limits
			if ( memcmp(cli->sharelimits, newcli->sharelimits, sizeof(cli->sharelimits)) ) {
				cli->flags |= FLAG_DISCONNECT;
				memcpy(cli->sharelimits, newcli->sharelimits, sizeof(cli->sharelimits));
			}
			// info+data
			struct client_info_data *info = cli->info;
			cli->info = newcli->info;
			newcli->info = info;
			cli->realname = newcli->realname;
			memcpy( &cli->enddate, &newcli->enddate, sizeof(struct tm) );
			cli->host = newcli->host;
		}
		else cli->flags |= FLAG_REMOVE;
		cli = cli->next;
	}
	// Move all newcli without FLAG_REMOVE to cli
	struct cc_client_data *prev = NULL;
	struct cc_client_data *newcli = newsrv->client;
	while (newcli) {
		struct cc_client_data *next = newcli->next;
		if (!(newcli->flags&FLAG_REMOVE)) {
			if (prev) prev->next = newcli->next; else newsrv->client = newcli->next;
			cfg_addcccamclient(srv, newcli);
		} else prev = newcli;
		newcli = next;
	}
	// Move all cli with FLAG_REMOVE to newcli
	prev = NULL;
	cli = srv->client;
	while (cli) {
		struct cc_client_data *next = cli->next;
		if (cli->flags&FLAG_REMOVE) {
			if (prev) prev->next = cli->next; else srv->client = cli->next;
			cfg_addcccamclient(newsrv, cli);
		} else prev = cli;
		cli = next;
	}
}


void update_cccam_servers(struct config_data *cfg, struct config_data *newcfg)
{
	struct cccamserver_data *srv = cfg->cccam.server;
	while (srv) {
		struct cccamserver_data *newsrv = newcfg->cccam.server;
		while (newsrv) {
			if (srv->port==newsrv->port) break;
			newsrv = newsrv->next;
		}
		if (newsrv) {
			newsrv->flags |= FLAG_REMOVE;
			update_cccam_clients( srv, newsrv );
		}
		else srv->flags |= FLAG_REMOVE;
		srv = srv->next;
	}
	// Move all newsrv without FLAG_REMOVE to srv
	struct cccamserver_data *prev = NULL;
	srv = newcfg->cccam.server;
	while (srv) {
		struct cccamserver_data *next = srv->next;
		if (!(srv->flags&FLAG_REMOVE)) {
			if (prev) prev->next = srv->next; else newcfg->cccam.server = srv->next;
			cfg_addcccamserver(cfg, srv);
		} else prev = srv;
		srv = next;
	}
	// Move all srv with FLAG_REMOVE to newsrv
	prev = NULL;
	srv = cfg->cccam.server;
	while (srv) {
		struct cccamserver_data *next = srv->next;
		if (srv->flags&FLAG_REMOVE) {
			if (prev) prev->next = srv->next; else cfg->cccam.server = srv->next;
			cfg_addcccamserver(newcfg, srv);
		} else prev = srv;
		srv = next;
	}
}


//////////////////////////// MGCAMD

void update_mgcamd_clients(struct mgcamdserver_data *srv, struct mgcamdserver_data *newsrv )
{
	// set remove flag to old deleted clients & update reused one
	struct mg_client_data *cli = srv->client;
	while (cli) {
		struct mg_client_data *newcli = newsrv->client;
		while (newcli) {
			if ( !(newcli->flags&FLAG_REMOVE) )
			if ( !(cli->flags&FLAG_REMOVE) )
			if ( !strcmp(cli->user, newcli->user) ) break;
			newcli = newcli->next;
		}
		if (newcli) {
			newcli->flags |= FLAG_REMOVE;
			// Update MGcamd Client Data
			// PASS
			if ( strcmp(cli->pass, newcli->pass) ) {
				cli->flags |= FLAG_DISCONNECT;
				strcpy(cli->pass, newcli->pass);
			}
			// dcwtime
			cli->dcwtime = newcli->dcwtime;
			// Share Limits
			if ( memcmp(cli->sharelimits, newcli->sharelimits, sizeof(cli->sharelimits)) ) {
				cli->flags |= FLAG_DISCONNECT;
				memcpy(cli->sharelimits, newcli->sharelimits, sizeof(cli->sharelimits));
			}
			// csport
			if ( memcmp(cli->csport,newcli->csport,sizeof(cli->csport)) ) {
				cli->flags |= FLAG_DISCONNECT;
				memcpy( cli->csport, newcli->csport, sizeof(cli->csport) );
			}
			// info+data
			struct client_info_data *info = cli->info;
			cli->info = newcli->info;
			newcli->info = info;
			cli->realname = newcli->realname;
			memcpy( &cli->enddate, &newcli->enddate, sizeof(struct tm) );
			cli->host = newcli->host;
		}
		else cli->flags |= FLAG_REMOVE;
		cli = cli->next;
	}
	// Move all newcli without FLAG_REMOVE to cli
	struct mg_client_data *prev = NULL;
	struct mg_client_data *newcli = newsrv->client;
	while (newcli) {
		struct mg_client_data *next = newcli->next;
		if (!(newcli->flags&FLAG_REMOVE)) {
			if (prev) prev->next = newcli->next; else newsrv->client = newcli->next;
			cfg_addmgcamdclient(srv, newcli);
		} else prev = newcli;
		newcli = next;
	}
	// Move all cli with FLAG_REMOVE to newcli
	prev = NULL;
	cli = srv->client;
	while (cli) {
		struct mg_client_data *next = cli->next;
		if (cli->flags&FLAG_REMOVE) {
			if (prev) prev->next = cli->next; else srv->client = cli->next;
			cfg_addmgcamdclient(newsrv, cli);
		} else prev = cli;
		cli = next;
	}
}

void update_mgcamd_servers(struct config_data *cfg, struct config_data *newcfg)
{
	struct mgcamdserver_data *srv = cfg->mgcamd.server;
	while (srv) {
		struct mgcamdserver_data *newsrv = newcfg->mgcamd.server;
		while (newsrv) {
			if (srv->port==newsrv->port) break;
			newsrv = newsrv->next;
		}
		if (newsrv) {
			newsrv->flags |= FLAG_REMOVE;
			update_mgcamd_clients( srv, newsrv );
		}
		else srv->flags |= FLAG_REMOVE;
		srv = srv->next;
	}
	// Move all newsrv without FLAG_REMOVE to srv
	struct mgcamdserver_data *prev = NULL;
	srv = newcfg->mgcamd.server;
	while (srv) {
		struct mgcamdserver_data *next = srv->next;
		if (!(srv->flags&FLAG_REMOVE)) {
			if (prev) prev->next = srv->next; else newcfg->mgcamd.server = srv->next;
			cfg_addmgcamdserver(cfg, srv);
		} else prev = srv;
		srv = next;
	}
	// Move all srv with FLAG_REMOVE to newsrv
	prev = NULL;
	srv = cfg->mgcamd.server;
	while (srv) {
		struct mgcamdserver_data *next = srv->next;
		if (srv->flags&FLAG_REMOVE) {
			if (prev) prev->next = srv->next; else cfg->mgcamd.server = srv->next;
			cfg_addmgcamdserver(newcfg, srv);
		} else prev = srv;
		srv = next;
	}
}



//////////////////////////// CACHE

void remove_cache_peers(struct config_data *cfg)
{
	while (cfg->cache.peer) {
		struct cachepeer_data *peer = cfg->cache.peer;
		cfg->cache.peer = peer->next;
		if (peer->outsock>0) close(peer->outsock);
		free( peer );
	}
}

void update_cache_peers(struct config_data *cfg, struct config_data *newcfg )
{
	// set remove flag to old deleted clients & update reused one
	struct cachepeer_data *peer = cfg->cache.peer;
	while (peer) {
		struct cachepeer_data *newpeer = newcfg->cache.peer;
		while (newpeer) {
			if ( !(newpeer->flags&FLAG_REMOVE) )
			if ( !(peer->flags&FLAG_REMOVE) )
			if ( peer->host==newpeer->host )
			if ( peer->port==newpeer->port ) break;
			newpeer = newpeer->next;
		}
		if (newpeer) {
			newpeer->flags |= FLAG_REMOVE;
			// Update
			peer->fblock0onid = newpeer->fblock0onid;
			peer->fsendrep = newpeer->fsendrep;

		}
		else if (!peer->runtime) peer->flags |= FLAG_REMOVE; // if it is not created at runtime so delete
		peer = peer->next;
	}
	// Move all new without FLAG_REMOVE to old
	struct cachepeer_data *prev = NULL;
	struct cachepeer_data *newpeer = newcfg->cache.peer;
	while (newpeer) {
		struct cachepeer_data *next = newpeer->next;
		if (!(newpeer->flags&FLAG_REMOVE)) {
			if (prev) prev->next = newpeer->next; else newcfg->cache.peer = newpeer->next;
			cfg_addcachepeer(cfg, newpeer);
		} else prev = newpeer;
		newpeer = next;
	}
	// Move all old with FLAG_REMOVE to new
	prev = NULL;
	peer = cfg->cache.peer;
	while (peer) {
		struct cachepeer_data *next = peer->next;
		if (peer->flags&FLAG_REMOVE) {
			if (prev) prev->next = peer->next; else cfg->cache.peer = peer->next;
			cfg_addcachepeer(newcfg, peer);
		} else prev = peer;
		peer = next;
	}
}

void update_cache_server(struct config_data *cfg, struct config_data *newcfg)
{
	cfg->cache.faccept0onid = newcfg->cache.faccept0onid;
	if (cfg->cache.port!=newcfg->cache.port) {
		//cfg->cache.flags |= FLAG_DISCONNECT;
		cfg->cache.port = newcfg->cache.port;
		newcfg->cache.handle = cfg->cache.handle;
		cfg->cache.handle = -1;
	}
	update_cache_peers( cfg, newcfg );
}



//////////////////////////// CARDSERVERS

void update_cardserver_newcamd_clients(struct cardserver_data *cs, struct cardserver_data *newcs )
{
	// set remove flag to old deleted clients & update reused one
	struct cs_client_data *cli = cs->newcamd.client;
	while (cli) {
		struct cs_client_data *newcli = newcs->newcamd.client;
		while (newcli) {
			if ( !(newcli->flags&FLAG_REMOVE) )
			if ( !(cli->flags&FLAG_REMOVE) )
			if ( !strcmp(cli->user, newcli->user) ) break;
			newcli = newcli->next;
		}
		if (newcli) {
			newcli->flags |= FLAG_REMOVE;
			// Update Newcamd Client Data
			// PASS
			if ( strcmp(cli->pass, newcli->pass) ) {
				cli->flags |= FLAG_DISCONNECT;
				strcpy(cli->pass, newcli->pass);
			}
			// Options
			cli->dcwtime = newcli->dcwtime;
			cli->dcwtimeout = newcli->dcwtimeout;
#ifdef DCWPERIOD
			cli->dcwperiod = newcli->dcwperiod;
#endif
			cli->dcwtime = newcli->dcwtime;
			// info+data
			struct client_info_data *info = cli->info;
			cli->info = newcli->info;
			newcli->info = info;
			cli->realname = newcli->realname;
		}
		else cli->flags |= FLAG_REMOVE;
		cli = cli->next;
	}
	// Move all newcli without FLAG_REMOVE to cli
	struct cs_client_data *prev = NULL;
	struct cs_client_data *newcli = newcs->newcamd.client;
	while (newcli) {
		struct cs_client_data *next = newcli->next;
		if (!(newcli->flags&FLAG_REMOVE)) {
			if (prev) prev->next = newcli->next; else newcs->newcamd.client = newcli->next;
			cs_addnewcamdclient(cs, newcli);
		} else prev = newcli;
		newcli = next;
	}
	// Move all cli with FLAG_REMOVE to newcli
	prev = NULL;
	cli = cs->newcamd.client;
	while (cli) {
		struct cs_client_data *next = cli->next;
		if (cli->flags&FLAG_REMOVE) {
			if (prev) prev->next = cli->next; else cs->newcamd.client = cli->next;
			cs_addnewcamdclient(newcs, cli);
		} else prev = cli;
		cli = next;
	}
}

void update_cardserver(struct config_data *cfg, struct config_data *newcfg)
{
	struct cardserver_data *cs = cfg->cardserver;
	while (cs) {
		struct cardserver_data *newcs = newcfg->cardserver;
		while (newcs) {
			if (cs->newcamd.port==newcs->newcamd.port) break;
			newcs = newcs->next;
		}
		if (newcs) {
			newcs->flags |= FLAG_REMOVE;
			// Name
			strcpy( cs->name, newcs->name);
			// Key
			if (memcmp( cs->newcamd.key, newcs->newcamd.key, sizeof(cs->newcamd.key)) ) {
				cs->newcamd.flags |= FLAG_DISCONNECT;
				memcpy( cs->newcamd.key, newcs->newcamd.key, sizeof(cs->newcamd.key) );
			}
			// card
			if ( memcmp( &cs->card, &newcs->card, sizeof(struct cs_card_data)) ) {
				cs->flags |= FLAG_DISCONNECT;
				memcpy( &cs->card, &newcs->card, sizeof(struct cs_card_data));
			}
			// options
			memcpy( &cs->option, &newcs->option, sizeof(cs->option) );
			update_cardserver_newcamd_clients( cs, newcs );
			// SIDS
			cs->isdeniedsids = newcs->isdeniedsids;
			void *tmp = cs->sids;
			cs->sids = newcs->sids;
			newcs->sids = tmp;
		}
		else cs->flags |= FLAG_REMOVE;
		cs = cs->next;
	}
	// Move all new without FLAG_REMOVE to current
	struct cardserver_data *prev = NULL;
	cs = newcfg->cardserver;
	while (cs) {
		struct cardserver_data *next = cs->next;
		if (!(cs->flags&FLAG_REMOVE)) {
			if (prev) prev->next = cs->next; else newcfg->cardserver = cs->next;
			cfg_addprofile(cfg, cs);
		} else prev = cs;
		cs = next;
	}
	// Move all current with FLAG_REMOVE to new
	prev = NULL;
	cs = cfg->cardserver;
	while (cs) {
		struct cardserver_data *next = cs->next;
		if (cs->flags&FLAG_REMOVE) {
			if (prev) prev->next = cs->next; else cfg->cardserver = cs->next;
			cfg_addprofile(newcfg, cs);
		} else prev = cs;
		cs = next;
	}
}

void server2string(struct cs_server_data *srv, char *line)
{
	int i;
	char tmp[255];
	if (!srv) return;
	if (srv->type==TYPE_CCCAM)
		sprintf(line, "C: %s %d %s %s", srv->host->name, srv->port, srv->user, srv->pass);
	else if (srv->type==TYPE_NEWCAMD) {
		sprintf(line, "N: %s %d %s %s ", srv->host->name, srv->port, srv->user, srv->pass);
		array2hex( srv->key, line+strlen(line), 14);
	}

	strcat(line," { ");

	// Option: Profiles
	if (srv->priority) {
		sprintf(tmp, "priority=%d; ", srv->priority);
		strcat(line,tmp);
	}

	// Option: Profiles
	if (srv->csport[0]) {
		sprintf(tmp, "profiles= %d", srv->csport[0]); strcat(line,tmp);
		for(i=1; i<MAX_CSPORTS; i++) {
			if (!srv->csport[i]) break;
			sprintf(tmp,", %d",srv->csport[i]); strcat(line,tmp);
		}
		strcat(line,"; ");
	}

	// Option: share
	if (srv->sharelimits[0].caid!=0xFFFF) {
		sprintf(tmp, "shares= %x:%x:%x", srv->sharelimits[0].caid,srv->sharelimits[0].provid,srv->sharelimits[0].uphops);
		strcat(line,tmp);
		for(i=1; i<100; i++) {
			if (srv->sharelimits[i].caid==0xFFFF) break;
			sprintf(tmp, ", %x:%x:%x", srv->sharelimits[i].caid,srv->sharelimits[i].provid,srv->sharelimits[i].uphops);
			strcat(line,tmp);
		}
		strcat(line,"; ");
	}

	// Option: sids
	if (srv->sids) {
		struct sid_chid_data *sid = srv->sids;
		if (sid->chid) sprintf(tmp, "sids= %04x:%04x", sid->sid, sid->chid); else sprintf(tmp, "sids= %04x", sid->sid);
		strcat(line,tmp);
		sid++;
		while (sid->sid) {
			if (sid->chid) sprintf(tmp, ", %04x:%04x", sid->sid, sid->chid); else sprintf(tmp, ", %04x", sid->sid);
			strcat(line,tmp);
			sid++;
		}
		strcat(line,"; ");
	}

	strcat(line," }");
}


//////////////////////////// SERVERS

void free_card(struct cs_card_data* card)
{
	int s;
	for(s=0; s<256; s++) {
		struct sid_data *sid1 = card->sids[s];
		while (sid1) {
			struct sid_data *sid = sid1;
			sid1 = sid1->next;
			free(sid);
		}
	}
	free(card);
}

void free_cardlist(struct cs_card_data* card)
{
	while (card) {
		struct cs_card_data *tmp = card;
		card = card->next;
		free_card( tmp );
	}
}



void remove_servers(struct config_data *cfg)
{
	while (cfg->server) {
		struct cs_server_data *srv = cfg->server;
		cfg->server = srv->next;
		pthread_mutex_destroy( &srv->lock );
		if (srv->handle>0) {
			if (srv->type==TYPE_NEWCAMD) cs_disconnect_srv(srv);
			else if (srv->type==TYPE_CCCAM) cc_disconnect_srv(srv);
			else close(srv->handle);
		}
		// Sids
		if (srv->sids) free(srv->sids);
		// Cards
		free_cardlist(srv->card);
		//
		free( srv );
	}
}


void update_servers(struct config_data *cfg, struct config_data *newcfg )
{
	// set remove flag to old deleted clients & update reused one
	struct cs_server_data *srv = cfg->server;
	while (srv) {
		struct cs_server_data *newsrv = newcfg->server;
		while (newsrv) {
			if ( !(newsrv->flags&FLAG_REMOVE) )
			if ( !(srv->flags&FLAG_REMOVE) )
			if ( srv->host==newsrv->host )
			if ( srv->port==newsrv->port )
			if ( !strcmp(srv->user, newsrv->user) ) break;
			newsrv = newsrv->next;
		}
		if (newsrv) {
			newsrv->flags |= FLAG_REMOVE;
			// PASS
			if ( strcmp(srv->pass, newsrv->pass) ) {
				srv->flags |= FLAG_DISCONNECT;
				strcpy(srv->pass, newsrv->pass);
			}
			// TYPE
			if (srv->type!=newsrv->type) { // ???
				srv->flags |= FLAG_DISCONNECT;
				srv->type = newsrv->type;
			}
			// NEWCAMD KEY
			if ( (srv->type==TYPE_NEWCAMD) && memcmp(srv->key,newsrv->key, sizeof(srv->key)) ) {
				srv->flags |= FLAG_DISCONNECT;
				memcpy(srv->key,newsrv->key, sizeof(srv->key));
			}
			// csport
			if ( memcmp(srv->csport,newsrv->csport,sizeof(srv->csport)) ) {
				srv->flags |= FLAG_DISCONNECT;
				memcpy( srv->csport, newsrv->csport, sizeof(srv->csport) );
			}
			// priority
			srv->priority = newsrv->priority;
			// Share Limits
			if ( memcmp(srv->sharelimits, newsrv->sharelimits, sizeof(srv->sharelimits)) ) {
				srv->flags |= FLAG_DISCONNECT;
				memcpy(srv->sharelimits, newsrv->sharelimits, sizeof(srv->sharelimits));
			}
			// ACCEPTED SIDs
			void *tmp = srv->sids;
			srv->sids = newsrv->sids;
			newsrv->sids = tmp;
		}
		else srv->flags |= FLAG_REMOVE;
		srv = srv->next;
	}
	// Move all newsrv without FLAG_REMOVE to srv
	struct cs_server_data *prev = NULL;
	struct cs_server_data *newsrv = newcfg->server;
	while (newsrv) {
		struct cs_server_data *next = newsrv->next;
		if (!(newsrv->flags&FLAG_REMOVE)) {
			if (prev) prev->next = newsrv->next; else newcfg->server = newsrv->next;
			cfg_addserver(cfg, newsrv);
		} else prev = newsrv;
		newsrv = next;
	}
	// Move all srv with FLAG_REMOVE to newsrv
	prev = NULL;
	srv = cfg->server;
	while (srv) {
		struct cs_server_data *next = srv->next;
		if (srv->flags&FLAG_REMOVE) {
			if (prev) prev->next = srv->next; else cfg->server = srv->next;
			cfg_addserver(newcfg, srv);
		} else prev = srv;
		srv = next;
	}
}

///////////////////////////////////////////////////////////////////////////////
void reread_config( struct config_data *cfg )
{
	struct config_data newcfg;

	// Remove files data from current config
	free_chinfo(cfg);
	free_ip2country(cfg);
	free_providers(cfg);

	init_config(&newcfg);
	newcfg.host = cfg->host;
	//usleep( 100000 );
	read_config( &newcfg );

	if (!cfg) return;

	cfg->host = newcfg.host;
	cfg->files = newcfg.files;

	// Files+Data
	strcpy(cfg->stylesheet_file,newcfg.stylesheet_file);
	strcpy(cfg->channelinfo_file,newcfg.channelinfo_file);
	strcpy(cfg->providers_file,newcfg.providers_file);
	strcpy(cfg->ip2country_file,newcfg.ip2country_file);
	cfg->ip2country = newcfg.ip2country;
	cfg->chninfo = newcfg.chninfo;
	cfg->providers = newcfg.providers;

	// HTTP SERVER
	strcpy(cfg->http.user, newcfg.http.user);
	strcpy(cfg->http.pass, newcfg.http.pass);
	cfg->http.noeditor = newcfg.http.noeditor;
	cfg->http.norestart = newcfg.http.norestart;
	cfg->http.autorefresh = newcfg.http.autorefresh;
	if (cfg->http.port!=newcfg.http.port) {
		//cfg->http.flags |= FLAG_DISCONNECT;
		cfg->http.port = newcfg.http.port;
		newcfg.http.handle = cfg->http.handle;
		cfg->http.handle = -1;
	}

	// DAB-DCW: remove old one and get new one
	void *tmp = cfg->bad_dcw;
	cfg->bad_dcw = newcfg.bad_dcw;
	newcfg.bad_dcw = tmp;

	// Servers

	update_cccam_servers( cfg, &newcfg );

	update_mgcamd_servers( cfg, &newcfg );

	update_cache_server( cfg, &newcfg );

	update_cardserver( cfg, &newcfg );
	cfg->newcamdclientid = newcfg.newcamdclientid;

	update_servers( cfg, &newcfg );

	///////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// FREE NEWCFG
	///////////////////////////////////////////////////////////////////////////
	sleep( 1 );

	// http Server
	if (newcfg.http.handle>0) close(newcfg.http.handle);

	// Server
	remove_servers(&newcfg);

	// CCcam Servers
	while (newcfg.cccam.server) {
		struct cccamserver_data *newsrv = newcfg.cccam.server;
		newcfg.cccam.server = newsrv->next;
		remove_cccam_clients( newsrv );
		if (newsrv->handle>0) close(newsrv->handle);
		free( newsrv );
	}

	// Mgcamd Servers
	while (newcfg.mgcamd.server) {
		struct mgcamdserver_data *newsrv = newcfg.mgcamd.server;
		newcfg.mgcamd.server = newsrv->next;
		remove_mgcamd_clients( newsrv );
		if (newsrv->handle>0) close(newsrv->handle);
		free( newsrv );
	}

	// Cache Server
	remove_cache_peers( &newcfg );
	if (newcfg.cache.handle>0) close(newcfg.cache.handle);

	// Cardservers
	while (newcfg.cardserver) {
		struct cardserver_data *newcs = newcfg.cardserver;
		newcfg.cardserver = newcs->next;
		remove_newcamd_clients( newcs );
		if (newcs->newcamd.handle>0) close(newcs->newcamd.handle);
		free( newcs );
	}

	// DAB-DCW
	while(newcfg.bad_dcw) {
		struct dcw_data *dcw = newcfg.bad_dcw;
		newcfg.bad_dcw = dcw->next;
		free( dcw );
	}

	///////////////////////////////////////////////////////////////////////////
	// DISCONNECT CFG
	///////////////////////////////////////////////////////////////////////////

	// Cardservers
	struct cardserver_data *cs = cfg->cardserver;
	while (cs) {
		struct cs_client_data *cli = cs->newcamd.client;
		while (cli) {
			if ( (cs->flags&FLAG_DISCONNECT)||(cs->newcamd.flags&FLAG_DISCONNECT)||(cli->flags&FLAG_DISCONNECT) ) cs_disconnect_cli( cli );
			cli->flags &= ~FLAG_DISCONNECT;
			cli = cli->next;
		}
		cs->newcamd.flags &= ~FLAG_DISCONNECT;
		cs->flags &= ~FLAG_DISCONNECT;
		cs = cs->next;
	}

	// CCcam Server
	struct cccamserver_data *cccam = cfg->cccam.server;
	while (cccam) {
		struct cc_client_data *cli = cccam->client;
		while (cli) {
			if ( (cccam->flags&FLAG_DISCONNECT)||(cli->flags&FLAG_DISCONNECT) ) cc_disconnect_cli( cli );
			cli->flags &= ~FLAG_DISCONNECT;
			cli = cli->next;
		}
		cccam->flags &= ~FLAG_DISCONNECT;
		cccam = cccam->next;
	}

	// Mgcamd Server
	struct mgcamdserver_data *mgcamd = cfg->mgcamd.server;
	while (mgcamd) {
		struct mg_client_data *cli = mgcamd->client;
		while (cli) {
			if ( (mgcamd->flags&FLAG_DISCONNECT)||(cli->flags&FLAG_DISCONNECT) ) mg_disconnect_cli( cli );
			cli->flags &= ~FLAG_DISCONNECT;
			cli = cli->next;
		}
		mgcamd->flags &= ~FLAG_DISCONNECT;
		mgcamd = mgcamd->next;
	}

	// Servers
	struct cs_server_data *srv = cfg->server;
	while (srv) {
		if (srv->flags&FLAG_DISCONNECT) {
			if (srv->type==TYPE_CCCAM) cc_disconnect_srv( srv );
			else if (srv->type==TYPE_NEWCAMD) cs_disconnect_srv( srv );
			else if (srv->type==TYPE_RADEGAST) rdgd_disconnect_srv( srv );
		}
		srv->flags &= ~FLAG_DISCONNECT;
		srv = srv->next;
	}

}




// Open ports
int check_config(struct config_data *cfg)
{
	// Open ports for new profiles
	struct cardserver_data *cs = cfg->cardserver;
	while (cs) {
		if (cs->newcamd.handle<=0) {
			if ( (cs->newcamd.port<1024)||(cs->newcamd.port>0xffff) ) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," [%s] Newcamd Server: invalid port value (%d)\n", cs->name, cs->newcamd.port);
				cs->newcamd.handle = INVALID_SOCKET;
			}
			else if ( (cs->newcamd.handle=CreateServerSockTcp_nonb(cs->newcamd.port)) == -1) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," [%s] Newcamd Server: bind port failed (%d)\n", cs->name, cs->newcamd.port);
				cs->newcamd.handle = INVALID_SOCKET;
			}
			else debugf(getdbgflag(DBG_CONFIG,0,0)," [%s] Newcamd Server started on port %d\n",cs->name,cs->newcamd.port);
		}
#ifdef RADEGAST_SRV
		if (cs->rdgd.handle<=0) {
			if ( (cs->rdgd.port<1024)||(cs->rdgd.port>0xffff) ) {
				//debugf(getdbgflag(DBG_CONFIG,0,0)," CardServer '%s': invalid port value (%d)\n", cs->name, cs->newcamd.port);
				cs->rdgd.handle = INVALID_SOCKET;
			}
			else if ( (cs->rdgd.handle=CreateServerSockTcp_nonb(cs->rdgd.port)) == -1) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," [%s] Radegast Server: bind port failed (%d)\n", cs->name, cs->rdgd.port);
				cs->rdgd.handle = INVALID_SOCKET;
			}
			else debugf(getdbgflag(DBG_CONFIG,0,0)," [%s] Radegast Server started on port %d\n",cs->name,cs->rdgd.port);
		}
#endif
		cs = cs->next;
	}

	// HTTP Port
#ifdef HTTP_SRV
	if (cfg->http.handle<=0) {
		if ( (cfg->http.port<1024)||(cfg->http.port>0xffff) ) {
			debugf(getdbgflag(DBG_CONFIG,0,0)," HTTP Server: invalid port value (%d)\n", cfg->http.port);
			cfg->http.handle = INVALID_SOCKET;
		}
		else if ( (cfg->http.handle=CreateServerSockTcp(cfg->http.port)) == -1) {
			debugf(getdbgflag(DBG_CONFIG,0,0)," HTTP Server: bind port failed (%d)\n", cfg->http.port);
			cfg->http.handle = INVALID_SOCKET;
		}
		else debugf(getdbgflag(DBG_CONFIG,0,0)," HTTP server started on port %d\n",cfg->http.port);
	}
#endif

#ifdef CCCAM_SRV
	// Open port for CCcam server
	struct cccamserver_data *cccam = cfg->cccam.server;
	while (cccam) {
		if (cccam->handle<=0) {
			if ( (cccam->port<1024)||(cccam->port>0xffff) ) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," CCcam Server: invalid port value (%d)\n", cccam->port);
				cccam->handle = INVALID_SOCKET;
			}
			else if ( (cccam->handle=CreateServerSockTcp_nonb(cccam->port)) == -1) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," CCcam Server: bind port failed (%d)\n", cccam->port);
				cccam->handle = INVALID_SOCKET;
			}
			else debugf(getdbgflag(DBG_CONFIG,0,0)," CCcam server %d started on port %d (version: %s)\n",cccam->id, cccam->port,cfg->cccam.version);
		}
		cccam = cccam->next;
	}
#endif

#ifdef FREECCCAM_SRV
	// Open port
	if (cfg->freecccam.server.handle<=0) {
		if ( (cfg->freecccam.server.port<1024)||(cfg->freecccam.server.port>0xffff) ) {
			debugf(getdbgflag(DBG_CONFIG,0,0)," FreeCCcam Server: invalid port value (%d)\n", cfg->freecccam.server.port);
			cfg->freecccam.server.handle = INVALID_SOCKET;
		}
		else if ( (cfg->freecccam.server.handle=CreateServerSockTcp_nonb(cfg->freecccam.server.port)) == -1) {
			debugf(getdbgflag(DBG_CONFIG,0,0)," FreeCCcam Server: bind port failed (%d)\n", cfg->freecccam.server.port);
			cfg->freecccam.server.handle = INVALID_SOCKET;
		}
		else debugf(getdbgflag(DBG_CONFIG,0,0)," FreeCCcam server started on port %d\n",cfg->freecccam.server.port);
	}
#endif

#ifdef MGCAMD_SRV
	// Open port for MGcamd servers
	struct mgcamdserver_data *mgcamd = cfg->mgcamd.server;
	while (mgcamd) {
		if (mgcamd->handle<=0) {
			if ( (mgcamd->port<1024)||(mgcamd->port>0xffff) ) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," MGcamd Server: invalid port value (%d)\n", mgcamd->port);
				mgcamd->handle = INVALID_SOCKET;
			}
			else if ( (mgcamd->handle=CreateServerSockTcp_nonb(mgcamd->port)) == -1) {
				debugf(getdbgflag(DBG_CONFIG,0,0)," MGcamd Server: bind port failed (%d)\n", mgcamd->port);
				mgcamd->handle = INVALID_SOCKET;
			}
			else debugf(getdbgflag(DBG_CONFIG,0,0)," MGcamd server %d started on port %d\n", mgcamd->id, mgcamd->port);
		}
		mgcamd = mgcamd->next;
	}
#endif

	// Open port for Cache
	if (cfg->cache.handle<=0) {
		if ( (cfg->cache.port<1024)||(cfg->cache.port>0xffff) ) {
			debugf(getdbgflag(DBG_CONFIG,0,0)," Cache Server: invalid port value (%d)\n", cfg->cache.port);
			cfg->cache.handle = INVALID_SOCKET;
		}
		else if ( (cfg->cache.handle=CreateServerSockUdp(cfg->cache.port)) == -1) {
			debugf(getdbgflag(DBG_CONFIG,0,0)," Cache Server: bind port failed (%d)\n", cfg->cache.port);
			cfg->cache.handle = INVALID_SOCKET;
		}
		else {
			debugf(getdbgflag(DBG_CONFIG,0,0)," Cache server started on port %d\n",cfg->cache.port);
		}
	}
	// create cahcepeer socket
	struct cachepeer_data *peer = cfg->cache.peer;
	while (peer) {
		if (peer->outsock<=0) peer->outsock = CreateClientSockUdp();
		peer = peer->next;
	}

	return 0;
}


void cfg_set_id_counters(struct config_data *cfg)
{

	// Servers
	cfg->totalservers = 0;
	struct cs_server_data *srv = cfg->server;
	while (srv) {
		if (!srv->id) {
			srv->id = cfg->serverid;
			cfg->serverid++;
		}
		cfg->totalservers++;
		srv = srv->next;
	}

	// Profiles
	cfg->totalprofiles = 0;
	struct cardserver_data *cs = cfg->cardserver;
	while (cs) {
		if (!cs->id) {
			cs->id = cfg->cardserverid;
			cfg->cardserverid++;
		}
		cs->newcamd.totalclients = 0;
		struct cs_client_data *cli = cs->newcamd.client;
		while (cli) {
			cli->pid = cs->id;
			if (!cli->id) {
				cli->id = cfg->clientid;
				cfg->clientid++;
			}
			cs->newcamd.totalclients++;
			cli = cli->next;
		}

		cfg->totalprofiles++;
		cs = cs->next;
	}

	// CCcam Servers/Clients
#ifdef CCCAM_SRV
	cfg->cccam.totalservers = 0;
	struct cccamserver_data *ccc = cfg->cccam.server;
	while (ccc) {
		ccc->totalclients = 0;
		struct cc_client_data *cccli = ccc->client;
		while (cccli) {
			cccli->srvid = ccc->id;
			if (!cccli->id) {
				cccli->id = cfg->cccam.clientid;
				cfg->cccam.clientid++;
			}
			ccc->totalclients++;
			cccli = cccli->next;
		}
		cfg->cccam.totalservers++;
		ccc = ccc->next;
	}
#endif

	// MGCAMD Clients
#ifdef MGCAMD_SRV
	cfg->mgcamd.totalservers = 0;
	struct mgcamdserver_data *mgcamd = cfg->mgcamd.server;
	while (mgcamd) {
		mgcamd->totalclients = 0;
		struct mg_client_data *cli = mgcamd->client;
		while (cli) {
			cli->srvid = mgcamd->id;
			if (!cli->id) {
				cli->id = cfg->mgcamd.clientid;
				cfg->mgcamd.clientid++;
			}
			mgcamd->totalclients++;
			cli = cli->next;
		}
		cfg->mgcamd.totalservers++;
		mgcamd = mgcamd->next;
	}
#endif

	// cahcepeers
	cfg->cache.totalpeers = 0;
	struct cachepeer_data *peer = cfg->cache.peer;
	while(peer) {
		if (!peer->id) {
			peer->id = cfg->cachepeerid;
			cfg->cachepeerid++;
		}
		cfg->cache.totalpeers++;
		peer = peer->next;
	}
}



// Close ports
int done_config(struct config_data *cfg)
{
	// Close Newcamd/Radegast Clients Connections & profiles ports
	struct cardserver_data *cs = cfg->cardserver;
	while (cs) {
		if (cs->newcamd.handle>0) {
			close(cs->newcamd.handle);
			struct cs_client_data *cscli = cs->newcamd.client;
			while (cscli) {
				if (cscli->handle>0) close(cscli->handle);
				cscli = cscli->next;
			}
		}
#ifdef RADEGAST_SRV
		if (cs->rdgd.handle>0) {
			close(cs->rdgd.handle);
			struct rdgd_client_data *rdgdcli = cs->rdgd.client;
			while (rdgdcli) {
				if (rdgdcli->handle>0) close(rdgdcli->handle);
				rdgdcli = rdgdcli->next;
			}
		}
#endif
		cs = cs->next;
	}

#ifdef HTTP_SRV
	// HTTP Port
	if (cfg->http.handle>0) close(cfg->http.handle);
#endif


#ifdef CCCAM_SRV
	struct cccamserver_data *cccam = cfg->cccam.server;
	while (cccam) {
		close(cccam->handle);
		struct cc_client_data *cccli = cccam->client;
		while (cccli) {
			if (cccli->handle>0) close(cccli->handle);
			cccli = cccli->next;
		}
		cccam = cccam->next;
	}
#endif


#ifdef FREECCCAM_SRV
	if (cfg->freecccam.server.handle>0) {
		close(cfg->freecccam.server.handle);
		struct cc_client_data *fcccli = cfg->freecccam.server.client;
		while (fcccli) {
			if (fcccli->handle>0) close(fcccli->handle);
			fcccli = fcccli->next;
		}
	}
#endif


#ifdef MGCAMD_SRV
	struct mgcamdserver_data *mgcamd = cfg->mgcamd.server;
	while (mgcamd) {
		close(mgcamd->handle);
		struct mg_client_data *cli = mgcamd->client;
		while (cli) {
			if (cli->handle>0) close(cli->handle);
			cli = cli->next;
		}
		mgcamd = mgcamd->next;
	}
#endif

	if (cfg->cache.handle>0) close(cfg->cache.handle);

	struct cachepeer_data *peer = cfg->cache.peer;
	while (peer) {
		if (peer->outsock>0) close(peer->outsock);
		peer = peer->next;
	}

	// Close Servers Connections
	struct cs_server_data *srv = cfg->server;
	while (srv) {
		if (srv->handle>0) close(srv->handle);
		srv = srv->next;
	}

	return 0;
}


// check if any profile updated for caid:provider
