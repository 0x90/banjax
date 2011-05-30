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

#ifndef NET_ENCODING_HPP
#define NET_ENCODING_HPP

#include <iosfwd>
#include <stdint.h>

namespace net {

   /**
    * encoding is an interface that specifies the timing
    * characteristics of the IEEE 802.11 radio channel. Concrete
    * subclasses implement this class for 802.11a/g (OFDM), 802.11b/g
    * (DSSS/OFDM) and 802.11b (DSSS) encodings.
    */
   class encoding : public boost::noncopyable {
   public:

      /**
       * (Virtual) encoding destructor.
       */
      virtual ~encoding();

      /**
       * Returns the DCF Inter-Frame Space (DIFS) time under this
       * encoding. By default a DIFS = 2 * slot_time + SIFS.
       */
      virtual uint16_t DIFS() const;

      /**
       * Returns the Short Inter-Frame Spacing (SIFS) time for this encoding.
       *
       * \return A uint16_t specifying the SIFS time.
       */
      virtual uint16_t SIFS() const = 0;

      /**
       * Return the slot time used for this encoding.
       */
      virtual uint16_t slot_time() const = 0;

      /**
       * Return the airtime (in microseconds) that it would take to
       * send a frame of the given size using this encoding.
       *
       * \param has_short_preamble true if short preamble was used; otherwise false.
       * \param frame_sz The size of the frame in octets.
       * \param rate_kbs The data rate in units of 1Kb/s.
       * \throws invalid_argument_exception When either frame_sz is
       * zero or rate_Kbs is not permitted using this encoding.
       */
      uint16_t txtime(bool has_short_preamble, uint16_t frame_sz, uint16_t rate_Kbs) const;

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      void write(std::ostream& os) const = 0;

   private:

      /**
       * Default constructor for encoding.
       */
      encoding();

   };

   /**
    * operator to stream a encoding to an ostream.
    *
    * \param os The stream to write to.
    * \param e The encoding to be streamed.
    * \return A reference to the modified ostream.
    */
   inline std::ostream&
   operator<<(std::ostream& os, const encoding& e)
   {
      os << e;
      return os;
   }

   /**
    * Alias for shared_ptr<encoding>.
    */
   typedef boost::shared_ptr<encoding> encoding_sptr;

}

#endif // NET_ENCODING_HPP
