
/////
// File: srv-newcamd.c
// Modification date: 2011.11.11
/////

///////////////////////////////////////////////////////////////////////////////
// PROTO
///////////////////////////////////////////////////////////////////////////////

void cs_disconnect_cli(struct cs_client_data *cli);
void *cs_connect_cli_thread(void *param);
void cs_cli_recvmsg(struct cs_client_data *cli,	struct cardserver_data *cs);
uint cs_check_sendcw();


struct cs_client_data *getnewcamdclientbyid(uint32 id)
{
	struct cardserver_data *cs = cfg.cardserver;
	while (cs) {
		if (!(cs->flags&FLAG_REMOVE)) {
			struct cs_client_data *cli = cs->newcamd.client;
			while (cli) {
				if (!(cli->flags&FLAG_REMOVE))
					if (cli->id==id) return cli;
				cli = cli->next;
			}
		}
		cs = cs->next;
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////


#ifdef CHECK_HACKER

struct ip_hacker_data *cs_getip(struct cardserver_data *cs, uint32 ip )
{
	struct ip_hacker_data *iplist = cs->iplist;
	while (iplist) {
		if (iplist->ip==ip) return iplist;
		iplist = iplist->next;
	}
	iplist = malloc( sizeof(struct ip_hacker_data) );
	memset(iplist, 0, sizeof(struct ip_hacker_data) );
	iplist->ip = ip;
	iplist->next = cs->iplist;
	cs->iplist = iplist;
	return iplist;
}

#endif


void cs_disconnect_cli(struct cs_client_data *cli)
{
	if (cli->handle!=INVALID_SOCKET) {
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," newcamd: client '%s' disconnected\n", cli->user);
		close(cli->handle);
		cli->handle = INVALID_SOCKET;
		uint ticks = GetTickCount();
		cli->uptime += ticks-cli->connected;
		cli->connected = ticks; // Last Seen
	}
}


struct cs_clicon {
	struct cardserver_data *cs;
	int sock;
	uint32 ip;
};


void *th_cs_connect_cli(struct cs_clicon *param)
{
    char passwdcrypt[120];
	unsigned char keymod[14];
	int i,index;
	unsigned char sessionkey[16];
	struct cs_custom_data clicd;
	unsigned char buf[CWS_NETMSGSIZE];

	struct cardserver_data *cs = param->cs;
	int sock = param->sock;
	uint32 ip = param->ip;
	free(param);

#ifdef CHECK_HACKER
	struct ip_hacker_data *iplist = cs_getip( cs, ip );
	iplist->nbconn++;
	if (iplist->count>0) {
		if ( (GetTickCount()-iplist->lastseen)<(1000*iplist->count) ) {
			close(sock);
			return NULL;
		}
		if (iplist->count>10)
		if ( (GetTickCount()-iplist->lastseen)<(5000*iplist->count) ) {
			close(sock);
			return NULL;
		}
		if (iplist->count>20)
		if ( (GetTickCount()-iplist->lastseen)<(20000*iplist->count) ) {
			close(sock);
			return NULL;
		}
		if (iplist->count>30)
		if ( (GetTickCount()-iplist->lastseen)<(60000*iplist->count) ) {
			close(sock);
			return NULL;
		}
	}
	iplist->lastseen = GetTickCount(); // Last Accepted time
	iplist->count++;
#endif

	// Create random deskey
	for (i=0; i<14; i++) keymod[i] = 0xff & rand();
	// Create Multics ID
	keymod[3] = (keymod[0]^'M') + keymod[1] + keymod[2];
	keymod[7] = keymod[4] + (keymod[5]^'C') + keymod[6];
	keymod[11] = keymod[8] + keymod[9] + (keymod[10]^'S');

//STAT: SEND RND KEY
	// send random des key

	if (send_nonb(sock, keymod, 14, 500)<=0) {
		debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," newcamd: error sending init sequence\n");
		close(sock);
		pthread_exit(NULL);
	}

