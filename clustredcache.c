
// Multics ID encryption

static unsigned char T1[]={
  0x2a,0xe1,0x0b,0x13,0x3e,0x6e,0x32,0x48,
  0xd3,0x31,0x08,0x8c,0x8f,0x95,0xbd,0xd0,
  0xe4,0x6d,0x50,0x81,0x20,0x30,0xbb,0x75,
  0xf5,0xd4,0x7c,0x87,0x2c,0x4e,0xe8,0xf4,
  0xbe,0x24,0x9e,0x4d,0x80,0x37,0xd2,0x5f,
  0xdb,0x04,0x7a,0x3f,0x14,0x72,0x67,0x2d,
  0xcd,0x15,0xa6,0x4c,0x2e,0x3b,0x0c,0x41,
  0x62,0xfa,0xee,0x83,0x1e,0xa2,0x01,0x0e,
  0x7f,0x59,0xc9,0xb9,0xc4,0x9d,0x9b,0x1b,
  0x9c,0xca,0xaf,0x3c,0x73,0x1a,0x65,0xb1,
  0x76,0x84,0x39,0x98,0xe9,0x53,0x94,0xba,
  0x1d,0x29,0xcf,0xb4,0x0d,0x05,0x7d,0xd1,
  0xd7,0x0a,0xa0,0x5c,0x91,0x71,0x92,0x88,
  0xab,0x93,0x11,0x8a,0xd6,0x5a,0x77,0xb5,
  0xc3,0x19,0xc1,0xc7,0x8e,0xf9,0xec,0x35,
  0x4b,0xcc,0xd9,0x4a,0x18,0x23,0x9f,0x52,
  0xdd,0xe3,0xad,0x7b,0x47,0x97,0x60,0x10,
  0x43,0xef,0x07,0xa5,0x49,0xc6,0xb3,0x55,
  0x28,0x51,0x5d,0x64,0x66,0xfc,0x44,0x42,
  0xbc,0x26,0x09,0x74,0x6f,0xf7,0x6b,0x4f,
  0x2f,0xf0,0xea,0xb8,0xae,0xf3,0x63,0x6a,
  0x56,0xb2,0x02,0xd8,0x34,0xa4,0x00,0xe6,
  0x58,0xeb,0xa3,0x82,0x85,0x45,0xe0,0x89,
  0x7e,0xfd,0xf2,0x3a,0x36,0x57,0xff,0x06,
  0x69,0x54,0x79,0x9a,0xb6,0x6c,0xdc,0x8b,
  0xa7,0x1f,0x90,0x03,0x17,0x1c,0xed,0xd5,
  0xaa,0x5e,0xfe,0xda,0x78,0xb0,0xbf,0x12,
  0xa8,0x22,0x21,0x3d,0xc2,0xc0,0xb7,0xa9,
  0xe7,0x33,0xfb,0xf1,0x70,0xe5,0x17,0x96,
  0xf8,0x8d,0x46,0xa1,0x86,0xe2,0x40,0x38,
  0xf6,0x68,0x25,0x16,0xac,0x61,0x27,0xcb,
  0x5b,0xc8,0x2b,0x0f,0x99,0xde,0xce,0xc5
};

static unsigned char T2[]={
  0xbf,0x11,0x6d,0xfa,0x26,0x7f,0xf3,0xc8,
  0x9e,0xdd,0x3f,0x16,0x97,0xbd,0x08,0x80,
  0x51,0x42,0x93,0x49,0x5b,0x64,0x9b,0x25,
  0xf5,0x0f,0x24,0x34,0x44,0xb8,0xee,0x2e,
  0xda,0x8f,0x31,0xcc,0xc0,0x5e,0x8a,0x61,
  0xa1,0x63,0xc7,0xb2,0x58,0x09,0x4d,0x46,
  0x81,0x82,0x68,0x4b,0xf6,0xbc,0x9d,0x03,
  0xac,0x91,0xe8,0x3d,0x94,0x37,0xa0,0xbb,
  0xce,0xeb,0x98,0xd8,0x38,0x56,0xe9,0x6b,
  0x28,0xfd,0x84,0xc6,0xcd,0x5f,0x6e,0xb6,
  0x32,0xf7,0x0e,0xf1,0xf8,0x54,0xc1,0x53,
  0xf0,0xa7,0x95,0x7b,0x19,0x21,0x23,0x7d,
  0xe1,0xa9,0x75,0x3e,0xd6,0xed,0x8e,0x6f,
  0xdb,0xb7,0x07,0x41,0x05,0x77,0xb4,0x2d,
  0x45,0xdf,0x29,0x22,0x43,0x89,0x83,0xfc,
  0xd5,0xa4,0x88,0xd1,0xf4,0x55,0x4f,0x78,
  0x62,0x1e,0x1d,0xb9,0xe0,0x2f,0x01,0x13,
  0x15,0xe6,0x17,0x6a,0x8d,0x0c,0x96,0x7e,
  0x86,0x27,0xa6,0x0d,0xb5,0x73,0x71,0xaa,
  0x36,0xd0,0x06,0x66,0xdc,0xb1,0x2a,0x5a,
  0x72,0xbe,0x3a,0xc5,0x40,0x65,0x1b,0x02,
  0x10,0x9f,0x3b,0xf9,0x2b,0x18,0x5c,0xd7,
  0x12,0x47,0xef,0x1a,0x87,0xd2,0xc2,0x8b,
  0x99,0x9c,0xd3,0x57,0xe4,0x76,0x67,0xca,
  0x3c,0xfb,0x90,0x20,0x14,0x48,0xc9,0x60,
  0xb0,0x70,0x4e,0xa2,0xad,0x35,0xea,0xc4,
  0x74,0xcb,0x39,0xde,0xe7,0xd4,0xa3,0xa5,
  0x04,0x92,0x8c,0xd9,0x7c,0x1c,0x7a,0xa8,
  0x52,0x79,0xf2,0x33,0xba,0x1f,0x30,0x9a,
  0x00,0x50,0x4c,0xff,0xe5,0xcf,0x59,0xc3,
  0xe3,0x0a,0x85,0xb3,0xae,0xec,0x0b,0xfe,
  0xe2,0xab,0x4a,0xaf,0x69,0x6c,0x2c,0x5d
};

#define SN(b) (((b&0xf0)>>4)+((b&0xf)<<4))

static void fase(unsigned char *k,unsigned char *D)
{
	unsigned char l,dt; // paso 1 

	for(l=0;l<4;++l) D[l]^=k[l];  // paso 2 

	for(l=0;l<4;++l) D[l]=T1[D[l]];

	for(l=6;l>3;--l) { 
		D[(l+2)&3]^=D[(l+1)&3]; 
		dt=(SN(D[(l+1)&3])+D[l&3])&0xff; 
		D[l&3]=T2[dt];
	} 
	for(l=3;l>0;--l) {
		D[(l+2)&3]^=D[(l+1)&3]; 
		D[l&3]=T1[(SN(D[(l+1)&3])+D[l&3])&0xff]; 
	} 
	D[2]^=D[1]; 
	D[1]^=D[0]; 
}


