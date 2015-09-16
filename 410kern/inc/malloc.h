#ifndef _410KERN_MALLOC_H_
#define _410KERN_MALLOC_H_

#include <malloc/malloc_internal.h>

/* Muahaha; P1 */

#define malloc(x) _malloc(x)
#define memalign(x) _memalign(x)
#define calloc(x,y) _calloc(x,y)
#define realloc(x,y) _realloc(x,y)
#define free(x) _free(x) 

#define smalloc(x) _smalloc(x)
#define smemalign(x,y) _smemalign(x,y)
#define sfree(x,y) _sfree(x,y)

#endif