	uint32 ticks = GetTickCount();
	// Calc SessionKey
	//debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," DES Key: "); debughex(keymod,14);
	des_login_key_get(keymod, cs->newcamd.key, 14, sessionkey);
	//debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," Login Key: "); debughex(sessionkey,16);

//STAT: LOGIN INFO
	// 3. login info
	i = cs_message_receive(sock, &clicd, buf, sessionkey,3000);
	if (i<=0) {
		if (i==-2) debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," %s: (%s) new connection closed, wrong des key\n", cs->name, ip2string(ip));
		else debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," %s: (%s) new connection closed, receive timeout\n", cs->name, ip2string(ip));
		close(sock);
		pthread_exit(NULL);
	}

	ticks = GetTickCount()-ticks;
	if (buf[0]!=MSG_CLIENT_2_SERVER_LOGIN) {
		close(sock);
		pthread_exit(NULL);
	}

	// Check username length
	if ( strlen( (char*)buf+3 )>63 ) {
		/*
		buf[0] = MSG_CLIENT_2_SERVER_LOGIN_NAK;
		buf[1] = 0;
		buf[2] = 0;
		cs_message_send(sock, NULL, buf, 3, sessionkey);
		*/
		debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," %s: (%s) new connection closed, wrong username length\n", cs->name, ip2string(ip));
		close(sock);
		return(NULL);
	}

#ifdef CHECK_HACKER
	strcpy(iplist->user, (char*)buf+3);
#endif

	pthread_mutex_lock(&prg.lockcli);
	index = 3;
	struct cs_client_data *cli = cs->newcamd.client;
	int found = 0;
	while (cli) {
		if ( !IS_DISABLED(cli->flags) )
		if (!strcmp(cli->user,(char*)&buf[index])) {
			if (IS_DISABLED(cli->flags)) { // Connect only enabled clients
				pthread_mutex_unlock(&prg.lockcli);
				debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," %s: connection refused for client '%s' (%s), client disabled\n", cs->name, cli->user, ip2string(ip));
				close(sock);
				return(NULL);
			}
			found=1;
			break;
		}
		cli = cli->next;
	}
	if (!found) {
		pthread_mutex_unlock(&prg.lockcli);
		buf[0] = MSG_CLIENT_2_SERVER_LOGIN_NAK;
		buf[1] = 0;
		buf[2] = 0;
		//cs_message_send(sock, NULL, buf, 3, sessionkey);
		debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," %s: unknown user '%s' (%s)\n", cs->name, &buf[3], ip2string(ip));
		close(sock);
		return(NULL);
	}

	// Check password
	index += strlen(cli->user) +1;
	__md5_crypt(cli->pass, "$1$abcdefgh$",passwdcrypt);
	if (!strcmp(passwdcrypt,(char*)&buf[index])) {
		if (cli->ip==ip) cli->nbdiffip++;
		//Check Reconnection
		if (cli->handle>0) {
			if (cli->ip==ip) {
				debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," %s: Client '%s' (%s) already connected\n", cs->name,cli->user, ip2string(ip));
				cs_disconnect_cli(cli);
			}
			else {
				if ( (GetTickCount()-cli->lastecmtime) < 60000 ) {
					pthread_mutex_unlock(&prg.lockcli);
					debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," %s: Client '%s' (%s) already connected with different ip, Connection aborted for recent activity\n", cs->name, cli->user, ip2string(ip));
					cli->nbloginerror++;
					close(sock);
					return NULL;
				}
				debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," %s: Client '%s' (%s) already connected with different ip \n", cs->name, cli->user, ip2string(ip));
				cs_disconnect_cli(cli);
			}
		}

		cli->nblogin++;

		//Check Reconnection
		if (cli->handle>0) {
			debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," %s: user already connected '%s' (%s)\n", cs->name, cli->user, ip2string(ip));
			cs_disconnect_cli(cli);
		}

		// Store program id
		cli->progid = clicd.sid;
		//
		buf[0] = MSG_CLIENT_2_SERVER_LOGIN_ACK;
		buf[1] = 0;
		buf[2] = 0;
		// Send Multics Id&Version
		if (clicd.provid==0x0057484F) { // WHO 
			clicd.provid = 0x004D4353; // MCS
			clicd.sid = REVISION; // Revision
			cs_message_send(sock, &clicd, buf, 3, sessionkey);
		} else cs_message_send(sock, NULL, buf, 3, sessionkey);
		des_login_key_get( cs->newcamd.key, (unsigned char*)passwdcrypt, strlen(passwdcrypt),sessionkey);
		memcpy( &cli->sessionkey, &sessionkey, 16);

