/*
 *  Copyright (C) 2021 Jacob Adams
 *  Copyright (C) 1991-2002 Linus Torvalds
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

void
spl_migrate(int node)
{
	if (node >= nr_node_ids) {
		printk(KERN_WARNING "SPL: Can't migrate to node %d!\n", node);
		return;
	}
	printk("SPL: Attempting  to migrate task %s from %d to %d\n",
			curthread->comm, curnode, node);
	set_cpus_allowed_ptr(curthread, cpumask_of_node(node));
	kernel_migrate_pages(0, nr_node_ids, nodemask_of_node(curnode),
			nodemask_of_node(node));
	if (curnode != node)
	{
		printk("SPL: Failed to migrate task %s!\n", curthread->comm);
		dump_stack();
	}
}
EXPORT_SYMBOL(spl_migrate);

