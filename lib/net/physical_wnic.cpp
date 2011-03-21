/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2010 Steve Glass
 * 
 * This file is part of banjax.
 * 
 * Banjax is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * Banjax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 */

#include <net/buffer.hpp>
#include <net/physical_wnic.hpp>
#include <util/error_msg.hpp>
#include <util/syscall_error.hpp>

#include <iomanip>
#include <net/if_arp.h>
#include <sstream>
#include <stdexcept>

using namespace net;
using namespace std;
using boost::shared_ptr;
using util::error_msg;

physical_wnic::~physical_wnic()
{
}

eui_48
physical_wnic::addr() const
{
   return addr_;
}

void
physical_wnic::addr(const eui_48& addr)
{
   addr_ = addr;
}

channel
physical_wnic::current_channel() const
{
   return chan_;
}

void
physical_wnic::current_channel(const channel& chan)
{
   chan_ = chan;
}

/**
void
physical_wnic::filter(const string& filt_spec)
{
}

void
physical_wnic::filter(const sock_fprog *filt_prog)
{
}
**/

buffer_sptr
physical_wnic::read()
{
   buffer_sptr null;
   return null;
}

void
physical_wnic::write(const buffer& b)
{
}

physical_wnic::physical_wnic(const string& dev_name) :
   dev_name_(dev_name),
   chan_(1),
   addr_("02:00:00:00:00:01")
{
}
