
#define KEEPALIVE_NEWCAMD	80


/// FLAGS defined in config
#define FLAG_DEFCONFIG  0x01
// FLAG_DISABLE: by user
#define FLAG_DISABLE    0x02
// FLAG_DISABLE: by user
#define FLAG_EXPIRED    0x04
// FLAG_REMOVE: by config reader
#define FLAG_REMOVE     0x08
// FLAG_DISCONNECT: by config reader, to reconnect
#define FLAG_DISCONNECT 0x10


#define IS_DISABLED(x)  ( x & (FLAG_DISABLE|FLAG_EXPIRED|FLAG_REMOVE) )


#if TARGET == 3
#define MAX_SIDS 1024
#else
#define MAX_SIDS 4096
#endif


struct sid_chid_data {
	uint16_t sid;
	uint16_t chid;
};

#define MAX_CSPORTS 200

struct sharelimit_data {
	uint16_t caid;
	uint32_t provid;
	uint8_t uphops; // 0: deny
};

struct ip2country_data {
	struct ip2country_data *next;
	uint32_t ipstart;
	uint32_t ipend;
	char code[70];
};

struct country_image_data {
	char code[3];
	char name[40];
	int len;
	uint8_t data[512];
};

struct chninfo_data
{
	struct chninfo_data *next;
	uint16_t sid;
	uint16_t caid;
	uint32_t prov;
	char name[0];
};

struct providers_data
{
	struct providers_data *next;
	uint32_t caprovid;
	char name[0];
};

struct sid_data
{
	struct sid_data *next;
	//uint8_t nodeid[8]; // CCcam nodeid card real owner, each server has many cards(locals+remote) and propably each server has different locals
	uint16_t sid;
	//uint16_t caid;
	uint32_t prov;
	int val;
};


#if TARGET == 3
#define CARD_MAXPROV 16
#else
#define CARD_MAXPROV 32
#endif

struct cs_card_data
{
	struct cs_card_data *next;
	uint32_t shareid;			// This is for CCcam
	uint32_t localid;			// This is for Fake CCcam Cards
#ifdef CCCAM_CLI
	uint8_t uphops;		// Max distance to get cards
	uint8_t dnhops;
	uint8_t nodeid[8]; // CCcam nodeid card real owner
#endif

	struct sid_data *sids[256]; // 0..FF

	// ECM Statistics
	int ecmerrdcw;  // null DCW/failed DCW checksum (diffeent dcw)
	int ecmnb;	// number of ecm's requested
	int ecmok;	// dcw returned to client
	int ecmoktime;

	uint16_t caid;		// Card CAID
	int  nbprov;				// Nb providers
	uint32_t prov[CARD_MAXPROV];		// Card Providers
};


struct host_data
{
	struct host_data *next;
	uint32_t flags;
	char name[256];
	uint32_t ip;
	uint32_t checkiptime;
	uint32_t clip; // client ip
};

/*
ECM total: 0 (average rate: 0/s)
ECM forwards: 0 
ECM cache hits: 0 
ECM denied: 0 
ECM filtered: 0 
ECM failures: 0
EMM total: 0
*/

struct sms_data {
	struct sms_data *next;
	int status; // bit 0 (0:in,1:out) bit 1 (0:unread/1:read)
	uint32_t hash;
	char msg[1024];
	struct timeval tv;
};

#define CACHE_PROG_DEFAULT 0
#define CACHE_PROG_CSP     1
#define CACHE_PROG_MULTICS 2

#define PEER_OFFLINE       0
#define PEER_ONLINE        1


struct cachepeer_data
{
	// Config data
	struct cachepeer_data *next;
	uint32_t flags;
	uint32_t id; // unique id

	struct host_data *host;
	uint16_t port;
	uint16_t csport[MAX_CSPORTS];
	int fblock0onid;
	int fsendrep;
	//## Runtime Data
	int runtime; // Added At Runtime

	struct sms_data *sms;

	int outsock;
	uint16_t recvport;
	int status;
	int ping; // <=0 : inactive
	uint32_t lastpingsent; // last ping sent to peer
	uint32_t lastpingrecv; // last ping received from peer after a ping request

