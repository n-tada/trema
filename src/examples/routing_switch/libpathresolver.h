/*
 * Path Resolver with dijkstra method
 *
 * Author: Shuji Ishii
 *
 * Copyright (C) 2008-2011 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef LIBPATHRESOLVER_H
#define LIBPATHRESOLVER_H


#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "libtopology.h"
#include "hash_table.h"
#include "doubly_linked_list.h"


typedef struct pathresolver_hop {
  uint64_t dpid;
  uint16_t in_port_no;
  uint16_t out_port_no;
} pathresolver_hop;


typedef void ( *resolve_path_callback )( void *user_data, dlist_element *hops );


bool resolve_path( uint64_t in_dpid, uint16_t in_port,
                   uint64_t out_dpid, uint16_t out_port,
                   void *user_data,
                   resolve_path_callback callback );
void free_hop_list( dlist_element *hops );


#endif	// LIBPATHRESOLVER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
