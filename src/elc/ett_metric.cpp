/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#include <ett_metric.hpp>

#include <iostream>
#include <iomanip>


using namespace net;
using namespace std;
using metrics::ett_metric;

ett_metric::ett_metric() :
   metric()
{
}

ett_metric::ett_metric(const ett_metric& other) :
   metric(other)
{
}

ett_metric&
ett_metric::operator=(const ett_metric& other)
{
   if(&other != this) {
      metric::operator=(other);
   }
   return *this;
}

ett_metric::~ett_metric()
{
}

void
ett_metric::add(buffer_sptr b)
{
}

ett_metric*
ett_metric::clone() const
{
   return new ett_metric(*this);
}

void
ett_metric::compute(uint32_t delta_us) const
{
}

void
ett_metric::reset()
{
}

void
ett_metric::write(ostream& os) const
{
   os << "ETT: ToDo";
}