	char program[32]; // Program Name
	char version[32]; // Program version
#ifdef NEWCACHE
	int protocol; // Cache Protocol Version (0:CSP Protocol)
	int ismultics;
#endif
	uint32_t cards[1024];
	int ipoll;
	//Stat
	int sentreq;
	int sentrep;
	//
	uint8_t crc[4];
/*
	int totreq; // total received requests (+errors)
	int totrep; // total received replies (+errors)
	int rep_badheader; // wrong header
	int rep_badfields; // badfields blocked replies
	int rep_failed; // failed replies
	int rep_baddcw;
*/
	int reqnb; // Total Requests (Good+Bad)
	int reqok; // Good Requests
	int repok;		// Replies without request
	int hitnb;     // All DCW transferred to clients
	int ihitnb;     // Instant Hits

	int hitfwd;  // Hits forwarded to peer
	int ihitfwd; // Instant Hits Forwarded to peer
	
	//Last Used Cached data
	uint16_t lastcaid;
	uint32_t lastprov;
	uint16_t lastsid;
	uint32_t lastdecodetime;

};


struct ip_hacker_data {
	struct ip_hacker_data *next;
	uint32_t ip;
	char user[256];
	uint32_t lastseen;
	int count;
	int nbconn;
};

///////////////////////////////////////////////////////////////////////////////

typedef enum
{
	STAT_DCW_SENT,	// no ecm found / DCW was sent to client
	STAT_ECM_SENT,	// ECM was sent to server
	STAT_ECM,		// ECM is waiting to be send
	STAT_DCW		// DCW is waiting to be send
} sendstatus_type;

// CLIENT FLAGS

struct client_info_data
{
	struct client_info_data *next;
	char name[32];
	char value[256];
};

struct cs_client_data
{
	struct cs_client_data *next;
	uint32_t flags;
	uint32_t id; // unique id
	uint32_t pid; // Profile id
	uint32_t gid;
	// User/Pass
	char user[64];
	char pass[64];
	//
	uint8_t type; // Clients type: NEWCAMD
	// Card
	struct cs_card_data card;
	//DCW Config
	uint32_t dcwtime; // minimum time interval (from Receiving ECM to sending CW) to client
	int dcwtimeout; // decode timeout interval in ms
#ifdef DCWPERIOD
	uint32_t dcwperiod; // min time interval between 2 cw
#endif

	// Client Info Data
	struct client_info_data *info;
	char *realname;
	struct tm enddate;
	struct host_data *host;

	//## Runtime Data (DYNAMIC)
	uint32_t ip;
	int handle;
	int ipoll;
	uint32_t chkrecvtime; // message recv time
	//
	uint16_t progid; // program id ex: 0x4343=>CCcam/ 0x0000=>Generic
	// Connection time
	uint32_t uptime;
	uint32_t connected;
	uint32_t ping; // ping time;
	// Session Key
	uint8_t sessionkey[16];
	// ECM Stat
	int ecmnb;	// ecm number requested by client
	int ecmdenied;	// ecm number requested by client
	int ecmok;	// dcw returned to client
	int ecmoktime;
	uint32_t lastactivity; // Last Received Packet
	uint32_t lastecmtime; // Last ecm time, if it was more than 5mn so reconnect to client
	uint32_t lastdcwtime; // last good dcw time sent to client

#ifdef SRV_CSCACHE
	int cachedcw; // dcw from client
#endif

	int freeze; //a freeze: is a decode failed to a channel opened last time within 3mn
	int zap;
	int nblogin; // Total Number of logins
	int nbloginerror; // Total Number of logins
	int nbdiffip; // Total Number of logins with different IP's

	// ECM
	struct {
		int busy; // if ecmbusy dont process anyother ecm until that current ecm was finished
		sendstatus_type status; // answer was sent to client?
		// Ecm Data
		uint32_t recvtime; // ECM Receive Time in ms
		int dcwtime; //depend on last ecm time
		int id;
		uint32_t hash; // to check for ecm
		int climsgid; // MessageID for the ecm request
		//Last Used Share Saved data
		int lastid;
		uint16_t lastcaid;
		uint32_t lastprov;
		uint16_t lastsid;
		int laststatus; // 1:success, 0:failed
		uint32_t lastdecodetime;
		// DCW SOURCE
		int lastdcwsrctype;
		int lastdcwsrcid;
		uint32_t lastcardid;
	} ecm;
};


#ifdef RADEGAST_SRV

struct rdgd_client_data { // Connected Client
	struct rdgd_client_data *next;
	uint32_t flags;
	uint32_t id; // unique id

