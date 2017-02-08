#ifndef _GETOPT_H_
#define _GETOPT_H_

#include <features.h>

__BEGIN_DECLS

extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

struct option
{
  __const char *name;
  int has_arg;
  int *flag;
  int val;
};

/* Names for the values of the `has_arg' field of `struct option'.  */

enum _argtype
{
  no_argument,
  required_argument,
  optional_argument
};

extern int getopt __P ((int __argc, char *__const *__argv, 
			__const char *__shortopts));
extern int getopt_long __P ((int __argc, char *__const *__argv,
			__const char *__shortopts,
		        __const struct option *__longopts, int *__longind));
extern int getopt_long_only __P ((int __argc, char *__const *__argv,
			     __const char *__shortopts,
		             __const struct option *__longopts,
			     int *__longind));

/* Internal only.  Users should not call this directly.  */
extern int _getopt_internal __P ((int __argc, char *__const *__argv,
			     __const char *__shortopts,
		             __const struct option *__longopts,
			     int *__longind, int __long_only));

__END_DECLS

#endif /* _GETOPT_H_ */
