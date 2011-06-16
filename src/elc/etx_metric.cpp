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

ett_metric::ett_metric()
{
}

ett_metric::ett_metric(const ett_metric& other)
{
}

ett_metric&
ett_metric::operator=(const ett_metric& other)
{
   if(&other != this) {
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

double
ett_metric::metric() const
{
   return 0.0;
}

void
ett_metric::reset()
{
}

void
ett_metric::write(ostream& os) const
{
   os << "ETT: " << metric();
}
