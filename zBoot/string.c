#include <linux/types.h>

int memcmp(const void *s1, const void *s2, size_t len)
{
  char diff;
  asm("repe; cmpsb; setnz %0"
      : "=qm" (diff), "+D" (s1), "+S" (s2), "+c" (len));
  return diff;
}
