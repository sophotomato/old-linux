/*
 * linux/fs/hfs/sysdep.c
 *
 * Copyright (C) 1996  Paul H. Hargrove
 * This file may be distributed under the terms of the GNU Public License.
 *
 * This file contains the code to do various system dependent things.
 *
 * "XXX" in a comment is a note to myself to consider changing something.
 *
 * In function preconditions the term "valid" applied to a pointer to
 * a structure means that the pointer is non-NULL and the structure it
 * points to has all fields initialized to consistent values.
 */

#include "hfs.h"
#include <linux/hfs_fs_sb.h>
#include <linux/hfs_fs_i.h>
#include <linux/hfs_fs.h>

static int hfs_hash_dentry(struct dentry *, struct qstr *);
static int hfs_compare_dentry(struct dentry *, struct qstr *, struct qstr *);
static void hfs_dentry_iput(struct dentry *, struct inode *);
struct dentry_operations hfs_dentry_operations =
{
	NULL,	                /* d_validate(struct dentry *) */
	hfs_hash_dentry,	/* d_hash */
	hfs_compare_dentry,    	/* d_compare */
	NULL,	                /* d_delete(struct dentry *) */
	NULL,                   /* d_release(struct dentry *) */
	hfs_dentry_iput         /* d_iput(struct dentry *, struct inode *) */
};

/*
 * hfs_buffer_get()
 *
 * Return a buffer for the 'block'th block of the media.
 * If ('read'==0) then the buffer is not read from disk.
 */
hfs_buffer hfs_buffer_get(hfs_sysmdb sys_mdb, int block, int read) {
	hfs_buffer tmp = HFS_BAD_BUFFER;

	if (read) {
		tmp = bread(sys_mdb->s_dev, block, HFS_SECTOR_SIZE);
	} else {
		tmp = getblk(sys_mdb->s_dev, block, HFS_SECTOR_SIZE);
		if (tmp) {
			mark_buffer_uptodate(tmp, 1);
		}
	}
	if (!tmp) {
		hfs_error("hfs_fs: unable to read block 0x%08x from dev %s\n",
			  block, hfs_mdb_name(sys_mdb));
	}

	return tmp;
}

/* dentry case-handling: just lowercase everything */

/* hfs_strhash now uses the same hashing function as the dcache. */
static int hfs_hash_dentry(struct dentry *dentry, struct qstr *this)
{
	if (this->len > HFS_NAMELEN)
	        return 0;
	
	this->hash = hfs_strhash(this->name, this->len);
	return 0;
}

/* return 1 on failure and 0 on success */
static int hfs_compare_dentry(struct dentry *dentry, struct qstr *a, 
			      struct qstr *b)
{
	if (a->len != b->len) return 1;

	if (a->len > HFS_NAMELEN)
	  return 1;

	return !hfs_streq(a->name, a->len, b->name, b->len);
}

static void hfs_dentry_iput(struct dentry *dentry, struct inode *inode)
{
	struct hfs_cat_entry *entry = HFS_I(inode)->entry;

	entry->sys_entry[HFS_ITYPE_TO_INT(HFS_ITYPE(inode->i_ino))] = NULL;
	iput(inode);
}
