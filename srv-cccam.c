
///////////////////////////////////////////////////////////////////////////////
// File: srv-cccam.c
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PROTO
///////////////////////////////////////////////////////////////////////////////

void *cc_connect_cli_thread(void *param);
void cc_cli_recvmsg(struct cc_client_data *cli);


struct cccamserver_data *getcccamserverbyid(uint32_t id)
{
	struct cccamserver_data *cccam = cfg.cccam.server;
	while (cccam) {
		if (!(cccam->flags&FLAG_REMOVE))
			if (cccam->id==id) return cccam;
		cccam = cccam->next;
	}
	return NULL;
}

struct cc_client_data *getcccamclientbyid(uint32_t id)
{
	struct cccamserver_data *cccam = cfg.cccam.server;
	while (cccam) {
		if (!(cccam->flags&FLAG_REMOVE)) {
			struct cc_client_data *cli = cccam->client;
			while (cli) {
				if (!(cli->flags&FLAG_REMOVE))
					if (cli->id==id) return cli;
				cli = cli->next;
			}
		}
		cccam = cccam->next;
	}
	return NULL;
}

struct cc_client_data *getcccamclientbyname(struct cccamserver_data *cccam, char *name)
{
	if (!(cccam->flags&FLAG_REMOVE)) {
		struct cc_client_data *cli = cccam->client;
		while (cli) {
			if (!(cli->flags&FLAG_REMOVE))
				if ( !strcmp(cli->user,name) ) return cli;
			cli = cli->next;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// CCCAM SERVER: DISCONNECT CLIENTS
///////////////////////////////////////////////////////////////////////////////

void cc_disconnect_cli(struct cc_client_data *cli)
{
	if (cli->handle>0) {
		debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: client '%s' disconnected \n", cli->user);
		close(cli->handle);
		cli->handle = INVALID_SOCKET;
		uint ticks = GetTickCount();
		cli->uptime += ticks-cli->connected;
		cli->connected = ticks; // Last Seen
	}
}


///////////////////////////////////////////////////////////////////////////////
// CCCAM SERVER: CONNECT CLIENTS
///////////////////////////////////////////////////////////////////////////////

unsigned int seed;
uint8 fast_rnd()
{
  unsigned int offset = 12923;
  unsigned int multiplier = 4079;

  seed = seed * multiplier + offset;
  return (uint8)(seed % 0xFF);
}

///////////////////////////////////////////////////////////////////////////////

int cc_sendinfo_cli(struct cc_client_data *cli, int sendversion)
{
	uint8 buf[CC_MAXMSGSIZE];
	memset(buf, 0, CC_MAXMSGSIZE);
	memcpy(buf, cfg.cccam.nodeid, 8 );
	memcpy(buf + 8, cfg.cccam.version, 32);		// cccam version (ascii)
	memcpy(buf + 40, cfg.cccam.build, 32);       // build number (ascii)
	if (sendversion) {
		buf[38] = REVISION >> 8;
		buf[37] = REVISION & 0xff;
		buf[36] = 0;
		buf[35] = 'S';
		buf[34] = 'C';
		buf[33] = 'M';
	}
	//debugdump(cfg.cccam.nodeid,8,"Sending server data version: %s, build: %s nodeid ", cfg.cccam.version, cfg.cccam.build);
	return cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_SRV_INFO, 0x48, buf);
}

///////////////////////////////////////////////////////////////////////////////

int cc_sendcard_cli(struct cardserver_data *cs, struct cc_client_data *cli, int uphops)
{
	uint8 buf[CC_MAXMSGSIZE];
	memset(buf, 0, sizeof(buf));
	buf[0] = cs->id >> 24;
	buf[1] = cs->id >> 16;
	buf[2] = cs->id >> 8;
	buf[3] = cs->id & 0xff;
	buf[4] = cs->id >> 24;
	buf[5] = cs->id >> 16;
	buf[6] = cs->id >> 8;
	buf[7] = cs->id & 0xff;
	buf[8] = cs->card.caid >> 8;
	buf[9] = cs->card.caid & 0xff;
	buf[10] = uphops;
	buf[11] = cli->dnhops; // Dnhops
	//buf[20] = cs->card.nbprov;
	int j;
	int nbprov = 0;
	for (j=0; j<cs->card.nbprov; j++) {
		if ( card_sharelimits(cli->sharelimits, cs->card.caid, cs->card.prov[j]) ) {
			//memcpy(buf + 21 + (j*7), card->provs[j], 7);
			buf[21+nbprov*7] = 0xff&(cs->card.prov[j]>>16);
			buf[22+nbprov*7] = 0xff&(cs->card.prov[j]>>8);
			buf[23+nbprov*7] = 0xff&(cs->card.prov[j]);
/*
			buf[24+nbprov*7] = 0xff&(cs->card.prov[j].ua>>24);
			buf[25+nbprov*7] = 0xff&(cs->card.prov[j].ua>>16);
			buf[26+nbprov*7] = 0xff&(cs->card.prov[j].ua>>8);
			buf[27+nbprov*7] = 0xff&(cs->card.prov[j].ua);
*/
			nbprov++;
		}
	}
	if (!nbprov) return 0; // Denied
	buf[20] = nbprov;
	buf[21 + (nbprov*7)] = 1;
	memcpy(buf + 22 + (nbprov*7), cfg.cccam.nodeid, 8);
	cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_CARD_ADD, 30 + (nbprov*7), buf);
	return 1;
}


///////////////////////////////////////////////////////////////////////////////

void cc_sendcards_cli(struct cc_client_data *cli)
{
	int nbcard=0;
	struct cardserver_data *cs = cfg.cardserver;

	int i;
	if (cli->csport[0]) {
		for(i=0;i<MAX_CSPORTS;i++) {
			if(cli->csport[i]) {
				cs = getcsbyport(cli->csport[i]);
				if (cs)
					if (cc_sendcard_cli(cs, cli,0)) nbcard++;
			} else break;
		}
	}
	else if (cfg.cccam.csport[0]) {
		for(i=0;i<MAX_CSPORTS;i++) {
			if(cfg.cccam.csport[i]) {
				cs = getcsbyport(cfg.cccam.csport[i]);
				if (cs)
					if (cc_sendcard_cli(cs, cli,0)) nbcard++;
			} else break;
		}
	}
	else {
		while (cs) {
			if (cc_sendcard_cli(cs, cli,0)) nbcard++;
			cs = cs->next;
		}
	}

	debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: %d cards --> client(%s)\n",  nbcard, cli->user);
}

struct cc_connect_cli_data {
	struct cccamserver_data *cccam;
	int sock;
	uint32 ip;
};

///////////////////////////////////////////////////////////////////////////////

void *cc_connect_cli(struct cc_connect_cli_data *param)
{
	uint8 buf[CC_MAXMSGSIZE];
	uint8 data[20];
	int i;
	struct cc_crypt_block sendblock;	// crypto state block
	struct cc_crypt_block recvblock;	// crypto state block
	char usr[64];
	char pwd[255];
	// Store data from param
	struct cccamserver_data *cccam = param->cccam;
	int sock = param->sock;
	uint32 ip = param->ip;
	free(param);

#ifdef CHECK_HACKER
	struct ip_hacker_data *iplist = iplist_get( ip );
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
	iplist->lastseen = GetTickCount();
	iplist->count++;
#endif

	memset(usr, 0, sizeof(usr));
	memset(pwd, 0, sizeof(pwd));

	// create & send random seed
	for(i=0; i<12; i++ ) data[i]=fast_rnd();
	// Create Multics ID
	data[3] = (data[0]^'M') + data[1] + data[2];
	data[7] = data[4] + (data[5]^'C') + data[6];
	data[11] = data[8] + data[9] + (data[10]^'S');
	//Create checksum for "O" cccam:
	for (i = 0; i < 4; i++) {
		data[12 + i] = (data[i] + data[4 + i] + data[8 + i]) & 0xff;
	}
	send_nonb(sock, data, 16, 100);
	//XOR init bytes with 'CCcam'
	cc_crypt_xor(data);
	//SHA1
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, data, 16);
	SHA1_Final(buf, &ctx);