// Packet Encryption

void encryptcache(uint8_t *buf, int len)
{
	int i;
	for (i=1; i<len; i++) buf[i] = (buf[i]+i) & 0xff;
}

void decryptcache(uint8_t *buf, int len)
{
	int i;
	for (i=1; i<len; i++) buf[i] = (0xff00+buf[i]-i) & 0xff;
}

///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

#define CACHE_STAT_WAIT     0
#define CACHE_STAT_DCW      1

#define CACHE_NONE_SENT     0
#define CACHE_REQUEST_SENT  1
#define CACHE_REPLY_SENT    2

struct cache_data {
	unsigned char status; // 0:Wait; 1: dcw received
	unsigned int recvtime;
	unsigned char tag;
	unsigned short sid;
	unsigned short onid;
	unsigned short caid;
	unsigned int hash;
	unsigned int prov;
	unsigned char cw[16];
	int peerid;
	//
	int sendpipe; // flag send dcw to ecmpipe
	int sendcache; // local cache send status
};


#define MAX_CACHETAB 9999


struct cache_data cachetab[MAX_CACHETAB];
int icachetab=0;

int prevcachetab( int index )
{
	if (index<1) index = MAX_CACHETAB-1; else index--; 
	return index;
}

struct cache_data *cache_new( struct cache_data *newdata )
{
	int i = icachetab;
	memset( &cachetab[i], 0, sizeof(struct cache_data) );
	cachetab[i].status = CACHE_STAT_WAIT; // 0:Wait; 1: dcw received
	cachetab[i].recvtime = GetTickCount();
	cachetab[i].tag = newdata->tag;
	cachetab[i].sid = newdata->sid;
	cachetab[i].onid = newdata->onid;
	cachetab[i].caid = newdata->caid;
	cachetab[i].hash = newdata->hash;
	cachetab[i].prov = newdata->prov;
	icachetab++;
	if (icachetab>=MAX_CACHETAB) icachetab=0;
	return &cachetab[i];
}

struct cache_data *cache_fetch( struct cache_data *thereq )
{
	int i = icachetab;
	uint32 ticks = GetTickCount();
	do {
		if (i<1) i = MAX_CACHETAB-1; else i--; 
		if ( (cachetab[i].recvtime+25000)<ticks ) return NULL;
		if ( (cachetab[i].hash==thereq->hash)&&(cachetab[i].caid==thereq->caid)&&(cachetab[i].tag==thereq->tag)&&(cachetab[i].sid==thereq->sid) )
			//if ( (cachetab[i].onid==0)||(cachetab[i].onid==thereq->onid) )
			return &cachetab[i];
	} while (i!=icachetab);
	return NULL;
}

int cache_check( struct cache_data *req )
{
	if ( ((req->tag&0xFE)!=0x80)||!req->caid||!req->hash ) return 0;
	//if (!cfg.cache.faccept0onid && !req->onid ) return 0;
	return 1;
}