	// Share Limits
	struct sharelimit_data sharelimits[100];
	// Client Info Data
	struct client_info_data *info;
	char *realname;
	struct tm enddate;
	struct host_data *host;

	//## Runtime Data (DYNAMIC)
	uint32_t ip;
	int handle;
	int ipoll;
	uint32_t chkrecvtime; // message recv time

	// Connection time
	uint32_t connected;
	uint8_t type;
	// ECM Stat
	int ecmnb;	// ecm number requested by client
	int ecmdenied;	// ecm number requested by client
	int ecmok;	// dcw returned to client
	int ecmoktime;
	uint32_t lastactivity; // Last Received Packet
	uint32_t lastecmtime; // Last ecm time, if it was more than 5mn so reconnect to client
	uint32_t lastdcwtime; // last good dcw time sent to client
	//
	int freeze; //a freeze: is a decode failed to a channel opened last time within 3mn
	int zap;
	int nblogin; // Total Number of logins
	int nbloginerror; // Total Number of logins
	int nbdiffip; // Total Number of logins with different IP's

	struct {
		int busy; // if ecmbusy dont process anyother ecm until that current ecm was finished
		sendstatus_type status; // answer was sent to client?
		// Ecm Data
		uint32_t recvtime; // ECM Receive Time in ms
		int dcwtime; //depend on last ecm time
		int id;
		//Last Used Share Saved data
		uint16_t lastcaid;
		uint32_t lastprov;
		uint16_t lastsid;
		int laststatus;
		// DCW SOURCE
		int lastdcwsrctype;
		int lastdcwsrcid;
		uint32_t lastcardid;
		uint32_t lastdecodetime;
		char *statmsg; // DCW Status Message
	} ecm;
};

#endif



#ifdef CAMD35_SRV

struct camd35_client_data { // Connected Client
	struct camd35_client_data *next;
	int disabled; // 0:Enable/1:Disable Client
	unsigned int id; // unique id

	struct client_info_data *info;
	char *realname;

	// User/Pass
	char user[64];
	char pass[64];
	// Card
	struct cs_card_data card;
	// AES KEYS
	AES_KEY decryptkey;
	AES_KEY encryptkey;
	uint32 ucrc;

	//## Runtime Data (DYNAMIC)
	unsigned int ip;
	int handle;
	int ipoll;
	uint32 chkrecvtime; // message recv time

	// Connection time
	unsigned int connected;
	unsigned char type;
	// ECM Stat
	int ecmnb;	// ecm number requested by client
	int ecmdenied;	// ecm number requested by client
	int ecmok;	// dcw returned to client
	int ecmoktime;
	unsigned int lastecmtime; // Last ecm time, if it was more than 5mn so reconnect to client
	unsigned int lastdcwtime; // last good dcw time sent to client
	//
	int freeze; //a freeze: is a decode failed to a channel opened last time within 3mn
	struct {
		int busy; // if ecmbusy dont process anyother ecm until that current ecm was finished
		sendstatus_type status; // answer was sent to client?
		// Ecm Data
		uint32 recvtime; // ECM Receive Time in ms
		int dcwtime; //depend on last ecm time
		int id;
		//Last Used Share Saved data
		uint16 lastcaid;
		uint32 lastprov;
		uint16 lastsid;
		int laststatus;
		// DCW SOURCE
		int lastdcwsrctype;
		int lastdcwsrcid;
		uint32 lastcardid;
		uint32 lastdecodetime;
		char *statmsg; // DCW Status Message
	} ecm;
};

#endif

// cs : newcamd
// cc : cccam

struct cardserver_data
{
	struct cardserver_data *next;
	uint32_t flags;

	uint32_t id; // unique id
	char name[64];
	//NEWCAMD SERVER
	struct {
		struct cs_client_data *client;
		int totalclients;
		uint32_t flags;
		uint8_t key[16];
		int port; // output port
		SOCKET handle;
		int ipoll;
	} newcamd;
#ifdef RADEGAST_SRV
	struct {
		struct rdgd_client_data *client;
		uint32_t flags;
		int port; // output port
		SOCKET handle;
		int ipoll;
	} radegast;
#endif

#ifdef CAMD35_SRV
	struct {
		struct camd35_client_data *client; // clients
		int port; // output port
		SOCKET handle;
		int ipoll;
	} camd35;
#endif

