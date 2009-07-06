#ifndef LINUX_26_27_COMPAT_H
#define LINUX_26_27_COMPAT_H

#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/compat_autoconf.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))

#include <linux/list.h>
#include <linux/rfkill.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>

#include <asm-generic/bug.h>

bool pci_pme_capable(struct pci_dev *dev, pci_power_t state);

/*
 * The net_device has a spin_lock on newer kernels, on older kernels we're out of luck
 */
#define netif_addr_lock_bh
#define netif_addr_unlock_bh

/*
 * To port this properly we'd have to port warn_slowpath_null(),
 * which I'm lazy to do so just do a regular print for now. If you
 * want to port this read kernel/panic.c
 */
#define __WARN_printf(arg...)   do { printk(arg); __WARN(); } while (0)

/* This is ported directly as-is on newer kernels */
#ifndef WARN
#define WARN(condition, format...) ({					\
	int __ret_warn_on = !!(condition);				\
	if (unlikely(__ret_warn_on))					\
		__WARN_printf(format);					\
	unlikely(__ret_warn_on);					\
})
#endif

/* On 2.6.27 a second argument was added, on older kernels we ignore it */
#define dma_mapping_error(pdev, dma_addr) dma_mapping_error(dma_addr)
#define pci_dma_mapping_error(pdev, dma_addr) dma_mapping_error(pdev, dma_addr)

/* This is from include/linux/rfkill.h */
#define RFKILL_STATE_SOFT_BLOCKED	RFKILL_STATE_OFF
#define RFKILL_STATE_UNBLOCKED		RFKILL_STATE_ON
/* This one is new */
#define RFKILL_STATE_HARD_BLOCKED	2

/* This is from include/linux/ieee80211.h */
#define IEEE80211_HT_CAP_DSSSCCK40		0x1000

/* New link list changes added as of 2.6.27, needed for ath9k */

static inline void __list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}


/* __list_splice as re-implemented on 2.6.27, we backport it */
static inline void __compat_list_splice_new_27(const struct list_head *list,
				 struct list_head *prev,
				 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice_tail(struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__compat_list_splice_new_27(list, head->prev, head);
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void list_splice_tail_init(struct list_head *list,
					 struct list_head *head)
{
	if (!list_empty(list)) {
		__compat_list_splice_new_27(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

extern unsigned int mmc_align_data_size(struct mmc_card *, unsigned int);
extern unsigned int sdio_align_size(struct sdio_func *func, unsigned int sz);

#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)) */

#endif /* LINUX_26_27_COMPAT_H */
