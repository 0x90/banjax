/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#include <link.hpp>

using namespace net;
using namespace std;
using metrics::link;

link::link(const eui_48& to, const eui_48& from, uint16_t rts_cts_threshold) :
   to_(to),
   from_(from),
   rts_cts_threshold_(rts_cts_threshold)
{
}

link::link(const link& other) :
   to_(other.to_),
   from_(other.from_),
   rts_cts_threshold_(other.rts_cts_threshold_)
{
}

metrics::link&
link::operator=(const link& other)
{
    if(this != &other) {
       to_ = other.to_;
       from_ = other.from_;
       rts_cts_threshold_ = other.rts_cts_threshold_;     
    }
    return *this;
}

link::~link()
{
}

bool
link::operator==(const link& other) const
{
   return to_ == other.to_ ;
}

bool
link::operator<(const link& other) const
{
   return to_ < other.to_;
}

size_t
link::hash() const
{
   return to_.hash();
}

void
link::write(ostream& os) const
{
/*
   os << "To: " << to_ << ", ";
   os << "From: " << from_ << ", ";
   os << "RTS/CTS: " << to_.rts_cts_threshold << endl;
*/
}

ostream&
metrics::operator<<(ostream& os, const link& addr)
{
   addr.write(os);
   return os;
}