	//CARD
	struct cs_card_data card;
	//OPTIONS
	struct {
		uint16_t onid;

		struct {
			uint32_t time; // minimum time interval to send decode answer to client
			uint32_t timeout; // decode timeout in ms
#ifdef DCWPERIOD
			uint32_t period; // min time interval between 2 cw
			uint32_t algorithm;
#endif
#ifdef CHECK_NEXTDCW
			uint16_t check;
#endif
		} dcw;

		int maxfailedecm; // Max failed ecm per sid

		int cachetimeout;

		int faccept0sid;
		int faccept0provider;
		int faccept0caid;

		int fallowcccam;	// Allow cccam server protocol to decode ecm
		int fallownewcamd;	// Allow newcamd server protocol to decode ecm
		int fallowradegast;
		int fallowcache;
		int cachesendreq;
		int cacheresendreq;
		int cachesendrep;
		int fmaxuphops; // allowed cards distance to decode ecm
		int cssendcaid; // flag send caid to servers
		int cssendprovid; // flag send provid to servers
		int cssendsid; // flag send sid to servers
		// Servers Config
		struct {
			uint32_t max;		// Maximum sevrer nb available to decode one ecm request
			uint32_t first; // on start request servers number
			uint32_t interval;    // interval between 2 same ecm request to diffrent server
			uint32_t timeout;     // timeout for resending ecm request to server
			uint32_t timeperecm;  // min time to senddo a request
			uint32_t validecmtime;  // max server ecm reply time
		} server;
		// Server Retry
		struct {
			int newcamd; // Newcamd Retries
			int cccam; // CCcam Retries
#ifdef RADEGAST_CLI
			int radegast; // Radegast Retries
#endif
		} retry;
	} option;
	// SIDS
	int isdeniedsids;
	struct sid_chid_data *sids; // global Accept sids

	///////////////////////////////////////////////////////////////////////////

	int cachehits;  // total cache hits
	int cacheihits; // instant cache hits
	// ECM Stat
	int ecmaccepted;	// accepted ecm
	int ecmdenied;	// denied/filtred ecm
	int ecmok;	// good dcw
	int ecmoktime;

	int ttime[101]; // contains number of dcw/time (0.0s,0.1s,0.2s ... 2.9s)
	int ttimecache[101]; // for cache only
	int ttimecards[101]; // for cards only
#ifdef SRV_CSCACHE
	int ttimeclients[101]; // for cards only
#endif
	// Last Decode
	struct {
		void *ecm;
		uint32_t ecmtime; // Last ecm time, if it was more than 5mn so reconnect to client
		uint32_t dcwtime; // last good dcw time sent to client
	} last;

	int ecmbusysrv; // nb of ecm returned with busy srv

#ifdef CHECK_HACKER
	struct ip_hacker_data *iplist;
#endif

};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//server is in config
#define FLAGSRV_CONFIG  1

// Server/client type
#define TYPE_NEWCAMD    1
#define TYPE_CCCAM      2
#define TYPE_GBOX       3
#define TYPE_RADEGAST   4
#define TYPE_CLONE      5


struct cs_server_data
{
	struct cs_server_data *next;
	uint32_t flags;
	uint32_t id; // unique id
	//*** Config DATA
	uint8_t type; // Clients type: NEWCAMD/CCcam
	struct host_data *host;
	int port;
	char user[64];
	char pass[64];
	pthread_mutex_t lock;
	// Newcamd
	uint8_t key[16];
	// Profiles
	uint16_t csport[MAX_CSPORTS];
	// Server Priority
	int priority; // Priority Server
	// Share Limits
	struct sharelimit_data sharelimits[100];
	// ACCEPTED SIDs
	struct sid_chid_data *sids; // Accepted sids