#ifdef CHECK_HACKER
		iplist->count = 10; // for reconnect
#endif

		// Setup User data
		cli->handle = sock;
		cli->ip = ip;
		cli->ping = ticks;
		cli->ecm.busy=0;
		cli->ecm.recvtime=0;
		cli->connected = cli->lastactivity = cli->lastecmtime = GetTickCount();
		cli->chkrecvtime = 0;
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," %s: client '%s' connected (%s)\n", cs->name, cli->user, ip2string(ip));
		pthread_mutex_unlock(&prg.lockcli);
		pipe_wakeup( srvsocks[1] );
	}
	else {
		pthread_mutex_unlock(&prg.lockcli);
		// send NAK
		buf[0] = MSG_CLIENT_2_SERVER_LOGIN_NAK;
		buf[1] = 0;
		buf[2] = 0;
		//cs_message_send(sock, NULL, buf, 3, sessionkey);
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," %s: client '%s' wrong password (%s)\n", cs->name, cli->user, ip2string(ip));
		cli->nbloginerror++;
		close(sock);
	}
}



void *cs_connect_cli_thread(void *param)
{
	int clientsock;
	struct sockaddr_in clientaddr;
	socklen_t socklen = sizeof(struct sockaddr);
	// Connect Clients 
	pthread_t srv_tid;

	while (1) {
		pthread_mutex_lock(&prg.locksrvcs);

		struct pollfd pfd[MAX_CSPORTS];
		int pfdcount = 0;

		struct cardserver_data *cs = cfg.cardserver;
		while(cs) {
			if ( !IS_DISABLED(cs->newcamd.flags)&&(cs->newcamd.handle>0) ) {
				cs->newcamd.ipoll = pfdcount;
				pfd[pfdcount].fd = cs->newcamd.handle;
				pfd[pfdcount++].events = POLLIN | POLLPRI;
			} else cs->newcamd.ipoll = -1;
			cs = cs->next;
		}

		int retval = poll(pfd, pfdcount, 3000);

		if (retval>0) {
			struct cardserver_data *cs = cfg.cardserver;
			while(cs) {
				if ( !IS_DISABLED(cs->newcamd.flags)&&(cs->newcamd.handle>0) && (cs->newcamd.ipoll>=0) && (cs->newcamd.handle==pfd[cs->newcamd.ipoll].fd) ) {
					if ( pfd[cs->newcamd.ipoll].revents & (POLLIN|POLLPRI) ) {
						clientsock = accept(cs->newcamd.handle, (struct sockaddr*)&clientaddr, &socklen );
						if (clientsock<=0) {
							//if (errno == EAGAIN || errno == EINTR) continue;
							//else {
								debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," newcamd: Accept failed (errno=%d)\n",errno);
							//}
						}
						else {
							debugf(getdbgflag(DBG_NEWCAMD,cs->id,0)," [%s] New Connection (%s)\n", cs->name, ip2string(clientaddr.sin_addr.s_addr) );
							///SetSocketKeepalive(clientsock); 
							struct cs_clicon *clicondata = malloc( sizeof(struct cs_clicon) );
							clicondata->cs = cs; 
							clicondata->sock = clientsock; 
							clicondata->ip = clientaddr.sin_addr.s_addr;
							//while(EAGAIN==pthread_create(&srv_tid, NULL, th_cs_connect_cli,clicondata)) usleep(1000); pthread_detach(&srv_tid);
							create_prio_thread(&srv_tid, (threadfn)th_cs_connect_cli,clicondata, 50);
						}
					}
				}
				cs = cs->next;
			}
		}
		pthread_mutex_unlock(&prg.locksrvcs);
		usleep(3000);
	}
	END_PROCESS = 1;
}


