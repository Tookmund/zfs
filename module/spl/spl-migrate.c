/*
 *  Copyright (C) 1991-2002 Linus Torvalds
 *  Copyright (C) 2003,2004 Andi Kleen, SuSE Labs.
 *  Copyright (C) 2005 Christoph Lameter, Silicon Graphics, Inc.
 *  Copyright (C) 2021 Jacob Adams
 *
 *  This file is part of the SPL, Solaris Porting Layer.
 *  For details, see <http://zfsonlinux.org/>.
 *
 *  The SPL is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  The SPL is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with the SPL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Solaris Porting Layer (SPL) Process Migration
 */

#include <sys/thread.h>
#include <sys/migrate.h>

/*
 * Ripped straight from kernel_migrates_pages in linux/mm/mempolicy.c
 * Requires do_migrate_pages to be exported
 * Skips a bunch of security checks, because those functions
 * aren't exported
 */
static inline int
spl_migrate_pages(struct task_struct *task, int node)
{
	struct mm_struct *mm = NULL;
	int err;
	nodemask_t old = nodemask_of_node(curnode);
	nodemask_t new = nodemask_of_node(node);

	/* Find the mm_struct */
	rcu_read_lock();
	if (!task) {
		rcu_read_unlock();
		err = -ESRCH;
		goto out;
	}
	get_task_struct(task);

	err = -EINVAL;

	rcu_read_unlock();

	mm = get_task_mm(task);
	put_task_struct(task);

	if (!mm) {
		err = -EINVAL;
		goto out;
	}

	err = do_migrate_pages(mm, &old, &new, MPOL_MF_MOVE);

	mmput(mm);

out:
	return err;
}

void
spl_migrate(int node)
{
	if (node >= nr_node_ids || node < 0) {
		pr_warn("SPL: Can't migrate to node %d!\n", node);
		return;
	}
	printk("SPL: Attempting to migrate task %s from %d to %d\n",
			curthread->comm, curnode, node);
	set_cpus_allowed_ptr(curthread, cpumask_of_node(node));
	spl_migrate_pages(curthread, node);
	if (curnode != node) {
		pr_err(KERN_ERR "SPL: Failed to migrate task %s!\n", curthread->comm);
		dump_stack();
	}
}
EXPORT_SYMBOL(spl_migrate);

// get_mm_counter from include/linux/mm.h
#define MMCOUNTER(stat)	((unsigned long)atomic_long_read(&mm->rss_stat.count[stat]))

unsigned long
spl_get_proc_size(void)
{
	struct mm_struct *mm = get_task_mm(curthread);
	if (mm == NULL)
		return 0;
	// According to task_statm from fs/proc/task_mmu, this is resident memory
	return (MMCOUNTER(MM_FILEPAGES) + MMCOUNTER(MM_SHMEMPAGES)
		+ MMCOUNTER(MM_ANONPAGES)) * PAGE_SIZE;
}
EXPORT_SYMBOL(spl_get_proc_size);
