/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#include <metric_demux.hpp>
#include <dot11/frame.hpp>
#include <util/exceptions.hpp>

#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;
using metrics::metric_demux;
using dot11::frame;
using net::buffer_sptr;
using net::eui_48;

metric_demux::metric_demux(metric_sptr proto) :
   proto_(proto)
{
}

metric_demux::metric_demux(const metric_demux& other) :
   proto_(other.proto_),
   links_(other.links_)
{
}

metric_demux& 
metric_demux::operator=(const metric_demux& other)
{
   if(&other != this) {
      proto_ = other.proto_;
      links_ = other.links_;
   }
   return *this;
}

metric_demux::~metric_demux()
{
}

void
metric_demux::add(buffer_sptr b)
{
   // ToDo: add a test for special/broadcast addresses!?

   metric_sptr m;
   frame f(b);
   eui_48 ra(f.address1());
   linkmap::iterator i(links_.find(ra));
   if(links_.end() != i) {
      m = i->second;
   } else {
      eui_48 ta(f.address2());
      m = metric_sptr(proto_->clone());
      links_[ra] = m;
   }
   m->add(b);
}

metric_demux*
metric_demux::clone() const
{
   return new metric_demux(*this);
}

void
metric_demux::reset()
{
   for(linkmap::iterator i(links_.begin()); i != links_.end(); ++i) {
      (i->second)->reset();
   }
}

void
metric_demux::write(ostream& os) const
{
   for(linkmap::const_iterator i(links_.begin()); i != links_.end(); ++i) {
      cout << i->first << ", " << *(i->second) << endl;
   }
}
