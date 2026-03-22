
void* thread_enddate(void *param)
{
	prg.pid_date = syscall(SYS_gettid);
	while(1) {
		pthread_mutex_lock(&prg.lockthreaddate);

		uint32_t ticks = GetTickCount();
		time_t nowtime = time(NULL);
		struct tm *nowtm = localtime(&nowtime);
		//strftime(buf, sizeof(buf), "%d %b %Y %H:%M", nowtm); printf(" Local Time = %s %d\n", buf, nowtm->tm_yday);

		int j = (nowtm->tm_mon<<16) | (nowtm->tm_mday<<8) | nowtm->tm_hour;
		// CCcam Clients
		struct cccamserver_data *cccam = cfg.cccam.server;
		while (cccam) {
			struct cc_client_data *cli = cccam->client;
			while (cli) {
				if (!(cli->flags&FLAG_REMOVE))
				if (cli->enddate.tm_year) {
					int i = (cli->enddate.tm_mon<<16) | (cli->enddate.tm_mday<<8) | cli->enddate.tm_hour;
					//strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &cli->enddate); printf(" Client End date = %s\n", buf);
					if (cli->flags&FLAG_EXPIRED) {
						if (cli->enddate.tm_year > nowtm->tm_year) {
							cli->flags &= ~FLAG_EXPIRED;
							debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Client '%s' Enabled\n", cccam->id, cli->user);
						}
						else if (cli->enddate.tm_year==nowtm->tm_year) {
							if (i>j) {
								cli->flags &= ~FLAG_EXPIRED;
								debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Client '%s' Enabled\n", cccam->id, cli->user);
							}
						}
					}
					else {
						if (cli->enddate.tm_year < nowtm->tm_year) {
							cli->flags |= FLAG_EXPIRED;
							cc_disconnect_cli(cli);
							debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Client '%s' Expired\n", cccam->id, cli->user);
						}
						else if (cli->enddate.tm_year==nowtm->tm_year) {
							if (j>=i) {
								cli->flags |= FLAG_EXPIRED; // printf(" Client Disabled %s\n", cli->user);
								cc_disconnect_cli(cli);
								debugf(getdbgflag(DBG_CCCAM,cli->srvid,cli->id)," CCcam%d: Client '%s' Expired\n", cccam->id, cli->user);
							}
						}
						else if ( (cli->handle>0) && ( (ticks-cli->lastecmtime)>600000 ) ) 	cc_disconnect_cli(cli);
					}
				}
				cli = cli->next;
			}
			cccam = cccam->next;
		}

		// MGcamd Clients
		// CCcam Clients
		struct mgcamdserver_data *mgcamd = cfg.mgcamd.server;
		while (mgcamd) {
			struct mg_client_data *mgcli = mgcamd->client;
			while (mgcli) {
				if (!(mgcli->flags&FLAG_REMOVE))
				if (mgcli->enddate.tm_year) {
					//strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &mgcli->enddate); printf(" Client End date = %s\n", buf);
					int i = (mgcli->enddate.tm_mon<<16) | (mgcli->enddate.tm_mday<<8) | mgcli->enddate.tm_hour;
					if (mgcli->flags&FLAG_EXPIRED) {
						if (mgcli->enddate.tm_year > nowtm->tm_year) {
							mgcli->flags &= ~FLAG_EXPIRED; //printf(" Client Enabled %s\n", mgcli->user);
							debugf(getdbgflag(DBG_MGCAMD,0,mgcli->id)," mgcamd%d: Client '%s' Enabled\n", mgcamd->id, mgcli->user);
						}
						else if (mgcli->enddate.tm_year==nowtm->tm_year) {
							if (i>j) {
								mgcli->flags &= ~FLAG_EXPIRED; //printf(" Client Enabled %s\n", mgcli->user);
								debugf(getdbgflag(DBG_MGCAMD,0,mgcli->id)," mgcamd%d: Client '%s' Enabled\n", mgcamd->id, mgcli->user);
							}
						}
					}
					else {
						if (mgcli->enddate.tm_year < nowtm->tm_year) {
							mgcli->flags |= FLAG_EXPIRED;
							mg_disconnect_cli(mgcli);
							debugf(getdbgflag(DBG_MGCAMD,0,mgcli->id)," mgcamd%d: Client '%s' Expired\n", mgcamd->id, mgcli->user);
						}
						else if (mgcli->enddate.tm_year==nowtm->tm_year) {
							if (j>=i) {
								mgcli->flags |= FLAG_EXPIRED;
								mg_disconnect_cli(mgcli);
								debugf(getdbgflag(DBG_MGCAMD,0,mgcli->id)," mgcamd%d: Client '%s' Expired\n", mgcamd->id, mgcli->user);
							}
						}
					}
				}
				mgcli = mgcli->next;
			}
			mgcamd = mgcamd->next;
		}

		pthread_mutex_unlock(&prg.lockthreaddate);
		sleep(10);
	}
}

void start_thread_date()
{
	create_prio_thread(&prg.tid_date, (threadfn)thread_enddate,NULL,50);
}