	//init crypto states
	cc_crypt_init(&sendblock, buf, 20);
	cc_decrypt(&sendblock, data, 16);
	cc_crypt_init(&recvblock, data, 16);
	cc_decrypt(&recvblock, buf, 20);

	//debugdump(buf, 20, "CCcam: SHA1 hash:");
	memcpy(usr,buf,20);
	if ((i=recv_nonb(sock, buf, 20,3000)) == 20) {
#ifdef DEBUG_NETWORK
		if (flag_debugnet) {
			debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," CCcam%d: receive SHA1 hash %d\n", cccam->id, i);
			debughex(buf,i);
		}
#endif
		cc_decrypt(&recvblock, buf, 20);
		//debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," Decrypted SHA1 hash (20):\n"); debughex(buf,20);
		if ( memcmp(buf,usr,20)!=0 ) {
			//debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," cc_connect_cli(): wrong sha1 hash from client! (%s)\n",ip2string(ip));
			close(sock);
			return NULL;
		}
	} else {
		//debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," cc_connect_cli(): recv sha1 timeout\n");
		close(sock);
		return NULL;
	}

	// receive username
	i = recv_nonb(sock, buf, 20,3000);
#ifdef DEBUG_NETWORK
	if (flag_debugnet) {
		debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," CCcam%d: receive username %d\n", cccam->id, i);
		debughex(buf,i);
	}
