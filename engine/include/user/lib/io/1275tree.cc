/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	lib/io/1275tree.cc
 * Description:	Support for the canonical representation of the
 *              Open Firmware device tree created by the boot loader.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: 1275tree.cc,v 1.1 2004/01/16 11:23:56 joshua Exp $
 *
 ***************************************************************************/

#include <config.h>

#if defined(CONFIG_COMPORT)
#include "1275tree.h"
#include "lib.h"

int of1275_device_t::get_depth()
{
    int depth = 0;
    char *c = this->name;

    while( *c )
    {
	if( *c == '/' )
	    depth++;
	c++;
    }
    return depth;
}

bool of1275_device_t::get_prop( const char *prop_name, char **data, L4_Word_t *data_len)
{
    of1275_item_t *item_name, *item_data;

    item_name = this->item_first();
    item_data = item_name->next();

    for( L4_Word_t i = 0; i < this->get_prop_count(); i++ )
    {
	if( !strcmp(item_name->data, prop_name) )
	{
	    *data = item_data->data;
	    *data_len = item_data->len;
	    return true;
	}
	item_name = item_data->next();
	item_data = item_name->next();
    }

    return false;
}

bool of1275_device_t::get_prop( L4_Word_t index, 
	char **prop_name, char **data, L4_Word_t *data_len )
{
    of1275_item_t *item_name, *item_data;

    if( index >= this->get_prop_count() )
	return false;

    item_name = this->item_first();
    item_data = item_name->next();

    for( L4_Word_t i = 0; i < index; i++ )
    {
	item_name = item_data->next();
	item_data = item_name->next();
    }

    *prop_name = item_name->data;
    *data = item_data->data;
    *data_len = item_data->len;
    return true;
}

of1275_device_t * of1275_tree_t::find( const char *name )
{
    of1275_device_t *dev = this->first();
    if( !dev )
        return nullptr;

    while( dev->is_valid() )
    {
	if( !strcmp(dev->get_name(), name) )
	    return dev;
	dev = dev->next();
    }

    return nullptr;
}

of1275_device_t * of1275_tree_t::find_handle( L4_Word_t handle )
{
    of1275_device_t *dev = this->first();
    if( !dev )
        return nullptr;

    while( dev->is_valid() )
    {
	if( dev->get_handle() == handle )
	    return dev;
	dev = dev->next();
    }

    return nullptr;
}

of1275_device_t * of1275_tree_t::get_parent( of1275_device_t *dev )
{
    char *slash = nullptr;
    int cnt, depth;

    if( !dev || !this->first() )
        return nullptr;

    // Do we have any parents?
    depth = dev->get_depth();
    if( depth <= 1 )
        return nullptr;

    // Locate the last slash in the name.
    for( char *c = dev->get_name(); *c; c++ )
	if( *c == '/' )
	    slash = c;
    if( slash == nullptr )
        return nullptr;

    // Count the offset of the last slash.
    cnt = 0;
    for( char *c = dev->get_name(); c != slash; c++ )
	cnt++;

    // Search for the parent node.
    of1275_device_t *parent = this->first();
    while( parent->is_valid() )
    {
	if( !strncmp(parent->get_name(), dev->get_name(), cnt) )
	    if( parent->get_depth() == (depth-1) )
		return parent;
	parent = parent->next();
    }

    return nullptr;
}

of1275_device_t * of1275_tree_t::find_device_type( const char *device_type )
{
    of1275_device_t *dev;
    L4_Word_t len;
    char *type;

    dev = this->first();
    if( !dev )
        return nullptr;

    while( dev->is_valid() )
    {
	if( dev->get_prop("device_type", &type, &len) )
	    if( !strcmp(type, device_type) )
		return dev;
	dev = dev->next();
    }

    return nullptr;
}

#endif	/* !CONFIG_COMPORT */
