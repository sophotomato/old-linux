/*
 * net/dst.h	Protocol independent destination cache definitions.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#ifndef _NET_DST_H
#define _NET_DST_H

#include <linux/config.h>
#include <net/neighbour.h>

/*
 * 0 - no debugging messages
 * 1 - rare events and bugs (default)
 * 2 - trace mode.
 */
#ifdef  NO_ANK_FIX
#define RT_CACHE_DEBUG		0
#else
#define RT_CACHE_DEBUG		1
#endif

#define DST_GC_MIN	(1*HZ)
#define DST_GC_INC	(5*HZ)
#define DST_GC_MAX	(120*HZ)

struct sk_buff;

struct dst_entry
{
	struct dst_entry        *next;
	atomic_t		refcnt;		/* tree/hash references	*/
	atomic_t		use;		/* client references	*/
	struct device	        *dev;
	int			obsolete;
	unsigned long		lastuse;
	unsigned		mxlock;
	unsigned		window;
	unsigned		pmtu;
	unsigned		rtt;
	unsigned long		rate_last;	/* rate limiting for ICMP */
	unsigned long		rate_tokens;

	int			error;

	struct neighbour	*neighbour;
	struct hh_cache		*hh;

	int			(*input)(struct sk_buff*);
	int			(*output)(struct sk_buff*);

#ifdef CONFIG_NET_CLS_ROUTE
	__u32			tclassid;
#endif

	struct  dst_ops	        *ops;
		
	char			info[0];
};


struct dst_ops
{
	unsigned short		family;
	unsigned short		protocol;
	unsigned		gc_thresh;

	int			(*gc)(void);
	struct dst_entry *	(*check)(struct dst_entry *, __u32 cookie);
	struct dst_entry *	(*reroute)(struct dst_entry *,
					   struct sk_buff *);
	void			(*destroy)(struct dst_entry *);
	struct dst_entry *	(*negative_advice)(struct dst_entry *);
	void			(*link_failure)(struct sk_buff *);

	atomic_t		entries;
};

#ifdef __KERNEL__

extern struct dst_entry * dst_garbage_list;
extern atomic_t	dst_total;

extern __inline__
struct dst_entry * dst_clone(struct dst_entry * dst)
{
	if (dst)
		atomic_inc(&dst->use);
	return dst;
}

extern __inline__
void dst_release(struct dst_entry * dst)
{
	if (dst)
		atomic_dec(&dst->use);
}

extern __inline__
struct dst_entry * dst_check(struct dst_entry ** dst_p, u32 cookie)
{
	struct dst_entry * dst = *dst_p;
	if (dst && dst->obsolete)
		dst = dst->ops->check(dst, cookie);
	return (*dst_p = dst);
}

extern __inline__
struct dst_entry * dst_reroute(struct dst_entry ** dst_p, struct sk_buff *skb)
{
	struct dst_entry * dst = *dst_p;
	if (dst && dst->obsolete)
		dst = dst->ops->reroute(dst, skb);
	return (*dst_p = dst);
}


extern void * dst_alloc(int size, struct dst_ops * ops);
extern void __dst_free(struct dst_entry * dst);
extern void dst_destroy(struct dst_entry * dst);

extern __inline__
void dst_free(struct dst_entry * dst)
{
	if (dst->obsolete > 1)
		return;
	if (!atomic_read(&dst->use)) {
		dst_destroy(dst);
		return;
	}
	__dst_free(dst);
}

extern __inline__ void dst_confirm(struct dst_entry *dst)
{
	if (dst)
		neigh_confirm(dst->neighbour);
}

extern __inline__ void dst_negative_advice(struct dst_entry **dst_p)
{
	struct dst_entry * dst = *dst_p;
	if (dst && dst->ops->negative_advice)
		*dst_p = dst->ops->negative_advice(dst);
}

extern __inline__ void dst_link_failure(struct sk_buff *skb)
{
	struct dst_entry * dst = skb->dst;
	if (dst && dst->ops && dst->ops->link_failure)
		dst->ops->link_failure(skb);
}
#endif

#endif /* _NET_DST_H */
