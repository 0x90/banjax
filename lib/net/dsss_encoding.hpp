/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 Steve Glass
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

#ifndef NET_DSSS_ENCODING_HPP
#define NET_DSSS_ENCODING_HPP

#include <iosfwd>
#include <stdint.h>

namespace net {

   /**
    * dsss_encoding is a specification class that defines the timing
    * characteristics for the IEEE 802.11b DSSS channel encoding.
    */
   class dsss_encoding : public boost::noncopyable {
   public:

      /**
       * Default constructor for dsss_encoding.
       */
      dsss_encoding();

      /**
       * (Virtual) dsss_encoding destructor.
       */
      virtual ~dsss_encoding();

      /**
       * Returns the Short Inter-Frame Spacing (SIFS) time for the DSSS encoding.
       *
       * \return A uint16_t specifying the SIFS time.
       */
      virtual uint16_t SIFS() const;

      /**
       * Return the slot time used for this dsss_encoding.
       */
      virtual uint16_t slot_time() const;

      /**
       * Return the airtime (in microsbool ignored, uint16_t frame_sz, uint16_t rate_Kbseconds) that it would take to
       * send a frame of the given size using the DSSS encoding.
       *
       * \param frame_sz The size of the frame in octets.
       * \param rate_kbs The data rate in units of 1Kb/s.
       * \param has_short_preamble  true if short preamble is used; otherwise false.
       * \throws invalid_argument_exception When rate_Kbs is not permitted using this encoding.
       */
      virtual uint16_t txtime(uint16_t frame_sz, uint32_t rate_Kbs, bool has_short_preamble) const;

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      void write(std::ostream& os) const;

   };

}

#endif // NET_DSSS_ENCODING_HPP
