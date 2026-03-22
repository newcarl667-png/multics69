
struct cs_card_data *cc_getcardbyid( struct cs_server_data *srv, uint32 id );
void cc_disconnect_srv(struct cs_server_data *srv);
int cc_sendecm_srv(struct cs_server_data *srv, ECM_DATA *ecm);

///////////////////////////////////////////////////////////////////////////////
// CLIENT CONNECT
///////////////////////////////////////////////////////////////////////////////

struct cs_card_data *cc_getcardbyid( struct cs_server_data *srv, uint32 id )
{
	struct cs_card_data *card = srv->card;
	while (card) {
		if (card->shareid==id) return card;
		card = card->next;
	}
	return NULL;
}


// Send Client info to server.
int cc_sendinfo_srv(struct cs_server_data *srv, int ismultics)
{
	uint8 buf[CC_MAXMSGSIZE];
	memset(buf, 0, CC_MAXMSGSIZE);
	memcpy(buf, srv->user, 20);
	memcpy(buf + 20, cfg.cccam.nodeid, 8 );
	buf[28] = 0; //srv->wantemus;
	memcpy(buf + 29, cfg.cccam.version, 32);	// cccam version (ascii)
	memcpy(buf + 61, cfg.cccam.build, 32);	// build number (ascii)
	debugf( getdbgflag(DBG_SERVER, 0, srv->id), " Server: send client info User: '%s', Version: '%s', Build: '%s'\n", srv->user, cfg.cccam.version, cfg.cccam.build);
	return cc_msg_send( srv->handle, &srv->sendblock, CC_MSG_CLI_INFO, 20 + 8 + 1 + 64, buf);
}


///////////////////////////////////////////////////////////////////////////////
// Connect to a server.
// Return
// 0: no error
int cc_connect_srv(struct cs_server_data *srv, int fd)
{
	int n;
	uint8 data[20];
	uint8 hash[SHA_DIGEST_LENGTH];
	uint8 buf[CC_MAXMSGSIZE];
	char pwd[64];
	//
	if (fd < 0) return -1;
	// INIT
	srv->progname = NULL;
	memset( srv->version, 0, sizeof(srv->version) );
	// get init seed(random) from server
	if((n = recv_nonb(fd, data, 16,3000)) != 16) {
		static char msg[]= "Server does not return init sequence";
		srv->statmsg = msg;
		debugf( getdbgflag(DBG_SERVER, 0, srv->id), " Server (%s:%d) does not return 16 bytes\n", srv->host->name,srv->port);
		close(fd);
		return -2;
	}

#ifdef DEBUG_NETWORK
	if (flag_debugnet) {
		debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: receive server init seed (%d)\n",n);
		debughex(data,n);
	}
#endif

	int ismultics = 0;
	cc_crypt_xor(data);  // XOR init bytes with 'CCcam'

	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, data, 16);
	SHA1_Final(hash, &ctx);

	//debugdump(hash, sizeof(hash), "CCcam: sha1 hash:");

	//initialisate crypto states
	cc_crypt_init(&srv->recvblock, hash, 20);
	cc_decrypt(&srv->recvblock, data, 16); 
	cc_crypt_init(&srv->sendblock, data, 16);
	cc_decrypt(&srv->sendblock, hash, 20);

	cc_msg_send( fd, &srv->sendblock, CC_MSG_NO_HEADER, 20,hash);   // send crypted hash to server
	memset(buf, 0, sizeof(buf));
	memcpy(buf, srv->user, 20);
	//debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: username '%s'\n",srv->username);
	cc_msg_send( fd, &srv->sendblock, CC_MSG_NO_HEADER, 20, buf);    // send usr '0' padded -> 20 bytes

	memset(buf, 0, sizeof(buf));
	memset(pwd, 0, sizeof(pwd));

	//debugf("CCcam: 'CCcam' xor\n");
	memcpy(buf, "CCcam\0", 6);
	strncpy(pwd, srv->pass, 63);
	cc_encrypt(&srv->sendblock, (uint8 *)pwd, strlen(pwd));
	cc_msg_send( fd, &srv->sendblock, CC_MSG_NO_HEADER, 6, buf); // send 'CCcam' xor w/ pwd
	if ((n = recv_nonb(fd, data, 20,3000)) != 20) {
		static char msg[]= "Password ACK not received";
		srv->statmsg = msg;
		debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: login failed to Server (%s:%d), pwd ack not received (n = %d)\n",srv->host->name,srv->port, n);
		return -2;
	}
	cc_decrypt(&srv->recvblock, data, 20);
	//hexdump(data, 20, "CCcam: pwd ack received:");

	if (memcmp(data, buf, 5)) {  // check server response
		static char msg[]= "Invalid user/pass";
		srv->statmsg = msg;
		debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: login failed to Server (%s:%d), usr/pwd invalid\n",srv->host->name,srv->port);
		return -2;
	}// else debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: login succeeded to Server (%s:%d)\n",srv->host->name,srv->port);

	srv->handle = fd;
	if (!cc_sendinfo_srv(srv,ismultics)) {
		srv->handle = -1;
		static char msg[]= "Error sending client data";
		srv->statmsg = msg;
		debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: login failed to Server (%s:%d), could not send client data\n",srv->host->name,srv->port);
		return -3;
	}

	static char msg[]= "Connected";
	srv->statmsg = msg;

	srv->keepalivesent = 0;
	srv->keepalivetime = GetTickCount();
	srv->connected = GetTickCount();

	srv->busy = 0;
	srv->lastecmoktime = 0;
	srv->lastecmtime = 0;
	srv->lastdcwtime = 0;
	srv->chkrecvtime = 0;

	memset(srv->version,0,32);
	pipe_wakeup( srvsocks[1] );
	return 0;
}


