/**
 * @file
 *
 * @brief
 *
 * @date 14.09.2011
 * @author Anton Bondarev
 */

#ifndef FILE_DESC_H_
#define FILE_DESC_H_

#include <lib/list.h>
#include <fs/node.h>

struct file_desc {
	//struct list_head;
	struct node *node;
	struct file_operations *ops;
	int cursor;
	int ungetsym;
};

extern struct file_desc *file_desc_alloc(void);

extern void file_desc_free(struct file_desc *desc);

#endif /* FILE_DESC_H_ */
