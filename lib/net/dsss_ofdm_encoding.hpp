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

#ifndef NET_DSSS_OFDM_ENCODING_HPP
#define NET_DSSS_OFDM_ENCODING_HPP

#include <net/encoding.hpp>

#include <iosfwd>
#include <stdint.h>

namespace net {

   /**
    * dsss_ofdm_encoding is a specification class that defines the
    * timing characteristics for the IEEE 802.11b/g DSSS/OFDM channel
    * encoding. Timing characteristics can be found in table 19-7 of
    * IEEE 802.11 (2007) and the TXTIME calculation in section
    * 19.8.3.3.
    */
   class dsss_ofdm_encoding : public encoding {
   public:

      /**
       * Return a pointer to an instance of this class.
       *
       * \return An encoding_sptr pointing to an dsss_ofdm_encoding instance.
       */
      static encoding_sptr get();

      /**
       * Return the value of CWMIN for this encoding.
       *
       * \return A uint16_t specifying the CWMIN value.
       */
      virtual uint16_t CWMIN() const;

      /**
       * Return the value of CWMAX for this encoding.
       *
       * \return A uint16_t specifying the CWMAX value.
       */
      virtual uint16_t CWMAX() const;

      /**
       * (Virtual) dsss_ofdm_encoding destructor.
       */
      virtual ~dsss_ofdm_encoding();

      /**
       * Returns the Short Inter-Frame Spacing (SIFS) time for the
       * OFDM encoding.
       *
       * \return A uint16_t specifying the SIFS time.
       */
      virtual uint16_t SIFS() const;

      /**
       * Return the slot time used for this dsss_ofdm_encoding.
       */
      virtual uint16_t slot_time() const;

      /**
       * Return the airtime (in microseconds) that it would take to
       * send a frame of the given size using the DSSS/OFDM
       * encoding. See section 19.8.3.3 of IEEE 802.11 (2007) for
       * details of the calculation.
       *
       * \param frame_sz The size of the frame in octets.
       * \param rate_kbs The data rate in units of 1Kb/s.
       * \param has_short_preamble true if short preamble is used; otherwise false.
       * \throws invalid_argument_exception When rate_Kbs is not supported using this encoding.
       */
      virtual uint16_t txtime(uint16_t frame_sz, uint32_t rate_Kbs, bool has_short_preamble) const;

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      virtual void write(std::ostream& os) const;

   private:

      /**
       * Default constructor for dsss_ofdm_encoding.
       */
      dsss_ofdm_encoding();

      /**
       * Number of data bits per symbol (NDBPS) lookup. This is
       * defined by IEEE 802.11-2007 table 17.3.
       *
       * \param rate_Kbs The data rate of the frame in units of 1Kb/s.
       * \return The number of data bits per symbol.
       * \throws invalid_argument When rate_Kbs is not permitted using this encoding.
       */
      uint16_t ndbps(uint32_t rate_Kbs) const;

   };

}

#endif // NET_DSSS_OFDM_ENCODING_HPP
