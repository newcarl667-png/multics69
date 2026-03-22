
int srv_sharelimits(struct cs_server_data *srv, uint16_t caid, uint32_t provid)
{
	int i;
	int uphops1 = 10; // for 0:0
	int uphops2 = 10; // for caid:0
	for (i=0; i<100; i++) {
		if (srv->sharelimits[i].caid==0xffff) break;
		if (!srv->sharelimits[i].caid) {
			if (!srv->sharelimits[i].provid) uphops1 = srv->sharelimits[i].uphops;
		}
		else if (srv->sharelimits[i].caid==caid) {
			if (srv->sharelimits[i].provid==provid) return srv->sharelimits[i].uphops;
			else if (!srv->sharelimits[i].provid) uphops2 = srv->sharelimits[i].uphops;
		}
	}
	if (uphops2<uphops1) return uphops2; else return uphops1;// Max UPHOPS
}

struct cs_server_data *getsrvbyid(uint32 id)
{
	if (!id) return NULL;
	struct cs_server_data *srv = cfg.server;
	while (srv) {
		if (srv->id==id) return srv;
		srv = srv->next;
	}
	return NULL;
}