void cc_disconnect_srv(struct cs_server_data *srv)
{
	if (srv->handle>0) {
		static char msg[]= "Disconnected";
		srv->statmsg = msg;
		debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: server (%s:%d) disconnected\n", srv->host->name,srv->port);
		close(srv->handle);
		srv->handle = INVALID_SOCKET;
		if (srv->busy) ecm_setsrvflag(srv->busyecmid, srv->id, ECM_SRV_EXCLUDE);
		uint ticks = GetTickCount();
		srv->uptime += ticks-srv->connected;
		srv->connected = ticks; // Last Seen
		srv->keepalivetime = 0;
		srv->keepalivesent = 0;
		srv->host->checkiptime = 60; // maybe ip changed
		// Remove Cards
		free_cardlist(srv->card);
		srv->card = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////

void cc_srv_recvmsg(struct cs_server_data *srv)
{     
	unsigned char buf[CC_MAXMSGSIZE];
	struct cs_card_data *card;
	struct cardserver_data *cs;
	int i, len;
	ECM_DATA *ecm;

	if ( (srv->type==TYPE_CCCAM)&&(srv->handle>0) ) {
		len = cc_msg_chkrecv(srv->handle,&srv->recvblock);
		if (len==0) {
			debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: server (%s:%d) read failed %d\n", srv->host->name, srv->port, len);
			cc_disconnect_srv(srv);
		}
		else if (len==-1) {
			if (!srv->chkrecvtime) srv->chkrecvtime = GetTickCount();
			else if ( (srv->chkrecvtime+500)<GetTickCount() ) {
				debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: server (%s:%d) read failed %d\n", srv->host->name, srv->port, len);
				cc_disconnect_srv(srv);
			}
		}
		else if (len>0) {
			srv->chkrecvtime = 0;
			len = cc_msg_recv(srv->handle, &srv->recvblock, buf, 3);
			if (len==0) {
				debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: server (%s:%d) read failed %d\n", srv->host->name, srv->port, len);
				cc_disconnect_srv(srv);
			}
			else if (len<0) {
				debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: server (%s:%d) read failed %d(%d)\n", srv->host->name, srv->port, len, errno);
				cc_disconnect_srv(srv);
			}
			else if (len>0) {
				switch (buf[1]) {
				case CC_MSG_CLI_INFO:
					debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: Client data ACK from Server (%s:%d)\n", srv->host->name,srv->port);
					break;

				case CC_MSG_ECM_REQUEST: // Get CW
					if (!srv->busy) {
						debugf( getdbgflag(DBG_SERVER, 0, srv->id), " [!] dcw error from server (%s:%d), unknown ecm request\n",srv->host->name,srv->port);
						break;
					}

					uint8 dcw[16];
					cc_crypt_cw( cfg.cccam.nodeid, srv->busycardid, &buf[4]);
					memcpy(dcw, &buf[4], 16);
					cc_decrypt(&srv->recvblock, buf+4, len-4); // additional crypto step				

					srv->busy = 0;
					srv->lastdcwtime = GetTickCount();

					pthread_mutex_lock(&prg.lockecm); //###

					ecm = getecmbyid(srv->busyecmid);
					if (!ecm) {
						debugf( getdbgflag(DBG_SERVER, 0, srv->id), " [!] error cw from server (%s:%d), ecm not found!!!\n",srv->host->name,srv->port);
						pthread_mutex_unlock(&prg.lockecm); //###
						srv->busy = 0;
						break;
					}
					// check for ECM???
					if (ecm->hash!=srv->busyecmhash) {
						debugf( getdbgflag(DBG_SERVER, 0, srv->id), " [!] error cw from server (%s:%d), ecm deleted!!!\n",srv->host->name,srv->port);
						pthread_mutex_unlock(&prg.lockecm); //###
						srv->busy = 0;
						break;
					}

					cs = getcsbyid(ecm->csid);
					card = cc_getcardbyid( srv, srv->busycardid );
					// Check for DCW
					if (!acceptDCW(dcw)) {
						srv->ecmerrdcw ++;
#ifdef SID_FILTER
						if (cs && card) {
							cardsids_update( card, ecm->provid, ecm->sid, -1);
							srv_cstatadd( srv, cs->id, 0 , 0);
						}
#endif
						ecm_setsrvflag(srv->busyecmid, srv->id, ECM_SRV_REPLY_FAIL);
						pthread_mutex_unlock(&prg.lockecm); //###
						break;
					}
//					else {

					srv->lastecmoktime = GetTickCount()-srv->lastecmtime;
					srv->ecmoktime += srv->lastecmoktime;
					srv->ecmok++;

					ecm_setsrvflagdcw(srv->busyecmid, srv->id, ECM_SRV_REPLY_GOOD,dcw);
#ifdef SID_FILTER
					if (cs && card) {
						cardsids_update( card, ecm->provid, ecm->sid, 1); /// + Card nodeID
						srv_cstatadd( srv, cs->id, 1 , srv->lastecmoktime);
					}
#endif
					if (card) {
						card->ecmoktime += GetTickCount()-srv->lastecmtime;
						card->ecmok++;
					}

					if (ecm->dcwstatus!=STAT_DCW_SUCCESS) {
						static char msg[] = "Good dcw from CCcam server";
						ecm->statusmsg = msg;
						ecm_setdcw( cs, ecm, dcw, DCW_SOURCE_SERVER, srv->id );
						debugf( getdbgflagpro(DBG_SERVER, 0, srv->id,cs->id), " <= cw from CCcam server (%s:%d) ch %04x:%06x:%04x (%dms)\n", srv->host->name,srv->port, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-srv->lastecmtime);
					}

					pthread_mutex_unlock(&prg.lockecm); //###
					break;

				case CC_MSG_ECM_NOK1: // EAGAIN, Retry
/*
					if (!srv->busy) break;
					ecm = srv->busyecm;
					debugf( getdbgflag(DBG_SERVER, 0, srv->id), " <| decode1 failed from CCcam server (%s:%d) ch %04x:%06x:%04x (%dms)\n", srv->host->name,srv->port, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-srv->lastecmtime);

					if ( (GetTickCount()-srv->lastecmtime)<CC_ECMRETRY_TIMEOUT ) {
						if (srv->retry<CC_ECMRETRY_MAX) {
							srv->busy = 0;
							if (cc_sendecm_srv(srv, ecm)) {
								srv->lastecmtime = GetTickCount();
								srv->busy = 1;
								srv->retry++;
								break;
							}
						}
					}
					if (srv->retry>=CC_ECMRETRY_MAX) {
						ecm_setsrvflag(ecm, srv->id, ECM_SRV_EXCLUDE); 
					}
					srv->busy = 0;
					break;
*/
				case CC_MSG_ECM_NOK2: // ecm decode failed
					if (!srv->busy) {
						debugf( getdbgflag(DBG_SERVER, 0, srv->id), " [!] dcw error from server (%s:%d), unknown ecm request\n",srv->host->name,srv->port);
						break;
					}

					pthread_mutex_lock(&prg.lockecm); //###

					ecm = getecmbyid(srv->busyecmid);
					if (!ecm) {
						debugf( getdbgflag(DBG_SERVER, 0, srv->id), " [!] dcw error from server (%s:%d), ecm not found!!!\n",srv->host->name,srv->port);
						pthread_mutex_unlock(&prg.lockecm); //###
						srv->busy = 0;
						break;
					}
					// check for ECM???
					if (ecm->hash!=srv->busyecmhash) {
						debugf( getdbgflag(DBG_SERVER, 0, srv->id), " [!] dcw error from server (%s:%d), ecm deleted!!!\n",srv->host->name,srv->port);
						pthread_mutex_unlock(&prg.lockecm); //###
						srv->busy = 0;
						break;
					}

					debugf( getdbgflagpro(DBG_SERVER, 0, srv->id,ecm->csid), " <| decode failed from CCcam server (%s:%d) ch %04x:%06x:%04x (%dms)\n", srv->host->name,srv->port, ecm->caid,ecm->provid,ecm->sid, GetTickCount()-srv->lastecmtime);
					cs = getcsbyid(ecm->csid);

					if (ecm->dcwstatus!=STAT_DCW_SUCCESS) {
						if ( cs && (GetTickCount()-ecm->recvtime)<cs->option.server.timeout ) {
							if (srv->retry<cs->option.retry.cccam) {
								srv->busy = 0;
								if (cc_sendecm_srv(srv, ecm)) {
									srv->lastecmtime = GetTickCount();
									srv->busy = 1;
									srv->retry++;
									debugf( getdbgflagpro(DBG_SERVER, 0, srv->id,ecm->csid), " (RE%d) -> ecm to CCcam server (%s:%d) ch %04x:%06x:%04x\n",srv->retry,srv->host->name,srv->port,ecm->caid,ecm->provid,ecm->sid);
									pthread_mutex_unlock(&prg.lockecm); //###
									break;
								}
							}
						}
					}

#ifdef SID_FILTER
					if (cs) {
						card = cc_getcardbyid( srv, srv->busycardid );
						if (card) cardsids_update(card, ecm->provid, ecm->sid, -1);
						srv_cstatadd( srv, cs->id, 0 , 0);
					}
#endif
					ecm_setsrvflag(srv->busyecmid, srv->id, ECM_SRV_REPLY_FAIL);

					srv->busy = 0;
					pthread_mutex_unlock(&prg.lockecm); //###
					break;


				case CC_MSG_BAD_ECM: // Add Card
					cc_msg_send( srv->handle, &srv->sendblock, CC_MSG_BAD_ECM, 0, NULL);
					//debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: cmd 0x05 from Server (%s:%d)\n",srv->host->name,srv->port);
					//currentecm.state = ECM_STATUS_FAILED;
					break;

				case CC_MSG_KEEPALIVE:
					srv->keepalivesent = 0;
					//debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: Keepalive ACK from Server (%s:%d)\n",srv->host->name,srv->port);
					break;

				case CC_MSG_CARD_DEL: // Delete Card
					pthread_mutex_lock(&prg.lockecm); //###
					card = srv->card;
					uint32 k = buf[4]<<24 | buf[5]<<16 | buf[6]<<8 | buf[7];
					struct cs_card_data *prevcard = NULL;
					while (card) {
						if (card->shareid==k) {
							debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: server (%s:%d), remove share-id %d\n",srv->host->name,srv->port,k);
							if (prevcard) prevcard->next = card->next; else srv->card = card->next;
							//Free SIDs
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
							// check for current ecm
							if (srv->busy && (srv->busycardid==k) ) ecm_setsrvflag(srv->busyecmid, srv->id, ECM_SRV_EXCLUDE);
							break;
						}
						prevcard = card;
						card = card->next;
					}
					pthread_mutex_unlock(&prg.lockecm); //###
			  		break;

				case CC_MSG_CARD_ADD:
					// remove own cards -> same nodeid "cfg.cccam.nodeid"
					if ( (buf[24]<=16) && memcmp(buf+26+buf[24]*7,cfg.cccam.nodeid,8) ) { // check Only the first 4 bytes
						// nodeid index = 26 + 7 * buf[24]
						struct cs_card_data tcard;
						memset(&tcard, 0, sizeof(struct cs_card_data) );
						tcard.shareid = buf[4]<<24 | buf[5]<<16 | buf[6]<<8 | buf[7];
						tcard.uphops = buf[14]+1;
						memcpy( tcard.nodeid, buf+26+buf[24]*7, 8);
						tcard.caid = (buf[12]<<8)+(buf[13]);
						tcard.nbprov = buf[24];
						i = 26+buf[24]*7;
						if (tcard.nbprov>CARD_MAXPROV) tcard.nbprov = CARD_MAXPROV;
						int nbprov = 0; // Accepted Providers
						for (i=0;i<tcard.nbprov; i++) {
							uint32_t provid = (buf[25+i*7]<<16) | (buf[26+i*7]<<8) | (buf[27+i*7]);
							if ( tcard.uphops <= srv_sharelimits( srv, tcard.caid, provid) ) {
								tcard.prov[nbprov] = provid;
								nbprov++;
								//debugf( getdbgflag(DBG_SERVER, 0, srv->id), " Accepted %04x:%06x\n", tcard.caid, provid);
							}
							//else debugf( getdbgflag(DBG_SERVER, 0, srv->id), " Ignored %04x:%06x\n", tcard.caid, provid);
						}
						if (nbprov) {
							tcard.nbprov = nbprov;
							debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: new card (%s:%d) %02x%02x%02x%02x%02x%02x%02x%02x_%x uphops %d caid %04x providers %d\n",srv->host->name,srv->port, buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7],tcard.shareid ,tcard.uphops, tcard.caid, tcard.nbprov);
							card = malloc( sizeof(struct cs_card_data) );
							memcpy( card, &tcard, sizeof(struct cs_card_data) );
							pthread_mutex_lock(&prg.lockecm); //###
							card->next = srv->card;
							srv->card = card;
							pthread_mutex_unlock(&prg.lockecm); //###
						}
					}
					break;

				case CC_MSG_SRV_INFO:
					memcpy(srv->nodeid, buf+4, 8);
					memcpy(srv->version, buf+12, 31);
					for (i=12; i<53; i++) {
						if (!buf[i]) break;
						if ( (buf[i]<32)||(buf[i]>'z') ) {
							memset(srv->version, 0, 31);
							break;
						}
					}
					memcpy(srv->build, buf+44, 31);
					debugf( getdbgflag(DBG_SERVER, 0, srv->id), " CCcam: server (%s:%d), info: version %s build %s\n",srv->host->name,srv->port,buf+12, buf+44);
					break;

				//default: debugdump(buf,len," CCcam: unknown packet from server (%s:%d): ",srv->host->name,srv->port);
				} // switch
			}
			srv->keepalivetime = GetTickCount();
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

int cc_sendecm_srv(struct cs_server_data *srv, ECM_DATA *ecm)
{
	unsigned char buf[CC_MAXMSGSIZE];

	//if ( (srv->handle>0)&&(!srv->busy) ) {
	//	if (!cc_getcardbyid(srv, srv->busycardid)) return 0;
		//debugf( getdbgflag(DBG_SERVER, 0, srv->id), " -> ecm to CCcam server (%s:%d) ch %04x:%06x:%04x shareid %x\n", srv->host->name, srv->port,ecm->caid,ecm->provid,ecm->sid,srv->busycardid);
		buf[0] = ecm->caid>>8;
		buf[1] = ecm->caid&0xff;
		buf[2] = ecm->provid>>24;
		buf[3] = ecm->provid>>16;
		buf[4] = ecm->provid>>8;
		buf[5] = ecm->provid&0xff;
		// srv->busycardid is saved from srvtab_arrange()
		buf[6] = srv->busycardid>>24;
		buf[7] = srv->busycardid>>16;
		buf[8] = srv->busycardid>>8;
		buf[9] = srv->busycardid&0xff;
		buf[10] = ecm->sid>>8;
		buf[11] = ecm->sid&0xff;
		buf[12] = ecm->ecmlen;
		memcpy( &buf[13],&ecm->ecm[0], ecm->ecmlen);
		srv->lastecmtime = GetTickCount();
		srv->busy = 1;
		return cc_msg_send( srv->handle, &srv->sendblock, CC_MSG_ECM_REQUEST, 13+ecm->ecmlen, buf );
	//}
	//return 0;
}

