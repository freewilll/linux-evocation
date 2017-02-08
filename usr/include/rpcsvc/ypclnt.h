/*	@(#)ypclnt.h 1.14 90/01/03 Copyr 1990 Sun Microsystems, Inc	*/

#ifndef _RPCSVC_YPCLNT_H
#define _RPCSVC_YPCLNT_H
 
#include <features.h>

#include <rpc/rpc.h>
#include <rpc/xdr.h>

/*
 * ypclnt.h
 * This defines the symbols used in the c language
 * interface to the NIS client functions.  A description of this interface
 * can be read in ypclnt(3N).
 */

/*
 * Failure reason codes.  The success condition is indicated by a functional
 * value of "0".
 */
#define YPERR_BADARGS 1			/* Args to function are bad */
#define YPERR_RPC 2			/* RPC failure */
#define YPERR_DOMAIN 3			/* Can't bind to a server which serves
					 *   this domain. */
#define YPERR_MAP 4			/* No such map in server's domain */
#define YPERR_KEY 5			/* No such key in map */
#define YPERR_YPERR 6			/* Internal NIS server or client
					 *   interface error */
#define YPERR_RESRC 7			/* Local resource allocation failure */
#define YPERR_NOMORE 8			/* No more records in map database */
#define YPERR_PMAP 9			/* Can't communicate with portmapper */
#define YPERR_YPBIND 10			/* Can't communicate with ypbind */
#define YPERR_YPSERV 11			/* Can't communicate with ypserv */
#define YPERR_NODOM 12			/* Local domain name not set */
#define YPERR_BADDB 13			/* NIS data base is bad */
#define YPERR_VERS 14			/* NIS version mismatch */
#define YPERR_ACCESS 15			/* Access violation */
#define YPERR_BUSY 16			/* Database is busy */

/*
 * Types of update operations
 */
#define YPOP_CHANGE 1			/* change, do not add */
#define YPOP_INSERT 2			/* add, do not change */
#define YPOP_DELETE 3			/* delete this entry */
#define YPOP_STORE  4			/* add, or change */
 
__BEGIN_DECLS 

/*           
 * Data definitions
 */

/*
 * struct ypall_callback * is the arg which must be passed to yp_all
 */

struct ypall_callback {
	int (*foreach)();		/* Return non-0 to stop getting
					 *  called */
	char *data;			/* Opaque pointer for use of callback
					 *   function */
};

/*
 * External NIS client function references. 
 */
extern int yp_bind __P ((char * __dom));
extern int _yp_dobind __P ((char *__dom, struct dom_binding **__ypdb));
extern void yp_unbind __P ((char * __dom));
extern int yp_get_default_domain  __P ((char ** __domp));
extern int yp_first  __P ((char * __indomain, char *__inmap, char ** __outkey,
			int * __outkeylen, char **outval, int * __outvallen));
extern int yp_next __P ((char * __indomain, char *__inmap, char * __inkey,
			int __inkeylen, char **__outkey, int *__outkeylen,
			char **__outval, int *__outvallen));
extern int yp_master __P ((char * __indomain, char * __inmap, char **__outname));
extern int yp_order __P ((char *__indomain, char * __inmap, int *outorder));
extern int yp_all __P ((char *__indomain, char * __inmap,
			struct ypall_callback *__incallback));
extern int yp_match __P ((char * __indomain, char *__inmap, char *__inkey,
		int __inkeylen, char **__outval, int *__outvallen));
extern char *yperr_string __P ((int __incode));
extern int ypprot_err __P ((unsigned int __incode));

extern int yp_maplist __P ((char * __indomain, struct ypmaplist **__outmaplist));
extern int _yp_check __P ((char ** __domp));

/*
 * Global NIS data structures
 */

__END_DECLS 

#endif	/* _RPCSVC_YPCLNT_H */