void cs_store_ecmclient(struct cardserver_data *cs, int ecmid, struct cs_client_data *cli, int climsgid)
{
	//check for last ecm recv time
	uint32 ticks = GetTickCount();

	cli->ecm.dcwtime = cli->dcwtime;

#ifdef DCWPERIOD
	if (cs->dcwalgorithm==0) {
		cli->ecm.dcwtime = cli->dcwtime;
		uint32 diff = ticks - cli->lastdcwtime;
		if (diff<cli->dcwperiod)
			if ( (cli->dcwperiod-diff)>cli->dcwtime )
				cli->ecm.dcwtime = cli->dcwperiod-diff;
	}
	else if (cs->dcwalgorithm==1) {
		unsigned int newperiod = 0;
		if ( cli->dcwperiod && (cli->dcwperiod>cli->dcwtime) ) {
			unsigned int msperecm = ((ticks-cli->connected)+cli->uptime) / (cli->ecmnb+1);
			if ( msperecm>(2*cli->dcwperiod-cli->dcwtime) )  newperiod = cli->dcwtime;
					else newperiod = (2*cli->dcwperiod)-msperecm;
		}
		else newperiod = cli->dcwtime;

		cli->ecm.dcwtime = cli->dcwtime;
		uint32 diff = ticks - cli->lastdcwtime;
		if (diff<newperiod)
			if ( (newperiod-diff)>cli->dcwtime ) cli->ecm.dcwtime = newperiod-diff;
	}
	if (cli->ecm.dcwtime>cs->option.dcw.timeout) cli->ecm.dcwtime = cs->option.dcw.timeout;
#endif

	cli->ecm.recvtime = ticks;
	cli->ecm.id = ecmid;
    cli->ecm.climsgid = climsgid;
    cli->ecm.status = STAT_ECM_SENT;
	ecm_addip(ecmid, cli->ip);
}