	//*** DYNAMIC DATA
	char *progname; // Known names
	char version[32];
	uint8_t sessionkey[16];
#ifdef CLI_CSCACHE
	int cscached; // flag for newcamd cached servers
#endif
	int error;
	char *statmsg; // Connection Status Message
	//CCcam additional data
#ifdef CCCAM_CLI
	struct cc_crypt_block sendblock;	// crypto state block
	struct cc_crypt_block recvblock;	// crypto state block
	uint8_t nodeid[8];
	char build[32];
#endif
	//Connection Data
	SOCKET handle;
	int ipoll;
	uint32_t chkrecvtime; // message recv time
	uint32_t connected;
	uint32_t uptime; // in ms
	struct cs_card_data *card;
	// TCP Connection Keepalive
	uint32_t ping; // ping time;
	uint32_t keepalivetime;	// CON: last Keepalive sent time / DIS: start time for trying connection 
	int keepalivesent;	// CON: Keepalive packet was sent to server and we have no response. / DIS: Retry number of reconnection
	// ECM Statistics
	int ecmtimeout; // number of errors for timeout (no cw returned by server)
	int ecmerrdcw;  // null DCW/failed DCW checksum (diffeent dcw)
	int ecmnb;	// total number of ecm requests
	int ecmok;	// dcw returned to client
	int ecmoktime;
	int ecmperhr;
	// CURRENT ECM DATA
	int busy; // cardserver is busy (ecm was sent) / or not (no ecm sent/dcw returned)
	int busyecmid; // ecm id
	uint32_t busyecmhash; // to check for ecm
	struct cs_card_data *busycard; // card
	uint32_t busycardid; // card
	// Last ECM Stat
	uint32_t lastecmoktime;
	uint32_t lastecmtime; // Last ecm time, if it was more than 5mn so reconnect to client
	uint32_t lastdcwtime; // last good dcw time sent to client

	int retry; // nb of retries of the current ecm request

	struct {
		int csid;
		int ecmnb;
		int ecmok;
		uint32_t ecmoktime;
		int hits; // Ecm hits got from this server 
	} cstat[MAX_CSPORTS];
};


#ifdef CCCAM_SRV

struct cc_client_data { // Connected Client
	//### Config Data (STATIC)
	struct cc_client_data *next;
	uint32_t flags;

	uint32_t id; // unique id
	uint32_t srvid;
	//fline
	char user[64];
	char pass[64];
	uint8_t dnhops;		// Max Down Hops
	uint32_t dcwtime;
	uint8_t uphops;		// Max distance to get cards
	uint8_t shareemus;		// Client use our emu
	uint8_t allowemm;		// Client has rights for au

	// Profiles
	uint16_t csport[MAX_CSPORTS];
	// Share Limits
	struct sharelimit_data sharelimits[100];
	// Client Info Data
	struct client_info_data *info;
	char *realname;
	struct tm enddate;
	struct host_data *host;

	//## Runtime Data (DYNAMIC)
	uint32_t ip;
	int handle;				// SOCKET
	int ipoll;
	uint32_t chkrecvtime; // message recv time
	// CCcam Connection Data
	struct cc_crypt_block sendblock;	// crypto state block
	struct cc_crypt_block recvblock;	// crypto state block
	// Connection time
	uint32_t uptime;
	uint32_t connected;

	uint8_t nodeid[8];
	char version[32];
	char build[32];

	int cardsent; // flag

	// ECM Stat
	int ecmnb;	// ecm number requested by client
	int ecmdenied;	// ecm number requested by client
	int ecmok;	// dcw returned to client
	int ecmoktime;
	uint32_t lastactivity; // Last Received Packet
	uint32_t lastecmtime; // Last ecm time, if it was more than 5mn so reconnect to client
	uint32_t lastdcwtime; // last good dcw time sent to client

	int freeze; //a freeze: is a decode failed to a channel opened last time within 3mn
	int zap;
	int nblogin; // Total Number of logins
	int nbloginerror; // Total Number of logins
	int nbdiffip; // Total Number of logins with different IP's

	struct {
		int busy; // if ecmbusy dont process anyother ecm until that current ecm was finished
		sendstatus_type status; // answer was sent to client?
		uint32_t cardid;
		// Ecm Data
		uint32_t recvtime; // ECM Receive Time in ms
		int dcwtime; //depend on last ecm time
		int id; //ecmid
		uint32_t hash; // to check for ecm
		//Last Used Share Saved data
		int lastid;
		uint16_t lastcaid;
		uint32_t lastprov;
		uint16_t lastsid;
		int laststatus;
		uint8_t lastdcw[16];
		int lastdcwsrctype;
		int lastdcwsrcid;
		uint32_t lastcardid;
		uint32_t lastdecodetime;
		char *statmsg; // DCW Status Message
	} ecm;
};

#endif

#ifdef CCCAM