#endif
	if (i == 20) {
		cc_decrypt(&recvblock, buf, i);
		memcpy(usr,buf,20);
		//strcpy(usr, (char*)buf);
		//debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," cc_connect_cli(): username '%s'\n", usr);
	}
	else {
		//debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," cc_connect_cli(): recv user timeout\n");
		close(sock);
		return NULL;
	}

#ifdef CHECK_HACKER
	strcpy(iplist->user,usr);
#endif


	// Check for username
	pthread_mutex_lock(&prg.lockcccli);
	int found = 0;
	struct cc_client_data *cli = cccam->client;
	while (cli) {
		if (!strcmp(cli->user,usr)) {
			if (IS_DISABLED(cli->flags)) { // Connect only enabled clients
				pthread_mutex_unlock(&prg.lockcccli);
				debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: connection refused for client '%s' (%s), client disabled\n", cccam->id, usr, ip2string(ip));
				close(sock);
				return NULL;
			}
			found = 1;
			break;
		}
		cli = cli->next;
	}
	pthread_mutex_unlock(&prg.lockcccli);

	if (!found) {
		debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," CCcam%d: Unknown Client '%s' (%s)\n", cccam->id, usr, ip2string(ip));
		close(sock);
		return NULL;
	}

	// Check for Host
	if (cli->host) {
		struct host_data *host = cli->host;
		host->clip = ip;
		if ( host->ip && (host->ip!=ip) ) {
			uint sec = getseconds()+60;
			if ( host->checkiptime > sec ) host->checkiptime = sec;
			debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Aborted connection from Client '%s' (%s), ip refused\n", cccam->id, usr, ip2string(ip));
			cli->nbloginerror++;
			close(sock);
			return NULL;
		}
	}

	// Encrypted Password
	if ((i=recv_nonb(sock, buf, 6,3000)) == 6) {
		memset(pwd, 0, sizeof(pwd));
		strcpy(pwd, cli->pass);
		cc_encrypt(&recvblock, (uint8*)pwd, strlen(pwd));
		cc_decrypt(&recvblock, buf, 6);
		if ( memcmp(buf,"CCcam\0",6) ) {
			debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: login failed from client '%s'\n", cccam->id, usr);
			cli->nbloginerror++;
			close(sock);
			return NULL;
		}
	} 
	else {
		debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: login failed from client '%s', error receiving crypted password\n", cccam->id, usr);
		cli->nbloginerror++;
		close(sock);
		return NULL;
	}

	if (cli->ip==ip) cli->nbdiffip++;

	// Disconnect old connection if isthere any
	if (cli->handle>0) {
		if (cli->ip==ip) {
			debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Client '%s' (%s) already connected\n", cccam->id, usr, ip2string(ip));
			cc_disconnect_cli(cli);
		}
		else {
			if ( (GetTickCount()-cli->lastecmtime) < 60000 ) {
				debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Client '%s' (%s) already connected with different ip, Connection aborted for recent activity\n", cccam->id, usr, ip2string(ip));
				cli->nbloginerror++;
				close(sock);
				return NULL;
			}
			debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Client '%s' (%s) already connected with different ip \n", cccam->id, usr, ip2string(ip));
			cc_disconnect_cli(cli);
		}
	}

	cli->nblogin++;

	// Send passwd ack
	memset(buf, 0, 20);
	memcpy(buf, "CCcam\0", 6);
	//debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id),"Server: send ack '%s'\n",buf);
	cc_encrypt(&sendblock, buf, 20);
	send_nonb(sock, buf, 20, 100);


	//cli->ecmnb=0;
	//cli->ecmok=0;
	memcpy(&cli->sendblock,&sendblock,sizeof(sendblock));
	memcpy(&cli->recvblock,&recvblock,sizeof(recvblock));
	debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: client '%s' connected\n", cccam->id, usr);

	// Recv cli data
	memset(buf, 0, sizeof(buf));
	i = cc_msg_recv( sock, &cli->recvblock, buf, 3000);
	if ( i<65 ) {
		debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Error recv cli data (%d)\n", cccam->id,i);
		debughex(buf,i);
		close(sock);
		return NULL;
	}
	// Setup Client Data
	// pthread_mutex_lock(&prg.lockcccli);
	memcpy( cli->nodeid, buf+24, 8);
	memcpy( cli->version, buf+33, 31);
	memcpy( cli->build, buf+65, 31 );
	debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: client '%s' running version %s build %s\n", cccam->id, usr, cli->version, cli->build);  // cli->nodeid,8,
	cli->cardsent = 0;
	cli->ecm.busy = 0;
	cli->connected = cli->lastactivity = cli->lastecmtime = GetTickCount();
	cli->ip = ip;
	cli->chkrecvtime = 0;
	cli->handle = sock;