void cs_cli_recvmsg(struct cs_client_data *cli,	struct cardserver_data *cs)
{
	struct cs_custom_data clicd; // Custom data
	int len;
	unsigned char buf[CWS_NETMSGSIZE];
	unsigned char data[CWS_NETMSGSIZE]; // for other use

	if (cli->handle>0) {
		len = cs_msg_chkrecv(cli->handle);
		if (len==0) {
			debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," newcamd: client '%s' read failed %d\n", cli->user,len);
			cs_disconnect_cli(cli);
		}
		else if (len==-1) {
			if (!cli->chkrecvtime) cli->chkrecvtime = GetTickCount();
			else if ( (cli->chkrecvtime+300)<GetTickCount() ) {
				debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," newcamd: client '%s' read timeout %d\n", cli->user,len);
				cs_disconnect_cli(cli);
			}
		}
		else if (len>0) {
			cli->chkrecvtime = 0;
			len = cs_message_receive(cli->handle, &clicd, buf, cli->sessionkey,3);
			if (len==0) {
				debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," newcamd: client '%s' read failed %d\n", cli->user,len);
				cs_disconnect_cli(cli);
			}
			else if (len<0) {
				debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," newcamd: client '%s' read failed %d(%d)\n", cli->user,len,errno);
				cs_disconnect_cli(cli);
			}
			else if (len>0) {
				cli->lastactivity = GetTickCount();
				switch ( buf[0] ) {
				case MSG_CARD_DATA_REQ:
					memset( buf, 0, sizeof(buf) );
					buf[0] = MSG_CARD_DATA;
					if (!cli->card.caid) {
						buf[4] = cs->card.caid>>8;
						buf[5] = cs->card.caid&0xff;
						//buf[14] = cs->card.nbprov;
						int nbprov=0;
						for(len=0;len<cs->card.nbprov;len++) {
							buf[15+11*nbprov] = cs->card.prov[len]>>16;
							buf[16+11*nbprov] = cs->card.prov[len]>>8;
							buf[17+11*nbprov] = cs->card.prov[len]&0xff;
							nbprov++;
						}
						buf[14] = nbprov;
						cs_message_send(cli->handle, &clicd, buf, 15+11*nbprov, cli->sessionkey);
					}
					else {
						buf[4] = cli->card.caid>>8;
						buf[5] = cli->card.caid&0xff;
						int nbprov=0;
						for(len=0;len<cli->card.nbprov;len++) {
							buf[15+11*nbprov] = cli->card.prov[len]>>16;
							buf[16+11*nbprov] = cli->card.prov[len]>>8;
							buf[17+11*nbprov] = cli->card.prov[len]&0xff;
							nbprov++;
						}
						buf[14] = nbprov;
						cs_message_send(cli->handle, &clicd, buf, 15+11*nbprov, cli->sessionkey);
					}
					//debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," newcamd: send card data to client '%s'\n", cli->user);
					break;
				case 0x80:
				case 0x81:
					//debugdump(buf, len, "ECM: ");
					cli->lastecmtime = GetTickCount();
					cli->ecmnb++;
					memcpy( data, buf, len);
					uint32_t provid = ecm_getprovid( data, clicd.caid );
					if (provid!=0) clicd.provid = provid;

					if (cli->ecm.busy) {
						cli->ecmdenied++;
						// send decode failed
						buf[1] = 0; buf[2] = 0;
						cs_message_send( cli->handle, &clicd, buf, 3, cli->sessionkey);
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <|> decode failed to client '%s' ch %04x:%06x:%04x, too many ecm requests\n", cli->user,clicd.caid,clicd.provid,clicd.sid);
						break;
					}

					char *error = cs_accept_ecm(cs,clicd.caid,clicd.provid,clicd.sid,ecm_getchid(data,clicd.caid));
					if (error) {
						cs->ecmdenied++;
						cli->ecmdenied++;
						cs_message_send( cli->handle, &clicd, buf, 3, cli->sessionkey);
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <|> decode failed to client '%s' ch %04x:%06x:%04x, %s\n",cli->user, clicd.caid,clicd.provid,clicd.sid, error);
						break;
					}

					clicd.caid = cs->card.caid; // if caid == 0

					// Check ECM length
					if ( !accept_ecmlen(len) ) {
						cli->ecmdenied++;
						cs->ecmdenied++;
						buf[1] = 0; buf[2] = 0;
						cs_message_send( cli->handle, &clicd, buf, 3, cli->sessionkey);
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <- ecm length error from client '%s'\n",cli->user);
						break;
					}
					// ACCEPTED
					pthread_mutex_lock(&prg.lockecm); //###

					// Search for ECM
					int ecmid = search_ecmdata_dcw( data,  len, clicd.sid); // dont get failed ecm request from cache
					if ( ecmid!=-1 ) {
						ECM_DATA *ecm=getecmbyid(ecmid);
						ecm->lastrecvtime = GetTickCount();
						//TODO: Add another card for sending ecm
						cs_store_ecmclient(cs, ecmid, cli, clicd.msgid);
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <- ecm from client '%s' ch %04x:%06x:%04x*\n",cli->user,clicd.caid,clicd.provid,clicd.sid);
						cli->ecm.busy=1;
						cli->ecm.hash = ecm->hash;
					}
					else {
						cs->ecmaccepted++;
						// Setup ECM Request for Server(s)
						ecmid = store_ecmdata(cs->id, &data[0], len, clicd.sid, clicd.caid, clicd.provid);
						ECM_DATA *ecm=getecmbyid(ecmid);
						cs_store_ecmclient(cs, ecmid, cli, clicd.msgid);
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <- ecm from client '%s' ch %04x:%06x:%04x (>%dms)\n",cli->user,clicd.caid,clicd.provid,clicd.sid, cli->ecm.dcwtime);
						cli->ecm.busy=1;
						cli->ecm.hash = ecm->hash;
						if (cs->option.fallowcache && cfg.cache.peer) {
							pipe_send_cache_find(ecm, cs);
							ecm->waitcache = 1;
							ecm->dcwstatus = STAT_DCW_WAITCACHE;
						} else ecm->dcwstatus = STAT_DCW_WAIT;
					}
					ecm_check_time = cs_dcw_check_time = 0;

					pthread_mutex_unlock(&prg.lockecm); //###
					break;


#ifdef SRV_CSCACHE
				// Incoming DCW from client
				case 0xC0:
				case 0xC1:
					cli->lastecmtime = GetTickCount();
					if (!cli->ecm.busy) break;

					pthread_mutex_lock(&prg.lockecm); //###

					ECM_DATA *ecm = getecmbyid( cli->ecm.id );
					if (!ecm) {
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <!= wrong cw from Client '%s' ch %04x:%06x:%04x\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->lastecmtime);
						pthread_mutex_unlock(&prg.lockecm);
						break;
					}
					if (ecm->hash!=cli->ecm.hash) {
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <!= wrong cw from Client '%s' ch %04x:%06x:%04x\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->lastecmtime);
						pthread_mutex_unlock(&prg.lockecm);
						break;
					}
					if ( (ecm->caid!=clicd.caid) || (ecm->sid!=clicd.sid) || (ecm->provid!=clicd.provid) || (cli->ecm.climsgid!=clicd.msgid) ) {
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <!= wrong cw from Client '%s' ch %04x:%06x:%04x\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->lastecmtime);
						pthread_mutex_unlock(&prg.lockecm);
						break;
					}
					if ( (buf[0]&0x81)!=ecm->ecm[0] ) {
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <!= wrong cw from Client '%s' ch %04x:%06x:%04x\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->lastecmtime);
						pthread_mutex_unlock(&prg.lockecm);
						break;
					}
					if (buf[2]!=0x10) {
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <!= wrong cw from Client '%s' ch %04x:%06x:%04x\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->lastecmtime);
						pthread_mutex_unlock(&prg.lockecm);
						break;
					}
					// Check for DCW
					if (!acceptDCW(&buf[3])) {
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <!= wrong cw from Client '%s' ch %04x:%06x:%04x\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->lastecmtime);
						pthread_mutex_unlock(&prg.lockecm);
						break;
					}
					debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <= cw from Client '%s' ch %04x:%06x:%04x (%dms)\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->lastecmtime);
					if (ecm->dcwstatus!=STAT_DCW_SUCCESS) {
						static char msg[] = "Good dcw from Newcamd Client";
						ecm->statusmsg = msg;
						// Store ECM Answer
						ecm_setdcw( getcsbyid(ecm->csid), ecm, &buf[3], DCW_SOURCE_CSCLIENT, (ecm->csid<<16)|cli->id );
					}
					else {	//TODO: check same dcw between cards
						if ( memcmp(&ecm->cw, &buf[3],16) ) debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," !!! different dcw from newcamd client '%s'\n", cli->user);
					}

					pthread_mutex_unlock(&prg.lockecm); //###
					break;
