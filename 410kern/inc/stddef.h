#ifndef _STDDEF_H_
#define _STDDEF_H_

#ifndef NULL
#define NULL ((void *)0)
#endif

#define offsetof(stype,field)	((size_t)(&((stype*)NULL)->field))

#endif /* _STDDEF_H_ */
