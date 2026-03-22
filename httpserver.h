
extern struct config_data cfg;
extern struct program_data prg;

extern int END_PROCESS;

char *getchname(uint16 caid, uint32 prov, uint16 sid );

#ifdef WIN32
DWORD WINAPI http_thread(int *param);
#else
void *http_thread(void *param);
#endif

int start_thread_http();