int cache_check_request( unsigned char tag, unsigned short sid, unsigned short onid, unsigned short caid, unsigned int hash )
{
	if ( ((tag&0xFE)!=0x80)||!caid||!hash ) return 0;
	//if (!cfg.cache.faccept0onid && !onid ) return 0;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// pipe --> cache
///////////////////////////////////////////////////////////////////////////////

int pipe_send_cache_find( ECM_DATA *ecm, struct cardserver_data *cs)
{
	if ( !cache_check_request(ecm->ecm[0], ecm->sid, cs->option.onid, ecm->caid, ecm->hash) ) return 0;
	//send pipe to cache
	uchar buf[32];
	buf[0] = PIPE_CACHE_FIND;
	buf[2] = ecm->ecm[0];
	buf[3] = (ecm->sid)>>8; buf[4] = (ecm->sid)&0xff;
	buf[5] = (cs->option.onid)>>8; buf[6] = (cs->option.onid)&0xff;
	buf[7] = (ecm->caid)>>8; buf[8] = (ecm->caid)&0xff;
	buf[9] = ecm->hash>>24; buf[10] = ecm->hash>>16; buf[11] = ecm->hash>>8; buf[12] = ecm->hash & 0xff;
	buf[13] = ecm->provid>>16; buf[14] = ecm->provid>>8; buf[15] = ecm->provid & 0xff;
	pipe_send( srvsocks[0], buf, 16);
	return 1;
}

int pipe_send_cache_request( ECM_DATA *ecm, struct cardserver_data *cs)
{
	if ( !cache_check_request(ecm->ecm[0], ecm->sid, cs->option.onid, ecm->caid, ecm->hash) ) return 0;
	//send pipe to cache
	uchar buf[32];
	buf[0] = PIPE_CACHE_REQUEST;

	buf[2] = ecm->ecm[0];
	buf[3] = (ecm->sid)>>8;
	buf[4] = (ecm->sid)&0xff;
	buf[5] = (cs->option.onid)>>8;
	buf[6] = (cs->option.onid)&0xff;
	buf[7] = (ecm->caid)>>8;
	buf[8] = (ecm->caid)&0xff;
	buf[9] = ecm->hash>>24;
	buf[10] = ecm->hash>>16;
	buf[11] = ecm->hash>>8;
	buf[12] = ecm->hash & 0xff;
	buf[13] = ecm->provid>>16; buf[14] = ecm->provid>>8; buf[15] = ecm->provid & 0xff;
	pipe_send( srvsocks[0], buf, 16);
	return 1;
}

int pipe_send_cache_reply( ECM_DATA *ecm, struct cardserver_data *cs)
{
	if ( !cache_check_request(ecm->ecm[0], ecm->sid, cs->option.onid, ecm->caid, ecm->hash) ) return 0;
	uchar buf[32];
	buf[0] = PIPE_CACHE_REPLY;

	buf[2] = ecm->ecm[0];
	buf[3] = (ecm->sid)>>8;
	buf[4] = (ecm->sid)&0xff;
	buf[5] = (cs->option.onid)>>8;
	buf[6] = (cs->option.onid)&0xff;
	buf[7] = (ecm->caid)>>8;
	buf[8] = (ecm->caid)&0xff;
	buf[9] = ecm->hash>>24;
	buf[10] = ecm->hash>>16;
	buf[11] = ecm->hash>>8;
	buf[12] = ecm->hash & 0xff;
	buf[13] = ecm->provid>>16; buf[14] = ecm->provid>>8; buf[15] = ecm->provid & 0xff;

	if (ecm->dcwstatus==STAT_DCW_SUCCESS) {
		memcpy(buf+16, ecm->cw, 16);
		pipe_send( srvsocks[0], buf, 32);
	}
	else {
		pipe_send( srvsocks[0], buf, 16);
	}
	return 1;
}



int pipe_send_cache( uint8_t type, ECM_DATA *ecm, struct cardserver_data *cs)
{
	if ( !cache_check_request(ecm->ecm[0], ecm->sid, cs->option.onid, ecm->caid, ecm->hash) ) return 0;
	uchar buf[32];
	buf[0] = type;
	buf[2] = ecm->ecm[0];
	buf[3] = (ecm->sid)>>8;
	buf[4] = (ecm->sid)&0xff;
	buf[5] = (cs->option.onid)>>8;
	buf[6] = (cs->option.onid)&0xff;
	buf[7] = (ecm->caid)>>8;
	buf[8] = (ecm->caid)&0xff;
	buf[9] = ecm->hash>>24;
	buf[10] = ecm->hash>>16;
	buf[11] = ecm->hash>>8;
	buf[12] = ecm->hash & 0xff;
	buf[13] = ecm->provid>>16; buf[14] = ecm->provid>>8; buf[15] = ecm->provid & 0xff;
	if (ecm->dcwstatus==STAT_DCW_SUCCESS) {
		memcpy(buf+16, ecm->cw, 16);
		pipe_send( srvsocks[0], buf, 32);
	}
	else {
		pipe_send( srvsocks[0], buf, 16);
	}
	return 1;
}


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

SOCKET outsock;

void sendtoip( uint32_t ip, int port, unsigned char *buf, int len)
{
	if (ip && port) {
		struct sockaddr_in si_other;
		int slen=sizeof(si_other);
		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons( port );
		si_other.sin_addr.s_addr = ip;
		sendto(outsock, buf, len, 0, (struct sockaddr *)&si_other, slen);
	}
}

///////////////////////////////////////////////////////////////////////////////

void sendtopeer( struct cachepeer_data *peer, unsigned char *buf, int len)
{

	if (peer->host->ip && peer->port) {
		struct sockaddr_in si_other;
		int slen=sizeof(si_other);
		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons( peer->port );
		si_other.sin_addr.s_addr = peer->host->ip;
#ifdef DEBUG_NETWORK
		if (flag_debugnet) {
			debugf(getdbgflag(DBG_CACHE,0,0)," cache: send data (%d) to peer (%s:%d)\n", len, peer->host->name,peer->port);
			debughex(buf,len);
		}
#endif
		while (1) {
			struct pollfd pfd;
			pfd.fd = peer->outsock;
			pfd.events = POLLOUT;
			int retval = poll(&pfd, 1, 10);
			if (retval>0) {
				if ( pfd.revents & (POLLOUT) ) {
					sendto(peer->outsock, buf, len, 0, (struct sockaddr *)&si_other, slen);
				} else debugf(getdbgflag(DBG_CACHE,0,0)," cache: error sending data\n");
				break;
			}
			else if (retval<0) {
				if ( (errno!=EINTR)&&(errno!=EAGAIN) ) {
					debugf(getdbgflag(DBG_CACHE,0,0)," cache: error sending data\n");
					break;
				}
			}
			else debugf(getdbgflag(DBG_CACHE,0,0)," cache: error sending data\n");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define TYPE_REQUEST   1
#define TYPE_REPLY     2
#define TYPE_PINGREQ   3
#define TYPE_PINGRPL   4
#define TYPE_RESENDREQ 5

#ifdef NEWCACHE

#define TYPE_HELLO          0x03
#define TYPE_HELLO_ACK      0x10
#define TYPE_KEEPALIVE      0x11
#define TYPE_KEEPALIVE_ACK  0x12
#define TYPE_CARD_LIST      0x13
#define TYPE_SMS            0x14
#define TYPE_SMS_ACK        0x15

#define TYPE_VERSION        0x90
#define TYPE_VERSION_ACK    0x91

#define TYPE_CHECKREQ       0x92
#define TYPE_CHECKREQ_ACK   0x93

#define TYPE_UNKNOWN        0xFF

#endif



int peer_acceptcard( struct cachepeer_data *peer, uint16_t caid, uint32_t prov)
{
	if (!peer->cards[0]) return 1;
	int i;
	int caprov = (caid<<16) | prov; 
	for (i=0; i<1024; i++) {
		if (!peer->cards[0]) return 0;
		if (peer->cards[i] == caprov) return 1;
	}
	return 0;
}		


void cache_send_request(struct cache_data *pcache,struct cachepeer_data *peer)
{
	uchar buf[16];
	//01 80 00CD 0001 0500 8D1DB359
	buf[0] = TYPE_REQUEST;
	buf[1] = pcache->tag;
	buf[2] = pcache->sid>>8;
	buf[3] = pcache->sid;
	buf[4] = pcache->onid>>8;
	buf[5] = pcache->onid&0xff;
	buf[6] = pcache->caid>>8;
	buf[7] = pcache->caid;
	buf[8] = pcache->hash>>24;
	buf[9] = pcache->hash>>16;
	buf[10] = pcache->hash>>8;
	buf[11] = pcache->hash;
	if (peer) {
		sendtopeer(peer, buf, 12);
		peer->sentreq++;
	}
	else {
		peer = cfg.cache.peer;
		while (peer) {
			if (!IS_DISABLED(peer->flags))
			if (peer->host->ip && peer->port)
#ifdef DONT_CHECK_PEER_ADDR
			if ( peer->ping>0 )
#endif
			if ( !peer->fblock0onid || pcache->onid )
			if ( peer_acceptcard(peer, pcache->caid, pcache->prov) ) {
				sendtopeer(peer, buf, 12);
				peer->sentreq++;
			}
			peer = peer->next;
		}
	}
}


int cache_send_reply(struct cache_data *pcache,struct cachepeer_data *peer)
{
	if (pcache->status==CACHE_STAT_DCW) {
		uchar buf[64];
		//Common Data
		buf[0] = TYPE_REPLY;
		buf[1] = pcache->tag;
		buf[2] = pcache->sid>>8;
		buf[3] = pcache->sid;
		buf[4] = pcache->onid>>8;
		buf[5] = pcache->onid&0xff;
		buf[6] = pcache->caid>>8;
		buf[7] = pcache->caid;
		buf[8] = pcache->hash>>24;
		buf[9] = pcache->hash>>16;
		buf[10] = pcache->hash>>8;
		buf[11] = pcache->hash;
		buf[12] = pcache->tag;
		memcpy( buf+13, pcache->cw, 16);
		if (peer) {
			sendtopeer(peer, buf, 29);
			peer->sentrep++;
			return 1;
		}
		else {
			int count = 0;
			peer = cfg.cache.peer;
			while ( peer ) {
				if ( !IS_DISABLED(peer->flags) )
				if ( peer->host->ip && peer->port )
#ifdef DONT_CHECK_PEER_ADDR
				if ( peer->ping>0 )
#endif
				if ( !peer->fblock0onid || pcache->onid )
				if ( peer_acceptcard(peer, pcache->caid, pcache->prov) ) {
					sendtopeer(peer, buf, 29);
					peer->sentrep++;
					count++;
				}
				peer = peer->next;
			}
			return count;
		}
	}
	return 0;
}

void cache_send_resendreq(struct cache_data *pcache,struct cachepeer_data *peer)
{
	// <1:TYPE_RESENDREQ> <2:port> <1:ecmtag> <2:sid> <2:onid> <2:caid> <4:hash>
	uchar buf[64];
	buf[0] = TYPE_RESENDREQ;
	//Port
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = cfg.cache.port>>8;
	buf[4] = cfg.cache.port&0xff;

	buf[5] = pcache->tag;
	buf[6] = pcache->sid>>8;
	buf[7] = pcache->sid;
	buf[8] = pcache->onid>>8;
	buf[9] = pcache->onid&0xff;
	buf[10] = pcache->caid>>8;
	buf[11] = pcache->caid;
	buf[12] = pcache->hash>>24;
	buf[13] = pcache->hash>>16;
	buf[14] = pcache->hash>>8;
	buf[15] = pcache->hash;

	if (peer) sendtopeer(peer, buf, 16);
	else {
		peer = cfg.cache.peer;
		while ( peer ) {
			if ( !IS_DISABLED(peer->flags) )
			if ( peer->host->ip && peer->port )
			if ( peer->ping>0 )
			if ( peer_acceptcard(peer, pcache->caid, pcache->prov) )
				sendtopeer(peer, buf, 16);
			peer = peer->next;
		}
	}
}


void cache_send_ping(struct cachepeer_data *peer)
{
	unsigned char buf[32];
	buf[0] = TYPE_PINGREQ;
	// New Cache IDENT
	buf[1] = 'M';
	buf[2] = 'C';
	buf[3] = 1;
	// PEER ID
	buf[4] = peer->id>>8; 
	buf[5] = peer->id&0xff;
	// MULTICS CRC
	buf[6] = peer->crc[0] = 0xff & rand();
	buf[7] = peer->crc[1] = 0xff & rand();
	buf[8] = peer->crc[2] = 0xff & rand();
	//Port
	buf[9] = peer->crc[3] = 0;
	buf[10] = 0;
	buf[11] = cfg.cache.port>>8;
	buf[12] = cfg.cache.port&0xff;
	//Program
	buf[13] = 0x01; //ID
	buf[14] = 7; //LEN
	buf[15] = 'M'; buf[16] = 'u'; buf[17] = 'l'; buf[18] = 't'; buf[19] = 'i'; buf[20] = 'C'; buf[21] = 'S';
	//Version
	buf[22] = 0x02; //ID
	buf[23] = 3; //LEN
	buf[24] = 'r'; buf[25] = '0'+(REVISION/10); buf[26] = '0'+(REVISION%10);
	//
	sendtopeer( peer, buf, 27);
}


#ifdef NEWCACHE

void cache_send_keepalive(struct cachepeer_data *peer)
{
	if (peer->protocol&1) {
		unsigned char buf[32];
		buf[0] = TYPE_KEEPALIVE;
		buf[1] = peer->id>>8; 
		buf[2] = peer->id&0xff;
		sendtopeer( peer, buf, 3);
	}
	else cache_send_ping(peer);
}


struct sms_data *cache_new_sms(char *msg)
{
	struct sms_data *sms = malloc( sizeof(struct sms_data) );
	int len = strlen(msg);
	uint32_t hash = hashCode((uint8_t*)msg, len);
	strcpy( sms->msg, msg );
	sms->hash = hash;
	gettimeofday( &sms->tv, NULL );
	sms->next = NULL;
	return (sms);
}

void cache_send_sms(struct cachepeer_data *peer, struct sms_data *sms)
{
	int len = strlen(sms->msg);
	sms->status = 1; // bit 0 (0:in,1:out) bit 1 (0:unread/unack, 1:read/ack)
	sms->next = peer->sms;
	peer->sms = sms;
	// Send Buffer
	uint8_t buf[1024];
	buf[0] = TYPE_SMS;
	buf[1] = sms->hash>>24;
	buf[2] = sms->hash>>16;
	buf[3] = sms->hash>>8;
	buf[4] = sms->hash;
	memcpy( buf+5, sms->msg, len );
	sendtopeer(peer, buf, len+5);
	// Debug
	debugf(getdbgflag(DBG_CACHE,0,0)," SMS to peer (%s:%d)\n", peer->host->name, peer->port);
}


#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

#ifdef DONT_CHECK_PEER_ADDR
struct cachepeer_data *getpeerbyip(uint32_t ip)
{
	struct cachepeer_data *peer = cfg.cache.peer;
	while(peer) {
		if (peer->host->ip==ip) return peer;
		peer = peer->next;
	}
	return NULL;
}
#endif

struct cachepeer_data *getpeerbyaddr(uint32_t ip, uint16_t port)
{
	struct cachepeer_data *peer = cfg.cache.peer;
	while(peer) {
		if ( (peer->host->ip==ip)&&(peer->recvport==port) ) return peer;
		peer = peer->next;
	}
	return NULL;
}

struct cachepeer_data *getpeerbyid(int id)
{
	struct cachepeer_data *peer = cfg.cache.peer;
	while(peer) {
		if (peer->id==id) return peer;
		peer = peer->next;
	}
	return NULL;
}


void peer_check_messages( struct cachepeer_data *peer )
{
	// Remove Old Messages if there is too much
	struct sms_data *sms = peer->sms;
	int nb = 0;
	while (sms) {
		nb++;
		if (nb>=30) break;
		sms = sms->next;
	}
	// Remove Messages
	if (sms) {
		struct sms_data *next = sms->next;
		sms->next = NULL;
		while (next) {
			sms = next;
			next = sms->next;
			free(sms);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

void cache_recvmsg()
{
	unsigned int recv_ip;
	unsigned short recv_port;
	unsigned char buf[1024];
	char str[1024];
	struct sockaddr_in si_other;
	socklen_t slen=sizeof(si_other);
	struct cachepeer_data *peer;

	int received = recvfrom( cfg.cache.handle, buf, sizeof(buf), 0, (struct sockaddr*)&si_other, &slen);
	if (received>1020) return;
	memcpy( &recv_ip, &si_other.sin_addr, 4);
	recv_port = ntohs(si_other.sin_port);

	if (received>0) {
#ifdef DEBUG_NETWORK
		if (flag_debugnet) {
			debugf(getdbgflag(DBG_CACHE,0,0)," cache: recv data (%d) from address (%s:%d)\n", received, ip2string(recv_ip), recv_port );
			debughex(buf,received);
		}
#endif
		// Store Data
		struct cache_data req;

		switch(buf[0]) {

				// 03 00 00 01 2F EE B1 CB 54 00 00 D8 03 
				case TYPE_PINGREQ:
					// Check Peer
					peer = cfg.cache.peer;
					int port = (buf[11]<<8)|buf[12];
					struct cachepeer_data *peerip = NULL;
					while (peer) {
						if (peer->host->ip==recv_ip) {
							peerip = peer;
							if (peer->port==port) break; // Found
						}
						peer = peer->next;
					}
					if (!peer) {
						if (peerip) {
							// Check for peer reuse
							peer = cfg.cache.peer;
							while (peer) {
								if ( (peer->port==port) && !strcmp(peer->host->name,peerip->host->name) ) break;
								peer = peer->next;
							}
							if (!peer) {
								if (peerip->port==0) peerip->port = port;
								else {
									// add new peer
									peer = malloc( sizeof(struct cachepeer_data) );
									memset( peer, 0, sizeof(struct cachepeer_data) );
									peer->host = peerip->host;
									peer->port = port;
									peer->id = cfg.cachepeerid;
									peer->outsock = CreateClientSockUdp();
									peer->runtime = 1;
									peer->fblock0onid = peerip->fblock0onid;
									//peer->fsendrep = peerip->fsendrep;
									cfg.cachepeerid++;
									cfg_addcachepeer(&cfg, peer);
									debugf(getdbgflag(DBG_CACHE,0,0), " Cache: new peer (%s:%d)\n", peer->host->name, peer->port );
								}
							}
							else {
								peer->host->checkiptime = 0;
							}
						}
						else debugf(getdbgflag(DBG_CACHE,0,0), " Cache Alert: unknown peer (%s:%d)\n", ip2string(recv_ip), port );
					}
					//
					if (peer) {
						// Check Status
						if (IS_DISABLED(peer->flags)) break;
						// Set Defaults
						memset( peer->cards, 0, sizeof(peer->cards) );
						peer->protocol = 0; // Normal CSP Protocol
						strcpy(peer->program,"CSP");
						peer->version[0] = 0;
						// Check for extended reply ( Program Name/Version )
						int index = 13;
						while (received>index) {
							if ( (index+buf[index+1]+2)>received ) break;
							switch(buf[index]) {
								case 0x01:
									if (buf[index+1]<32) { memcpy(peer->program, buf+index+2, buf[index+1]); peer->program[buf[index+1]] = 0; }
									break;
								case 0x02:
									if (buf[index+1]<32) { memcpy(peer->version, buf+index+2, buf[index+1]); peer->version[buf[index+1]] = 0; }
									break;
							}
							index += 2+buf[index+1];
						}
						// Set Default Reply
						buf[0] = TYPE_PINGRPL;
						// Check for New Protocol
						if ( buf[1]=='M' && buf[2]=='C' ) {
							peer->protocol = buf[3];
							if (peer->protocol&1) {
								buf[0] = TYPE_HELLO_ACK;
								// Decode CRC
								buf[13] = cfg.cache.port>>8;
								buf[14] = cfg.cache.port;
								fase( buf+11, buf+6);
							}
						}
						sendtopeer( peer, buf, 9);
						// Check for activity
						if (peer->recvport!=recv_port) {
							peer->ping = 0;
							peer->recvport = recv_port;
							peer->ismultics = 0;
						}
					}
					break;


				case TYPE_PINGRPL:
					// Get Peer
					peer = cfg.cache.peer;
					int peerid = (buf[4]<<8) | buf[5];
					while (peer) {
						if ( (peer->host->ip==recv_ip)&&(peer->id==peerid) ) {
							peer->protocol = 0;
							peer->recvport = recv_port;
							peer->lastpingrecv = GetTickCount();
							if (peer->ping>0)
								peer->ping = (peer->ping+peer->lastpingrecv-peer->lastpingsent)/2;
							else {
								debugf(getdbgflag(DBG_CACHE,0,peer->id), " Cache: Peer (%s:%d) come Online\n", peer->host->name, peer->port );
								peer->ping = peer->lastpingrecv-peer->lastpingsent;
							}
							peer->ping++;
							break;
						}
						peer = peer->next;
					}
					break;

				case TYPE_REQUEST:
					// Check Peer
					peer = getpeerbyaddr(recv_ip,recv_port);
#ifdef DONT_CHECK_PEER_ADDR
					if (!peer) peer = getpeerbyip(recv_ip);
#endif
					if (!peer) break;
					// Check Status
					if (IS_DISABLED(peer->flags)) break;
					// Check Multics r50/r51
					if ( !strcmp("r50",peer->version)||!strcmp("r51",peer->version) ) break;
					// Get DATA
					req.tag = buf[1];
					req.sid = (buf[2]<<8) | buf[3];
					req.onid = (buf[4]<<8) | buf[5];
					req.caid = (buf[6]<<8) | buf[7];
					req.hash = (buf[8]<<24) | (buf[9]<<16) | (buf[10]<<8) |buf[11];
					// Check Cache Request
					if (!cache_check(&req)) break;
					//
					peer->reqnb++;
					// ADD CACHE
					struct cache_data *pcache = cache_fetch( &req );
					if (pcache==NULL) {
						pcache = cache_new( &req );
						// Send Tracker
						if (cfg.cache.tracker) {
							pcache->sendcache = CACHE_REQUEST_SENT;
							cfg.cache.req++;
							struct cachepeer_data *p = cfg.cache.peer;
							while (p) {
								if (p!=peer)
								if (!IS_DISABLED(p->flags))
								if (p->host->ip && p->port)
								if (p->ping>0)
								if ( !p->fblock0onid || pcache->onid )
									cache_send_request(pcache,p);
								p = p->next;
							}
						}
					}
					else
					 if ( !cfg.cache.tracker && (pcache->status==CACHE_STAT_DCW) && (pcache->sendcache==CACHE_NONE_SENT) ) {
						peer->ihitfwd++;
						peer->hitfwd++;
						cache_send_reply(pcache,peer);
					}
					break;

				case TYPE_REPLY:
					// Check Peer
					peer = getpeerbyaddr(recv_ip,recv_port);
#ifdef DONT_CHECK_PEER_ADDR
					if (!peer) peer = getpeerbyip(recv_ip);
#endif
					if (!peer) break;
					// Check Status
					if (IS_DISABLED(peer->flags)) break;
					// Check Multics r50/r51
					if ( !strcmp("r50",peer->version)||!strcmp("r51",peer->version) ) break;
					// Check Integrity
					if (buf[12]!=buf[1]) break;
					// SetUp Request
					req.tag = buf[1];
					req.sid = (buf[2]<<8) | buf[3];
					req.onid = (buf[4]<<8) | buf[5];
					req.caid = (buf[6]<<8) | buf[7];
					req.hash = (buf[8]<<24) | (buf[9]<<16) | (buf[10]<<8) |buf[11];
					// Check Cache Request
					if (!cache_check(&req)) break;
					//
					if (received==13) break;
					else if (received>=29) {
						if ( !acceptDCW(buf+13) ) break;
						peer->repok++; // Request+Reply
						// Search for Cache data
						struct cache_data *pcache = cache_fetch( &req );
						if (pcache==NULL) pcache = cache_new( &req );
						//
						if ( (pcache->status!=CACHE_STAT_DCW)
							|| ( (pcache->status==CACHE_STAT_DCW) && pcache->sendpipe && memcmp(pcache->cw, buf+13, 16) )
						) {
							//*debugf(" [CACHE] Update Cache DCW %04x:%04x:%08x\n", pcache->caid, pcache->sid, pcache->hash);
							pcache->peerid = peer->id;
							memcpy(pcache->cw, buf+13, 16);
							pcache->status = CACHE_STAT_DCW;
							// Send PIPE
							if (pcache->sendpipe) {
								uchar buf[32];
								buf[0] = PIPE_CACHE_FIND_SUCCESS;
								buf[1] = 11+2+16; // Data length
								buf[2] = pcache->tag;
								buf[3] = pcache->sid>>8; buf[4] = pcache->sid&0xff;
								buf[5] = pcache->onid>>8; buf[6] = pcache->onid&0xff;
								buf[7] = pcache->caid>>8; buf[8] = pcache->caid&0xff;
								buf[9] = pcache->hash>>24; buf[10] = pcache->hash>>16; buf[11] = pcache->hash>>8; buf[12] = pcache->hash & 0xff;
								buf[13] = peer->id>>8; buf[14] = peer->id&0xff;
								memcpy( buf+15, pcache->cw, 16);
								//*debugf(" pipe Cache->Ecm: PIPE_CACHE_FIND_SUCCESS %04x:%04x:%08x\n",pcache->caid, pcache->sid, pcache->hash); // debughex(buf, 13+16);
								pipe_send( srvsocks[1], buf, 13+2+16);
								//pcache->sendpipe = 0;
							}
							// Send Tracker
							if (cfg.cache.tracker) {
								pcache->sendcache = CACHE_REPLY_SENT;
								cfg.cache.rep++;
								struct cachepeer_data *p = cfg.cache.peer;
								while (p) {
									if (p!=peer)
									if (!IS_DISABLED(p->flags))
									if (p->host->ip && p->port)
#ifdef DONT_CHECK_PEER_ADDR
									if (p->ping>0)
#endif
									if ( !p->fblock0onid || pcache->onid )
										cache_send_reply(pcache,p);
									p = p->next;
								}
							}
							else {
								struct cachepeer_data *p = cfg.cache.peer;
								while (p) {
									if (p->fsendrep)
									if (p!=peer)
									if (!IS_DISABLED(p->flags))
									if (p->host->ip && p->port)
#ifdef DONT_CHECK_PEER_ADDR
									if (p->ping>0)
#endif
									if ( !p->fblock0onid || pcache->onid )
										cache_send_reply(pcache,p);
									p = p->next;
								}
							}
						}
					}
					break;

				case TYPE_RESENDREQ:
					// Check Peer
					peer = getpeerbyaddr(recv_ip,recv_port);
					if (!peer) break;
					// Check Status
					if (IS_DISABLED(peer->flags)) break;
					// Check Multics r50/r51
					if ( !strcmp("r50",peer->version)||!strcmp("r51",peer->version) ) break;
					//
					if (received>=16) { // Good Packet
						struct cache_data req;
						req.tag = buf[5];
						req.sid = (buf[6]<<8) | buf[7];
						req.onid = (buf[8]<<8) | buf[9];
						req.caid = (buf[10]<<8) | buf[11];
						req.hash = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
						// Check Cache Request
						if (!cache_check(&req)) break;
						struct cache_data *pcache = cache_fetch( &req );
						if ( (pcache!=NULL)&&(pcache->status==CACHE_STAT_DCW) ) {
							buf[4] = TYPE_REPLY;
							buf[16] = buf[3];
							memcpy( buf+17, pcache->cw, 16);
							sendtopeer( peer, buf+4, 29);
						}
					}
					break;


#ifdef NEWCACHE
				case TYPE_HELLO_ACK:
					// Get Peer
					peerid = (buf[4]<<8) | buf[5];
					peer = cfg.cache.peer;
					while (peer) {
						if ( (peer->host->ip==recv_ip)&&(peer->id==peerid) ) {
							// Check for new cache
							uint8_t k[4]; k[0] = cfg.cache.port>>8; k[1] = cfg.cache.port; k[2]=peer->port>>8; k[3]=peer->port; 
							fase( k, peer->crc);
							if ( (peer->crc[0]==buf[6])&&(peer->crc[1]==buf[7])&&(peer->crc[2]==buf[8]) ) peer->ismultics =1; else peer->ismultics = 0;
							peer->recvport = recv_port;
							peer->lastpingrecv = GetTickCount();
							if (peer->ping>0)
								peer->ping = (peer->ping+peer->lastpingrecv-peer->lastpingsent)/2;
							else {
								peer->ping = peer->lastpingrecv-peer->lastpingsent;
								debugf(getdbgflag(DBG_CACHE,0,peer->id), " Cache: Peer (%s:%d) come Online*\n", peer->host->name, peer->port );
							}
							peer->ping++;
							//debugf(getdbgflag(DBG_CACHE,0,peer->id), " Cache: sending card data to peer (%s:%d)\n", peer->host->name, peer->port );
							// Send CARDS DATA
							buf[0] = TYPE_CARD_LIST;
							buf[1] = 1; // Reset Cards
							int pos = 2;
							struct cardserver_data *cs = cfg.cardserver;
							while (cs) {
								int i;
								if (cs->option.fallowcache)
								for (i=0; i<cs->card.nbprov; i++) {
									uint32_t caprov = (cs->card.caid<<16)|(cs->card.prov[i]);
 									buf[pos] = caprov>>24;
 									buf[pos+1] = caprov>>16;
 									buf[pos+2] = caprov>>8;
 									buf[pos+3] = caprov;
									pos +=4;
									if (pos>400) {
										sendtopeer( peer, buf, pos);
										buf[0] = TYPE_CARD_LIST;
										buf[1] = 0; // no Reset
										pos = 2;
									}
								}
								cs = cs->next;
							}
							if (pos>2) {
								sendtopeer( peer, buf, pos);
							}
							break;
						}
						peer = peer->next;
					}
					break;

				case TYPE_CARD_LIST:
					// Check Peer
					peer = getpeerbyaddr(recv_ip,recv_port);
					if (!peer) break;
					// reset cards
					int i = 0;
					if (buf[1]&1) memset( peer->cards, 0, sizeof(peer->cards) ); 
					else for (i=0; i<1024; i++) if (!peer->cards[i]) break;
					//
					int totalcards = (received-2)/4;
					int j=0;
					while ( (j<totalcards)&&(i<1024) ){
						peer->cards[i] = (buf[2+j*4]<<24)|(buf[3+j*4]<<16)|(buf[4+j*4]<<8)|(buf[5+j*4]);
						j++; i++; 
					}
					break;

				case TYPE_KEEPALIVE:
					// Check Peer
					peer = getpeerbyaddr(recv_ip,recv_port);
					if (!peer) break;
					// Check Status
					if (IS_DISABLED(peer->flags)) break;
					// Send Reply
					buf[0] = TYPE_KEEPALIVE_ACK;
					sendtopeer( peer, buf, received);
					break;


				case TYPE_KEEPALIVE_ACK:
					// Check Peer
					peer = getpeerbyaddr(recv_ip,recv_port);
					if (!peer) break;
					// Check Status
					if (IS_DISABLED(peer->flags)) break;
					//
					peer->lastpingrecv = GetTickCount();
					if (peer->ping>0)
						peer->ping = (peer->ping+peer->lastpingrecv-peer->lastpingsent)/2;
					else
						peer->ping = peer->lastpingrecv-peer->lastpingsent;
					peer->ping++;
					break;

				case TYPE_CHECKREQ:
					//
					if (received!=20) break;
					decryptcache( buf, received );
					if (buf[19]!=TYPE_CHECKREQ) break;
					struct cache_data req;
					req.tag = buf[1];
					req.sid = (buf[2]<<8) | buf[3];
					req.onid = (buf[4]<<8) | buf[5];
					req.caid = (buf[6]<<8) | buf[7];
					req.hash = (buf[8]<<24) | (buf[9]<<16) | (buf[10]<<8) | buf[11];
					uint32_t ip = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
					port =  (buf[16]<<8) | buf[17];
					int uphops = buf[18];
					// Get Cache Request
					if (!cache_check(&req)) break;
					pcache = cache_fetch( &req );
					if ( (pcache!=NULL)&&(pcache->status==CACHE_STAT_DCW) ) {
						buf[0] = TYPE_CHECKREQ_ACK;
						buf[12] = pcache->tag;
						memcpy( buf+13, pcache->cw, 16);
						buf[29] = TYPE_CHECKREQ_ACK;
						encryptcache( buf, 30 );
						sendtoip( ip, port, buf, 30 );
					}
					else if ( uphops && port && ip ) {
						buf[18]--;
						// Send to MultiCS PEERS
						encryptcache( buf, 20 );
						struct cachepeer_data *p = cfg.cache.peer;
						while (p) {
							if (!IS_DISABLED(p->flags))
							if (p->host->ip && p->port)
							if ( p->ping>0 )
							//if ( peer_acceptcard(p, req.caid, req.prov) )
							if ( p->protocol&1 )
							if ( p->ismultics )
								sendtopeer(p, buf, 20);
							p = p->next;
						}
					}
					break;


				case TYPE_SMS:
					peer = getpeerbyaddr(recv_ip,recv_port);
					if (!peer) break;
					if (received<6) break;
					uint32_t hash = (buf[1]<<24) | (buf[2]<<16) | (buf[3]<<8) | (buf[4]);
					buf[received] = 0;
					//
					peer_check_messages( peer );
					// Create data
					struct sms_data *sms = malloc( sizeof(struct sms_data) );
					strcpy( sms->msg, (char*)buf+5);
					sms->hash = hash;
					sms->status = 0; // bit 0 (0:in,1:out) bit 1 (0:unread/unack, 1:read/ack)
					gettimeofday( &sms->tv, NULL );
					sms->next = peer->sms;
					peer->sms = sms;

					// SEND ACK
					buf[0] = TYPE_SMS_ACK;
					sendtopeer( peer, buf, 5 );
					// debug
					debugf(getdbgflag(DBG_CACHE,0,0)," Cache: SMS from peer (%s:%d)\n", peer->host->name, peer->port);
					break;

				case TYPE_SMS_ACK:
					peer = getpeerbyaddr(recv_ip,recv_port);
					if (!peer) break;
					if (received!=5) break;
					if (!peer->sms) break;
					hash = (buf[1]<<24) | (buf[2]<<16) | (buf[3]<<8) | (buf[4]);
					// Search for data
					sms = peer->sms;
					while (sms) {
						if ( (sms->hash==hash)&&(sms->status==1) ) {
							debugf(getdbgflag(DBG_CACHE,0,0)," Cache: SMS ACK from peer (%s:%d)\n", peer->host->name, peer->port);
							sms->status = 3;
						}
						sms = sms->next;
					}
					break;
#endif

				case TYPE_VERSION:
					peer = getpeerbyaddr(recv_ip,recv_port);
					if ( peer && (peer->protocol&1) ) {
						buf[0] = TYPE_UNKNOWN;
						buf[1] = TYPE_VERSION;
						sendtopeer( peer, buf, 2 );
					}
					break;

				case TYPE_UNKNOWN:
					break;

				default:
					if (received>100) array2hex( buf, str, 100); else array2hex( buf, str, received);
					debugf(getdbgflag(DBG_CACHE,0,0)," Cache: Unknown message from (%s) : %s\n", ip2string(recv_ip), received, str );
#ifdef NEWCACHE
					peer = getpeerbyaddr(recv_ip,recv_port);
					if ( peer && (peer->protocol&1) ) {
						buf[1] = buf[0];
						buf[0] = TYPE_UNKNOWN;
						sendtopeer( peer, buf, 2 );
					}
#endif
					break;
			}
	}
}

  
void cache_pipe_recvmsg()
{
	uchar buf[300];
	struct cache_data req;
	struct cache_data *pcache;

	int len =  pipe_recv( srvsocks[1], buf );
	if (len>0) {
		//debugf(" Recv from Cache Pipe\n"); debughex(buf,len);
		switch(buf[0]) {
			case PIPE_LOCK:
				pthread_mutex_lock(&prg.lockmain);
				pthread_mutex_unlock(&prg.lockmain);
				break;
			case PIPE_CACHE_FIND:
				//Check for cachepeer
				req.tag = buf[2];
				req.sid = (buf[3]<<8) | buf[4];
				req.onid = (buf[5]<<8) | buf[6];
				req.caid = (buf[7]<<8) | buf[8];
				req.hash = (buf[9]<<24) | (buf[10]<<16) | (buf[11]<<8) | (buf[12]);
				req.prov = (buf[13]<<16) | (buf[14]<<8) | (buf[15]);
				// Check Cache Request
				if (!cache_check(&req)) {
					// Send find failed
					buf[0] = PIPE_CACHE_FIND_FAILED;
					buf[1] = 11;
					pipe_send( srvsocks[1], buf, 13);
					break;
				}
				pcache = cache_fetch( &req );
				if (pcache==NULL) {
					pcache = cache_new( &req );
					pcache->sendpipe = 1;
					// Send find failed
					buf[0] = PIPE_CACHE_FIND_FAILED;
					buf[1] = 11;
					//*debugf(" Pipe Cache->Ecm: PIPE_CACHE_FIND_FAILED\n"); debughex(buf,len);
					pipe_send( srvsocks[1], buf, 13);
				}
				else {
					pcache->prov = req.prov;
					//if (!pcache->sid) pcache->sid = req.sid;
					//if (!pcache->caid) pcache->caid = req.caid;
					if (pcache->status==CACHE_STAT_DCW) {
						struct cachepeer_data *peer = getpeerbyid(pcache->peerid);
						if (peer) {
							peer->lastcaid = pcache->caid;
							peer->lastprov = pcache->prov;
							peer->lastsid = pcache->sid;
							peer->lastdecodetime = 0; //ticks - ecm->recvtime;
							buf[13] = peer->id>>8; buf[14] = peer->id & 0xff;
							memcpy( buf+15, pcache->cw, 16);
							buf[0] = PIPE_CACHE_FIND_SUCCESS;
							buf[1] = 11+2+16;
							//*debugf(" Pipe Cache->Ecm: PIPE_CACHE_FIND_SUCCESS\n"); debughex(buf,len);
							pipe_send( srvsocks[1], buf, 13+2+16);
							pcache->sendpipe = 0;
						}// else buf[13]=0; buf[14]=0;
					}
					else if (pcache->status==CACHE_STAT_WAIT) {
						pcache->sendpipe = 1;
						buf[0] = PIPE_CACHE_FIND_WAIT;
						buf[1] = 11;
						//debugf(" Pipe Cache->Ecm: PIPE_CACHE_FIND_WAIT\n");
						pipe_send( srvsocks[1], buf, 13);
					}
					else  {
						pcache->sendpipe = 1;
						// Send find failed
						buf[0] = PIPE_CACHE_FIND_FAILED;
						buf[1] = 11;
						//*debugf(" Pipe Cache->Ecm: PIPE_CACHE_FIND_FAILED\n"); debughex(buf,len);
						pipe_send( srvsocks[1], buf, 13);
					}
				}
				//debugf(" Cache Req Sendpipe = %d\n", pcache->sendpipe);
				break;

			case PIPE_CACHE_REQUEST:
				// Setup Cache Request
				req.tag = buf[2];
				req.sid = (buf[3]<<8) | buf[4];
				req.onid = (buf[5]<<8) | buf[6];
				req.caid = (buf[7]<<8) | buf[8];
				req.hash = (buf[9]<<24) | (buf[10]<<16) | (buf[11]<<8) |buf[12];
				req.prov = (buf[13]<<16) | (buf[14]<<8) | (buf[15]);
				// Check Cache Request
				if (!cache_check(&req)) break;
				//
				pcache = cache_fetch( &req );
				if (pcache==NULL) pcache = cache_new( &req );
				// Send Request if not sent
				if (pcache->sendcache==CACHE_NONE_SENT) {
					pcache->sendcache = CACHE_REQUEST_SENT;
					cfg.cache.req++;
					cache_send_request(pcache,NULL);
				}
				break;

			case PIPE_CACHE_REPLY:
				// Setup Cache Request
				req.tag = buf[2];
				req.sid = (buf[3]<<8) | buf[4];
				req.onid = (buf[5]<<8) | buf[6];
				req.caid = (buf[7]<<8) | buf[8];
				req.hash = (buf[9]<<24) | (buf[10]<<16) | (buf[11]<<8) |buf[12];
				req.prov = (buf[13]<<16) | (buf[14]<<8) | (buf[15]);
				// Check Cache Request
				if (!cache_check(&req)) break;
				//
				pcache = cache_fetch( &req );
				if (pcache==NULL) pcache = cache_new( &req );
				// Send Reply
				if (pcache->sendcache!=CACHE_REPLY_SENT) {
					//Set DCW
					if (len==32) {
						if (pcache->status!=CACHE_STAT_DCW) { // already sent
							memcpy( pcache->cw, buf+16, 16); //dcw
							pcache->status = CACHE_STAT_DCW;
							pcache->peerid = 0; // from local
						}
					}
					pcache->sendcache = CACHE_REPLY_SENT;
					if (pcache->status==CACHE_STAT_DCW) cfg.cache.rep++;
					cache_send_reply(pcache,NULL);
				}
				break;

		}//switch
	}//if
}

void cache_check_peers()
{
	struct cachepeer_data *peer = cfg.cache.peer;
	while (peer) {
		if ( (peer->host->ip)&&(peer->port) ) {
			uint ticks = GetTickCount();
			if (peer->ping==0) { // inactive
				if ( (!peer->lastpingsent)||((peer->lastpingsent+5000)<ticks) ) { // send every 15s
					cache_send_ping(peer);
					peer->lastpingsent = ticks;
					peer->lastpingrecv = 0;
					peer->ping = -1;
				}
			}
			else if (peer->ping==-1) { // inactive
				if ( (!peer->lastpingsent)||((peer->lastpingsent+10000)<ticks) ) { // send every 15s
					cache_send_ping(peer);
					peer->lastpingsent = ticks;
					peer->lastpingrecv = 0;
					peer->ping = -2;
				}
			}
			else if (peer->ping==-2) { // inactive
				if ( (!peer->lastpingsent)||((peer->lastpingsent+20000)<ticks) ) { // send every 15s
					cache_send_ping(peer);
					peer->lastpingsent = ticks;
					peer->lastpingrecv = 0;
					peer->ping = -3;
				}
			}
			else if (peer->ping<=-3) { // inactive
				if ( (!peer->lastpingsent)||((peer->lastpingsent+60000)<ticks) ) { // send every 15s
					cache_send_ping(peer);
					peer->lastpingsent = ticks;
					peer->lastpingrecv = 0;
				}
			}
			else if (peer->ping>0) {
				if ( (!peer->lastpingrecv)&&((peer->lastpingsent+5000)<ticks) ) {
					cache_send_ping(peer);
					peer->lastpingsent = ticks;
					peer->lastpingrecv = 0;
					peer->ping = 0;
					peer->host->checkiptime = 0; // maybe ip changed
				}
				else if ( (peer->lastpingsent+60000)<ticks ) { // send every 75s
#ifdef NEWCACHE
					cache_send_keepalive(peer);
#else
					cache_send_ping(peer);
#endif
					peer->lastpingsent = ticks;
					peer->lastpingrecv = 0;
				}
			}
		}
		peer = peer->next;
	}
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

void *cache_thread(void *param)
{
	prg.pid_cache = syscall(SYS_gettid);
	prg.tid_cache = pthread_self();

	uint32 chkticks = 0;

	while(1) {
		if (cfg.cache.handle>0) {
			// Check Peers Ping
			if ( GetTickCount()>(chkticks+3000) ) {
				cache_check_peers();
				chkticks = GetTickCount();
			}
			struct pollfd pfd;
			pfd.fd = cfg.cache.handle;
			pfd.events = POLLIN | POLLPRI;
			int retval = poll(&pfd, 1, 3000);

			if ( retval>0 ) {
				if ( pfd.revents & (POLLIN|POLLPRI) ) {
					pthread_mutex_lock( &prg.lockcache );
					cache_recvmsg();
/*
					do {
						cache_recvmsg();
						pfd.fd = cfg.cache.handle;
						pfd.events = POLLIN | POLLPRI;
						if ( poll(&pfd, 1, 0) <1 ) break;
					} while (pfd.revents & (POLLIN|POLLPRI));
*/
					pthread_mutex_unlock( &prg.lockcache );
				}
			}

		} else usleep( 30000 );
	}
	close(cfg.cache.handle);
}


void *cachepipe_thread(void *param)
{
	while(1) {
		struct pollfd pfd;
		pfd.fd = srvsocks[1];
		pfd.events = POLLIN | POLLPRI;
		int retval = poll(&pfd, 1, 3000);
		if ( retval>0 ) {
			if ( pfd.revents & (POLLIN|POLLPRI) ) {
				pthread_mutex_lock( &prg.lockcache );
				cache_pipe_recvmsg();
				pthread_mutex_unlock( &prg.lockcache );
			}
		}
	}
}

int start_thread_cache()
{
	// Default outsock
	outsock = CreateClientSockUdp();
	create_prio_thread(&prg.tid_cache, (threadfn)cachepipe_thread,NULL, 50);
	create_prio_thread(&prg.tid_cache, (threadfn)cache_thread,NULL, 50);
	return 0;
}