#ifdef CHECK_HACKER
	iplist->count = 10; // for reconnect
#endif

//	pthread_mutex_unlock(&prg.lockcccli);

	// send cli data ack
	cc_msg_send( sock, &cli->sendblock, CC_MSG_CLI_INFO, 0, NULL);
	//cc_msg_send( sock, &cli->sendblock, CC_MSG_BAD_ECM, 0, NULL);
	int sendversion = ( (cli->version[28]=='W')&&(cli->version[29]='H')&&(cli->version[30]='O') );
	cc_sendinfo_cli(cli, sendversion);
	//cc_msg_send( sock, &cli->sendblock, CC_MSG_BAD_ECM, 0, NULL);
	cli->cardsent = 1;
	//TODO: read from client packet CC_MSG_BAD_ECM
	//len = cc_msg_recv(cli->handle, &cli->recvblock, buf, 3);
	usleep(10000);
	cc_sendcards_cli(cli);
	cli->handle = sock;
	pipe_wakeup( srvsocks[1] );
	return cli;
}

////////////////////////////////////////////////////////////////////////////////

void *cc_connect_cli_thread(void *param)
{
	SOCKET client_sock =-1;
	struct sockaddr_in client_addr;
	socklen_t socklen = sizeof(client_addr);

	pthread_t srv_tid;

	while(1) {
		pthread_mutex_lock(&prg.locksrvcc);

		struct pollfd pfd[MAX_CSPORTS];
		int pfdcount = 0;

		struct cccamserver_data *cccam = cfg.cccam.server;
		while (cccam) {
			if ( !IS_DISABLED(cccam->flags) && (cccam->handle>0) ) {
				cccam->ipoll = pfdcount;
				pfd[pfdcount].fd = cccam->handle;
				pfd[pfdcount++].events = POLLIN | POLLPRI;
			} else cccam->ipoll = -1;
			cccam = cccam->next;
		}

		int retval = poll(pfd, pfdcount, 3000);

		if (retval>0) {
			struct cccamserver_data *cccam = cfg.cccam.server;
			while (cccam) {
				if ( !IS_DISABLED(cccam->flags) && (cccam->handle>0) && (cccam->ipoll>=0) && (cccam->handle==pfd[cccam->ipoll].fd) ) {
					if ( pfd[cccam->ipoll].revents & (POLLIN|POLLPRI) ) {
						client_sock = accept( cccam->handle, (struct sockaddr*)&client_addr, /*(socklen_t*)*/&socklen);
						if ( client_sock<=0 ) {
							debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," CCcam%d: Accept failed (errno=%d)\n", cccam->id,errno);
							//usleep(30000);
						}
						else {
							///SetSocketKeepalive(client_sock); no keepalive
							debugf(getdbgflag(DBG_CCCAM,cccam->id,0)," CCcam%d: new connection (%s)\n", cccam->id, ip2string(client_addr.sin_addr.s_addr) );
							struct cc_connect_cli_data *clicondata = malloc( sizeof(struct cc_connect_cli_data) );
							clicondata->cccam = cccam; 
							clicondata->sock = client_sock; 
							clicondata->ip = client_addr.sin_addr.s_addr;
							create_prio_thread(&srv_tid, (threadfn)cc_connect_cli,clicondata, 50);
						}
					}
				}
				cccam = cccam->next;
			}
		}
		pthread_mutex_unlock(&prg.locksrvcc);
		usleep(3000);
	}// While
}