#endif

				default:
					if (buf[0]==MSG_KEEPALIVE) {
#ifdef SRV_CSCACHE
						// Check for Cache client??
						if (clicd.sid==(('C'<<8)|'H')) clicd.caid = ('O'<<8)|'K';
#endif
						//debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," <-> keepalive from/to client '%s'\n",cli->user);
						cs_message_send(cli->handle, &clicd, buf, 3, cli->sessionkey);
					}
					else {
						debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," newcamd: unknown message type '%02x' from client '%s'\n",buf[0],cli->user);
						buf[1]=0; buf[2]=0;
						cs_message_send( cli->handle, &clicd, buf, 3, cli->sessionkey);
					}
				} // Switch
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// DCW
///////////////////////////////////////////////////////////////////////////////

void cs_senddcw_cli(struct cs_client_data *cli)
{
	unsigned char buf[CWS_NETMSGSIZE];
	struct cs_custom_data clicd; // Custom data

	if (cli->ecm.status==STAT_DCW_SENT) {
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," +> cw send failed to client '%s', cw already sent\n", cli->user); 
		return;
	}
	if (cli->handle==INVALID_SOCKET) {
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," +> cw send failed to client '%s', client disconnected\n", cli->user); 
		return;
	}
	if (!cli->ecm.busy) {
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," +> cw send failed to client '%s', no ecm request\n", cli->user); 
		return;
	}

	ECM_DATA *ecm = getecmbyid(cli->ecm.id);
	//FREEZE
	int enablefreeze;
	if ( (cli->ecm.lastcaid==ecm->caid)&&(cli->ecm.lastprov==ecm->provid)&&(cli->ecm.lastsid==ecm->sid) ) {
		if ( (cli->ecm.laststatus=1)&&(cli->lastdcwtime+200<GetTickCount()) ) enablefreeze = 1;
	} else cli->zap++;

	cli->ecm.lastcaid = ecm->caid;
	cli->ecm.lastprov = ecm->provid;
	cli->ecm.lastsid = ecm->sid;
	cli->ecm.lastdecodetime = GetTickCount()-cli->ecm.recvtime;
	cli->ecm.lastid = cli->ecm.id;

	clicd.msgid = cli->ecm.climsgid;
	clicd.sid = ecm->sid;
	clicd.caid = ecm->caid;
	clicd.provid = ecm->provid;

	if ( (ecm->hash==cli->ecm.hash)&&(ecm->dcwstatus==STAT_DCW_SUCCESS) ) {
		cli->ecm.lastdcwsrctype = ecm->dcwsrctype;
		cli->ecm.lastdcwsrcid = ecm->dcwsrcid;
		cli->ecm.laststatus=1;
		cli->ecmok++;
		cli->ecmoktime += GetTickCount()-cli->ecm.recvtime;
		buf[0] = ecm->ecm[0];
		buf[1] = 0;
		buf[2] = 0x10;
		memcpy( &buf[3], &ecm->cw, 16 );
		cs_message_send( cli->handle, &clicd, buf, 19, cli->sessionkey);
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," => cw to client '%s' ch %04x:%06x:%04x (%dms)\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->ecm.recvtime);
		cli->lastdcwtime = GetTickCount();
	}
	else { //if (ecm->data->dcwstatus==STAT_DCW_FAILED)
		if (enablefreeze) cli->freeze++;
		cli->ecm.laststatus=0;
		cli->ecm.lastdcwsrctype = DCW_SOURCE_NONE;
		cli->ecm.lastdcwsrcid = 0;
		buf[0] = ecm->ecm[0];
		buf[1] = 0;
		buf[2] = 0;
		cs_message_send(  cli->handle, &clicd, buf, 3, cli->sessionkey);
		debugf(getdbgflag(DBG_NEWCAMD,cli->pid,cli->id)," |> decode failed to client '%s' ch %04x:%06x:%04x (%dms)\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->ecm.recvtime);
	}
	cli->ecm.busy=0;
	cli->ecm.status = STAT_DCW_SENT;
}

