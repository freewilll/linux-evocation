
#define SM_PROG ((u_long)100024)
#define SM_VERS ((u_long)1)
#define SM_STAT ((u_long)1)
extern struct sm_stat_res *sm_stat_1();
#define SM_MON ((u_long)2)
extern struct sm_stat_res *sm_mon_1();
#define SM_UNMON ((u_long)3)
extern struct sm_stat *sm_unmon_1();
#define SM_UNMON_ALL ((u_long)4)
extern struct sm_stat *sm_unmon_all_1();
#define SM_SIMU_CRASH ((u_long)5)
extern void *sm_simu_crash_1();

#define SM_MAXSTRLEN 1024

struct sm_name {
	char *mon_name;
};
typedef struct sm_name sm_name;
bool_t xdr_sm_name();


struct my_id {
	char *my_name;
	int my_prog;
	int my_vers;
	int my_proc;
};
typedef struct my_id my_id;
bool_t xdr_my_id();


struct mon_id {
	char *mon_name;
	struct my_id my_id;
};
typedef struct mon_id mon_id;
bool_t xdr_mon_id();


struct mon {
	struct mon_id mon_id;
	char priv[16];
};
typedef struct mon mon;
bool_t xdr_mon();


struct sm_stat {
	int state;
};
typedef struct sm_stat sm_stat;
bool_t xdr_sm_stat();


enum res {
	stat_succ = 0,
	stat_fail = 1,
};
typedef enum res res;
bool_t xdr_res();


struct sm_stat_res {
	res res_stat;
	int state;
};
typedef struct sm_stat_res sm_stat_res;
bool_t xdr_sm_stat_res();


struct status {
	char *mon_name;
	int state;
	char priv[16];
};
typedef struct status status;
bool_t xdr_status();

