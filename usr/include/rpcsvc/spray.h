#define SPRAYMAX 8845

struct spraytimeval {
	u_int sec;
	u_int usec;
};
typedef struct spraytimeval spraytimeval;
bool_t xdr_spraytimeval();


struct spraycumul {
	u_int counter;
	spraytimeval clock;
};
typedef struct spraycumul spraycumul;
bool_t xdr_spraycumul();


typedef struct {
	u_int sprayarr_len;
	char *sprayarr_val;
} sprayarr;
bool_t xdr_sprayarr();


#define SPRAYPROG ((u_long)100012)
#define SPRAYVERS ((u_long)1)
#define SPRAYPROC_SPRAY ((u_long)1)
extern void *sprayproc_spray_1();
#define SPRAYPROC_GET ((u_long)2)
extern spraycumul *sprayproc_get_1();
#define SPRAYPROC_CLEAR ((u_long)3)
extern void *sprayproc_clear_1();