///////////////////////////////////////////////////////////////////////////////
// CCCAM SERVER: RECEIVE MESSAGES FROM CLIENTS
///////////////////////////////////////////////////////////////////////////////

void cc_store_ecmclient(int ecmid, struct cc_client_data *cli)
{
	uint32 ticks = GetTickCount();

	cli->ecm.dcwtime = cli->dcwtime;
	cli->ecm.recvtime = ticks;
	cli->ecm.id = ecmid;
    cli->ecm.status = STAT_ECM_SENT;
	ecm_addip(ecmid, cli->ip);
}

///////////////////////////////////////////////////////////////////////////////

// Receive messages from client
void cc_cli_recvmsg(struct cc_client_data *cli)
{     
	unsigned char buf[CC_MAXMSGSIZE];
	unsigned char data[CC_MAXMSGSIZE]; // for other use
	unsigned int cardid;
	int len;

	if (cli->handle>0) {
		len = cc_msg_chkrecv(cli->handle,&cli->recvblock);
		if (len==0) {
			debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: client '%s' read failed %d\n", cli->user,len);
			cc_disconnect_cli(cli);
		}
		else if (len==-1) {
			if (!cli->chkrecvtime) cli->chkrecvtime = GetTickCount();
			else if ( (cli->chkrecvtime+500)<GetTickCount() ) {
				debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: client '%s' read failed %d\n", cli->user,len);
				cc_disconnect_cli(cli);
			}
		}
		else if (len>0) {
			cli->chkrecvtime = 0;
			len = cc_msg_recv( cli->handle, &cli->recvblock, buf, 3);
			if (len==0) {
				debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: client '%s' read failed %d\n", cli->user,len);
				cc_disconnect_cli(cli);
			}
			else if (len<0) {
				debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: client '%s' read failed %d(%d)\n", cli->user,len,errno);
				cc_disconnect_cli(cli);
			}
			else if (len>0) {
				cli->lastactivity = GetTickCount();
				switch (buf[1]) {
					 case CC_MSG_ECM_REQUEST:
						cli->ecmnb++;
						cli->lastecmtime = GetTickCount();
	
						if (cli->ecm.busy) {
							// send decode failed
							cli->ecmdenied++;
							cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_ECM_NOK2, 0, NULL);
							debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," <|> decode failed to CCcam client '%s', too many ecm requests\n", cli->user);
							break;
						}
						//Check for card availability
						memcpy( data, buf+17, len-17);
						cardid = buf[10]<<24 | buf[11]<<16 | buf[12]<<8 | buf[13];
						uint16 caid = buf[4]<<8 | buf[5];
						uint16 sid = buf[14]<<8 | buf[15];
						uint32_t provid = ecm_getprovid( data, caid );
						if (provid==0) provid = buf[6]<<24 | buf[7]<<16 | buf[8]<<8 | buf[9];

						// Check for Profile
						struct cardserver_data *cs=getcsbyid( cardid );
						if (!cs) {
							// check for cs by caid:prov
							cs = getcsbycaidprov(caid,provid);
							if (!cs) {
								cli->ecmdenied++;
								cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_ECM_NOK2, 0, NULL);
								debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," <|> decode failed to CCcam client '%s', card-id %x (%04x:%06x) not found\n",cli->user, cardid, caid,provid);
								break;
							}
						}

						char *error = cs_accept_ecm(cs,caid,provid,sid,ecm_getchid(data,caid));
						if (error) {
							cs->ecmdenied++;
							cli->ecmdenied++;
							cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_ECM_NOK2, 0, NULL);
							debugf(getdbgflagpro(DBG_CCCAM,cli->srvid,cli->id,cs->id)," <|> decode failed to CCcam client '%s' ch %04x:%06x:%04x, %s\n", cli->user, caid,provid,sid, error);
							break;
						}

						// Check ECM length
						if ( !accept_ecmlen(len) ) {
							cli->ecmdenied++;
							cs->ecmdenied++;
							buf[1] = 0; buf[2] = 0;
							cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_ECM_NOK2, 0, NULL);
							debugf(getdbgflagpro(DBG_CCCAM,cli->srvid,cli->id,cs->id)," <|> decode failed to CCcam client '%s' ch %04x:%06x:%04x, ecm length error(%d)\n", cli->user,caid,provid,sid, len);
							break;
						}
	
						// ACCEPTED
						//cs->ecmaccepted++;
						//cli->ecmaccepted++;

						// XXX: check ecm tag = 0x80,0x81

						pthread_mutex_lock(&prg.lockecm);
	
						// Search for ECM
						int ecmid = search_ecmdata_dcw( data,  len-17, sid); // dont get failed ecm request from cache
						if ( ecmid!=-1 ) {
							ECM_DATA *ecm=getecmbyid(ecmid);
							ecm->lastrecvtime = GetTickCount();
							//TODO: Add another card for sending ecm
							cc_store_ecmclient(ecmid, cli);
							debugf(getdbgflagpro(DBG_CCCAM,cli->srvid,cli->id,cs->id)," <- ecm from CCcam client '%s' ch %04x:%06x:%04x*\n", cli->user, caid, provid, sid);
							cli->ecm.busy=1;
							cli->ecm.hash = ecm->hash;
							cli->ecm.cardid = cardid;
						}
						else {
							cs->ecmaccepted++;
							// Setup ECM Request for Server(s)
							ecmid = store_ecmdata(cs->id, &data[0], len-17, sid, caid, provid);
							ECM_DATA *ecm=getecmbyid(ecmid);
							cc_store_ecmclient(ecmid, cli);
							debugf(getdbgflagpro(DBG_CCCAM,cli->srvid,cli->id,cs->id)," <- ecm from CCcam client '%s' ch %04x:%06x:%04x (>%dms)\n",cli->user,caid,provid,sid, cli->ecm.dcwtime);
							cli->ecm.busy=1;
							cli->ecm.hash = ecm->hash;
							cli->ecm.cardid = cardid;
							if (cs->option.fallowcache && cfg.cache.peer) {
								pipe_send_cache_find(ecm, cs);
								ecm->waitcache = 1;
								ecm->dcwstatus = STAT_DCW_WAITCACHE;
							} else ecm->dcwstatus = STAT_DCW_WAIT;
						}
						checkfreeze_checkECM( ecmid, cli->ecm.lastid);

						ecm_check_time = cc_dcw_check_time = 0;

						pthread_mutex_unlock(&prg.lockecm);
						break;
		
					 case CC_MSG_KEEPALIVE:
						//printf(" Keepalive from client '%s'\n",cli->user);
						cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_KEEPALIVE, 0, NULL);
						break;
					 case CC_MSG_BAD_ECM:
						//debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: cmd 0x05 ACK from client '%s'\n",cli->user);
						//if (cli->cardsent==0) {
						//	cc_sendcards_cli(cli);
						//	cli->cardsent=1;
						//}
	
						break;
					 //default: debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam: Unknown Packet ID : %02x from client '%s'\n", buf[1],cli->user);
				}
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// CCCAM SERVER: SEND DCW TO CLIENTS
////////////////////////////////////////////////////////////////////////////////