// Check sending cw to clients
uint cs_check_sendcw()
{
	struct cardserver_data *cs = cfg.cardserver;
	uint restime = GetTickCount() + 10000;
	uint clitime = 0;

	while (cs) {
		if (  !IS_DISABLED(cs->flags)&&(cs->newcamd.handle>0) ) {
			struct cs_client_data *cli = cs->newcamd.client;
			uint ticks = GetTickCount();
			while (cli) {
				if (  !IS_DISABLED(cli->flags)&&(cli->handle!=INVALID_SOCKET)&&(cli->ecm.busy)&&(cli->ecm.status==STAT_ECM_SENT) ) {
					clitime = ticks+11000;

					pthread_mutex_lock(&prg.lockecm); //###

					// Check for DCW ANSWER
					ECM_DATA *ecm = getecmbyid(cli->ecm.id);
					if (ecm->hash!=cli->ecm.hash) cs_senddcw_cli( cli );
					//debugf(" cs_check_sendcw: %s:%s ch %04x:%06x:%04x\n", cs->name, cli->user, ecm->caid, ecm->provid, ecm->sid);
					// Check for FAILED
					if (ecm->dcwstatus==STAT_DCW_FAILED) {
						cs_senddcw_cli( cli );
					}
					// Check for SUCCESS
					else if (ecm->dcwstatus==STAT_DCW_SUCCESS) {
						// check for client allowed cw time
						if ( (cli->ecm.recvtime+cli->ecm.dcwtime)<=ticks )
							cs_senddcw_cli( cli );
						else
							clitime = cli->ecm.recvtime+cli->ecm.dcwtime;
					}
					// check for timeout
					else if ( (cli->ecm.recvtime+cs->option.dcw.timeout) <= ticks ) {
						cs_senddcw_cli( cli );
					}
					else clitime = cli->ecm.recvtime+cs->option.dcw.timeout;
					if (restime>clitime) restime = clitime;

					pthread_mutex_unlock(&prg.lockecm); //###
				}
				cli = cli->next;
			}
		}
		cs = cs->next;
	}
	return (restime+1);
}