struct cccamserver_data {
#ifdef CCCAM_SRV
	struct cccamserver_data *next;
	uint32_t flags;
	struct cc_client_data *client;
	int totalclients;
	int id;
	int handle;
	int ipoll;
	int port; // output port
#endif
};

#endif





#ifdef MGCAMD_SRV

struct mg_client_data
{
	struct mg_client_data *next;
	uint32_t flags;

	uint32_t id; // unique id
	uint32_t srvid;
	// NEWCAMD SPECIFIC DATA
	char user[64];
	char pass[64];
	//DCW Config
	uint32_t dcwtime; // minimum time interval (from Receiving ECM to sending CW) to client
	int dcwtimeout; // decode timeout interval in ms

	// Profiles
	uint16_t csport[MAX_CSPORTS];
	// Share Limits
	struct sharelimit_data sharelimits[100];
	// Client Info Data
	struct client_info_data *info;
	char *realname;
	struct tm enddate;
	struct host_data *host;


	//## Runtime Data (DYNAMIC)
	uint32_t ip;
	SOCKET handle;
	int ipoll;
	uint32_t chkrecvtime; // message recv time
	uint16_t progid; // program id ex: 0x4343=>CCcam/ 0x0000=>Generic
	uint8_t sessionkey[16];
	// Connection time
	uint32_t uptime;
	uint32_t connected;
	uint32_t ping; // ping time;

	int cardsent; // flag 0:none, 1:default, 2:all
	// ECM Stat
	int ecmnb;	// ecm number requested by client
	int ecmdenied;	// ecm number requested by client
	int ecmok;	// dcw returned to client
	int ecmoktime;
	uint32_t lastactivity; // Last Received Packet
	uint32_t lastecmtime; // Last ecm time, if it was more than 5mn so reconnect to client
	uint32_t lastdcwtime; // last good dcw time sent to client

	int freeze; //a freeze: is a decode failed to a channel opened last time within 3mn
	int zap;
	int nblogin; // Total Number of logins
	int nbloginerror; // Total Number of logins
	int nbdiffip; // Total Number of logins with different IP's

#ifdef SRV_CSCACHE
	int cachedcw; // dcw from client
#endif

	struct {
		int busy; // if ecmbusy dont process anyother ecm until that current ecm was finished
		sendstatus_type status; // answer was sent to client?
		// Ecm Data
		uint32_t recvtime; // ECM Receive Time in ms
		int dcwtime; //depend on last ecm time
		int id;
		uint32_t hash; // to check for ecm
		int climsgid;
		//Last Used Share Saved data
		int lastid;
		uint16_t lastcaid;
		uint32_t lastprov;
		uint16_t lastsid;
		int laststatus;
		// DCW SOURCE
		int lastdcwsrctype;
		int lastdcwsrcid;
		uint32_t lastcardid;
		uint32_t lastdecodetime;
	} ecm;
};


struct mgcamdserver_data {
	struct mgcamdserver_data *next;
	uint32_t flags;
	struct mg_client_data *client;
	int totalclients;

	int id;
	int handle;
	int ipoll;
	int port;

	uint16_t csport[MAX_CSPORTS]; // default cards
	uint8_t key[16];
	int dcwtime;
};

#endif

struct dcw_data {
	struct dcw_data *next;
	uchar dcw[16];
};

struct filename_data
{
	struct filename_data *next;
	char name[512];
	int wd;
};


// Configurable Data
struct config_data
{
	struct filename_data *files;
	char stylesheet_file[256];
	char channelinfo_file[256];
	char providers_file[256];
	char ip2country_file[256];
	struct ip2country_data *ip2country;
	struct chninfo_data *chninfo;
	struct providers_data *providers;

#ifdef TESTCHANNEL
	struct {
		uint16_t caid;
		uint32_t prov;
		uint16_t sid;
	} testchn;
#endif

	// UniqueID counters
	int clientid;
	int serverid;
	int cardserverid; // Profiles
	int cachepeerid; // 

	struct dcw_data *bad_dcw;

	// CACHE SERVERS
	struct {
		int tracker; // 0: normal mode, 1: trackermode
		int port;
		int handle;
		struct cachepeer_data *peer;
		int totalpeers;
		//
		int hits;  // Total Hits
		int ihits; // Instant Hits
		int req; // Request sent
		int rep; // Replies
		int faccept0onid;
	} cache;