void cc_senddcw_cli(struct cc_client_data *cli)
{
	unsigned char buf[CC_MAXMSGSIZE];

	if (cli->ecm.status==STAT_DCW_SENT) {
		debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," +> cw send failed to CCcam client '%s', cw already sent\n", cli->user); 
		return;
	}
	if (cli->handle<=0) {
		debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," +> cw send failed to CCcam client '%s', client disconnected\n", cli->user); 
		return;
	}
	if (!cli->ecm.busy) {
		debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," +> cw send failed to CCcam client '%s', no ecm request\n", cli->user); 
		return;
	}

	ECM_DATA *ecm = getecmbyid(cli->ecm.id);
	//FREEZE
	int enablefreeze=0;
	if ( (cli->ecm.lastcaid==ecm->caid)&&(cli->ecm.lastprov==ecm->provid)&&(cli->ecm.lastsid==ecm->sid) ) {
		if ( (cli->ecm.laststatus=1)&&(cli->lastdcwtime+200<GetTickCount()) ) enablefreeze = 1;
	} else cli->zap++;
	//
	cli->ecm.lastcaid = ecm->caid;
	cli->ecm.lastprov = ecm->provid;
	cli->ecm.lastsid = ecm->sid;
	cli->ecm.lastdecodetime = GetTickCount()-cli->ecm.recvtime;
	cli->ecm.lastid = cli->ecm.id;

	if ( (ecm->dcwstatus==STAT_DCW_SUCCESS)&&(ecm->hash==cli->ecm.hash) ) {
		cli->ecm.lastdcwsrctype = ecm->dcwsrctype;
		cli->ecm.lastdcwsrcid = ecm->dcwsrcid;
		cli->ecm.laststatus=1;
		cli->ecmok++;
		cli->lastdcwtime = GetTickCount();
		cli->ecmoktime += GetTickCount()-cli->ecm.recvtime;
		//cli->lastecmoktime = GetTickCount()-cli->ecm.recvtime;
		memcpy( buf, ecm->cw, 16 );
		cc_crypt_cw( cli->nodeid, cli->ecm.cardid , buf);
		cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_ECM_REQUEST, 16, buf);
		cc_encrypt(&cli->sendblock, buf, 16); // additional crypto step

		debugf(getdbgflagpro(DBG_CCCAM,cli->srvid,cli->id,ecm->csid)," => cw to CCcam client '%s' ch %04x:%06x:%04x (%dms)\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->ecm.recvtime);
	}
	else { //if (ecm->data->dcwstatus==STAT_DCW_FAILED)
		if (enablefreeze) {
			cli->freeze++;
/*
			fdebugf( "## FREEZE:\nClient: %s - CHN: %04X:%06X:%04X (%dms)\n", cli->user, ecm->caid, ecm->provid, ecm->sid, GetTickCount()-cli->ecm.recvtime);
			char http_buf[4048];
			array2hex( ecm->ecm, http_buf, ecm->ecmlen);
			fdebugf( "ECM: %s\n", http_buf);

#ifdef CHECK_NEXTDCW
			if ( (ecm->lastdecode.status>0)&&(ecm->lastdecode.counter>0) ) {
				array2hex( ecm->lastdecode.dcw, http_buf, 16 );
				fdebugf( "Last DCW: %s\n", http_buf);
				fdebugf( "Total wrong DCW = %d\n", ecm->lastdecode.error);
				fdebugf( "Total Consecutif DCW = %d\nECM Interval = %ds\n", ecm->lastdecode.counter, ecm->lastdecode.dcwchangetime/1000);
			}
#endif
			if (ecm->server[0].srvid) {
				int i;
				for(i=0; i<20; i++) {
					if (!ecm->server[i].srvid) break;
					char* str_srvstatus[] = { "WAIT", "OK", "NOK", "BUSY" };
					struct cs_server_data *srv = getsrvbyid(ecm->server[i].srvid);
					if (srv) {
						fdebugf( "%d (%s:%d) -> %s (%dms)", i+1, srv->host->name, srv->port, str_srvstatus[ecm->server[i].flag], ecm->server[i].sendtime - ecm->recvtime );
						// Recv Time
						if (ecm->server[i].statustime>ecm->server[i].sendtime)
							fdebugf( " from %dms to %dms", ecm->server[i].statustime - ecm->recvtime, ecm->server[i].statustime-ecm->server[i].sendtime );
						// DCW
						if (ecm->server[i].flag==ECM_SRV_REPLY_GOOD) {
							array2hex( ecm->server[i].dcw, http_buf, 16 );
							fdebugf( "DCW: %s", http_buf);
						}
						fdebugf( "\n");
					}
				}
			}
*/
		}
		cli->ecm.lastdcwsrctype = DCW_SOURCE_NONE;
		cli->ecm.lastdcwsrcid = 0;
		cli->ecm.laststatus=0;
		cc_msg_send( cli->handle, &cli->sendblock, CC_MSG_ECM_NOK1, 0, NULL);
		debugf(getdbgflagpro(DBG_CCCAM,cli->srvid,cli->id,ecm->csid)," |> decode failed to CCcam client '%s' ch %04x:%06x:%04x (%dms)\n", cli->user, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-cli->ecm.recvtime);
	}
	cli->ecm.busy=0;
	cli->ecm.status = STAT_DCW_SENT;
}

