#ifndef JIFFIES_H
#define JIFFIES_H


#define time_after(a,b)					((long)(b) - (long)(a) < 0)
#define time_before(a,b)				time_after(b, a)

#define msecs_to_jiffies_2(msec)		((msec)/2)

#define msecs_to_jiffies_10(msec)		((msec)/10)
//#define msecs_to_jiffies_100(msec)       ((msec)/100)
#define jiffies_to_msecs(j)				((j)*10)

#define jiffies_to_msecs_10(j)			(((j)*625UL)/10000)

#define msecs_to_jiffies(msec)   ((msec)*1000/625)

#endif

