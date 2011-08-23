/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#include <metric_group.hpp>
#include <util/exceptions.hpp>

#include <iostream>
#include <iomanip>

using namespace std;
using metrics::metric_group;
using net::buffer_sptr;

metric_group::metric_group()
{
}

metric_group::metric_group(const metric_group& other) :
   metrics_(other.metrics_)
{
}

metric_group&
metric_group::operator=(const metric_group& other)
{
   if(&other != this) {
      metrics_ = other.metrics_;
   }
   return *this;
}

metric_group::~metric_group()
{
}

void
metric_group::push_back(metric_sptr m)
{
   metrics_.push_back(m);
}

void
metric_group::add(buffer_sptr b)
{
   for(metric_list::iterator i(metrics_.begin()); i != metrics_.end(); ++i) {
      (*i)->add(b);
   }
}

metric_group*
metric_group::clone() const
{
   return new metric_group(*this);
}

void
metric_group::compute(uint32_t delta_us)
{
   for(metric_list::iterator i(metrics_.begin()); i != metrics_.end(); ++i) {
      (*i)->compute(delta_us);
   }
}

void
metric_group::write(ostream& os) const
{
   metric_list::const_iterator i(metrics_.begin());
   if(i != metrics_.end()) {
      os << **i;
      while(++i != metrics_.end()) {
         os << ", " << **i;
      }
   }
}