///////////////////////////////////////////////////////////////////////////////

// Check sending cw to clients
uint32 cc_check_sendcw()
{
	uint restime = GetTickCount() + 10000;
	uint clitime = 0;

	struct cccamserver_data *cccam = cfg.cccam.server;
	while (cccam) {
		if ( !IS_DISABLED(cccam->flags) && (cccam->handle>0) ) {
			struct cc_client_data *cli = cccam->client;
			uint ticks = GetTickCount();
			while (cli) {
				if ( !IS_DISABLED(cli->flags)&&(cli->handle>0)&&(cli->ecm.busy) ) { //&&(cli->ecm.status==STAT_ECM_SENT) ) {
					clitime = ticks+11000;
					pthread_mutex_lock(&prg.lockecm); //###
					// Check for DCW ANSWER
					ECM_DATA *ecm = getecmbyid(cli->ecm.id);
					if (ecm->hash!=cli->ecm.hash) cc_senddcw_cli( cli );
					// Check for FAILED
					if (ecm->dcwstatus==STAT_DCW_FAILED) {
						static char msg[] = "decode failed";
						cli->ecm.statmsg=msg;
						cc_senddcw_cli( cli );
					}
					// Check for SUCCESS
					else if (ecm->dcwstatus==STAT_DCW_SUCCESS) {
						// check for client allowed cw time
						if ( (cli->ecm.recvtime+cli->ecm.dcwtime)<=ticks ) {
							static char msg[] = "good dcw";
							cli->ecm.statmsg = msg;
							cc_senddcw_cli( cli );
						} else clitime = cli->ecm.recvtime+cli->ecm.dcwtime;
					}
					// check for timeout / no server again
					else if (ecm->dcwstatus==STAT_DCW_WAIT){
						uint32 timeout;
						struct cardserver_data *cs = getcsbyid( ecm->csid);
						if (cs) timeout = cs->option.dcw.timeout; else timeout = 5700;
						if ( (cli->ecm.recvtime+timeout) < ticks ) {
							static char msg[] = "dcw timeout";
							cli->ecm.statmsg=msg;
							cc_senddcw_cli( cli );
						} else clitime = cli->ecm.recvtime+timeout;
					}
					if (restime>clitime) restime = clitime;
					pthread_mutex_unlock(&prg.lockecm); //###
				}
				cli = cli->next;
			}
		}
		cccam = cccam->next;
	}
	return (restime+1);
}


///////////////////////////////////////////////////////////////////////////////
// CCCAM SERVER: START/STOP
///////////////////////////////////////////////////////////////////////////////

pthread_t cc_cli_tid;
int start_thread_cccam()
{
	create_prio_thread(&cc_cli_tid, cc_connect_cli_thread, NULL, 50); // Lock server
	return 0;
}

void done_cccamsrv()
{
 // if (close(cfg.cccam.handle)) debugf(getdbgflag(DBG_CCCAM,cccam->id,0),"Close failed(%d)",cfg.cccam.handle);
}