	//SERVERS
	int newcamdclientid;
	struct cs_server_data *server;
	int totalservers;

	// Host List
	struct host_data *host; 

	//CS PROFILES
	struct cardserver_data *cardserver;
	int totalprofiles;

#ifdef CCCAM
	struct {
		struct cccamserver_data *server;
		int totalservers;
		int clientid; // CCcam Clients
		int serverid; // CCcam Servers
		uint32_t dcwtime;
		uint8_t nodeid[8];
		char version[32];
		char build[32];
		uint16_t csport[MAX_CSPORTS]; // default cards
	} cccam;
#endif

#ifdef FREECCCAM_SRV
	struct {
		struct cccamserver_data server;
		int clientid; // CCcam Clients
		int serverid; // CCcam Servers
		uint32_t dcwtime;
		uint8_t nodeid[8];
		char version[32];
		char build[32];
		char user[64];
		char pass[64];
		int maxusers;
		uint16_t csport[MAX_CSPORTS]; // default cards
	} freecccam;
#endif


#ifdef MGCAMD_SRV
	struct {
		struct mgcamdserver_data *server;
		int totalservers;
		int clientid; // mgcamd Clients
		int serverid; // mgcamd Servers
		uint32_t dcwtime;
		uint16_t csport[MAX_CSPORTS]; // default cards
	} mgcamd;
#endif

	//WEBIF
#ifdef HTTP_SRV
	struct {
		int port;
		int handle;
		char user[64];
		char pass[64];
		int noeditor;
		int norestart;
		int autorefresh;
	} http;
#endif

	void *lastecm;
};

// Static Data
struct program_data
{
	int restart;
	struct timeval exectime; // last dcw time sent to client

#ifdef CHECK_HACKER
	struct ip_hacker_data *iplist;
#endif

	//PROCESS_ID
	pid_t pid_main;
	pid_t pid_cfg;
	pid_t pid_dns;
	pid_t pid_srv;
	pid_t pid_msg;
	pid_t pid_cache;

	//THREAD_ID
	pthread_t tid_cfg;
	pthread_t tid_dns;
	pthread_t tid_srv;
	pthread_t tid_msg;
	pthread_t tid_cache;

	pthread_t tid_date;
	pid_t pid_date;
	pthread_mutex_t lockthreaddate;

	pthread_mutex_t lockcache;

	pthread_mutex_t lock;		// ECM DATA(main data)
	pthread_mutex_t lockcli;	// CS Clients data
	pthread_mutex_t locksrv;	// CS Servers data
	// THREADS
	pthread_mutex_t locksrvth; // Servers connection thread
	pthread_mutex_t locksrvcs; // Newcamd server
	pthread_mutex_t lockdnsth; // DNS lookup Thread

#ifdef CCCAM_SRV
	pthread_mutex_t lockcccli; // CCcam Clients data
	pthread_mutex_t locksrvcc; // CCcam server
#endif
#ifdef FREECCCAM_SRV
	pthread_mutex_t lockfreecccli; // FreeCCcam Clients data
	pthread_mutex_t locksrvfreecc; // FreeCCcam server
#endif
	pthread_mutex_t lockrdgdcli; // Radegast Clients
	pthread_mutex_t lockrdgdsrv; // Radegast Server

#ifdef MGCAMD_SRV
	pthread_mutex_t lockclimg;	// CCcam Clients data
	pthread_mutex_t locksrvmg; // CCcam server
#endif

	pthread_mutex_t lockmain; // Check ECM/DCW Thread
	pthread_mutex_t lockecm;

	pthread_mutex_t lockhttp; // http Thread
	pthread_mutex_t lockdns;

	pthread_mutex_t lockdcw;
};

extern char config_file[256];
extern char cccam_nodeid[8];

void init_config(struct config_data *cfg);
int read_config(struct config_data *cfg);

int read_chinfo( struct config_data *cfg );
void free_chinfo( struct config_data *cfg );
int read_ip2country( struct config_data *cfg );
void free_ip2country( struct config_data *cfg );
int read_providers( struct config_data *cfg );
void free_providers( struct config_data *cfg );

void reread_config( struct config_data *cfg );
int check_config(struct config_data *cfg);
int done_config(struct config_data *cfg);
void cfg_set_id_counters(struct config_data *cfg);

void free_card(struct cs_card_data* card);
void free_cardlist(struct cs_card_data* card);

