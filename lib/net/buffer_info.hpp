/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2010-2011 Steve Glass
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

#ifndef NET_BUFFER_INFO_HPP
#define NET_BUFFER_INFO_HPP

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <stdint.h>
#include <iosfwd>

namespace net {

   /**
    * Property label type.
    */
   typedef uint32_t property_t;

   /* Property labels (comment defines actual type).
    */
   const property_t TIMESTAMP1 = 0x01; // uint64_t
   const property_t TIMESTAMP2 = 0x02; // uint64_t
   const property_t RATE_Kbs   = 0x04; // uint32_t
   const property_t FREQ_MHz   = 0x08; // uint32_t
   const property_t SIGNAL_dBm = 0x10; // int8_t
   const property_t RETRIES    = 0x20; // uint8_t
   const property_t RXFLAGS    = 0x40; // uint64_t

   /**
    * Max number of properties currently used by buffer_info.
    */
   const size_t NOF_PROPS = 7;

   /**
    * Property value type.
    */
   typedef uint64_t value_t;

   /* RX flag bitmasks.
    */
   const value_t RXFLAGS_PREAMBLE_LONG  = 0x0001;
   const value_t RXFLAGS_PREAMBLE_SHORT = 0x0002;
   const value_t RXFLAGS_CODING_DSSS    = 0x0010;
   const value_t RXFLAGS_CODING_OFDM    = 0x0020;
   const value_t RXFLAGS_CODING_FHSS    = 0x0040;
   const value_t RXFLAGS_CODING_DYNAMIC = 0x0080;
   const value_t RXFLAGS_RATE_FULL      = 0x0100;
   const value_t RXFLAGS_RATE_HALF      = 0x0200;
   const value_t RXFLAGS_RATE_QUARTER   = 0x0400;
   const value_t RXFLAGS_BAD_FCS        = 0x1000;

   /**
    * buffer_info is a concrete, leaf class that provides meta
    * information for a buffer. All of the properties are optional and
    * user code must take care to check (using #has()) whether the
    * desired properties are available.
    */
   class buffer_info : public boost::noncopyable {
   public:
      /**
       * buffer_info default constructor.
       */
      buffer_info();

      /**
       * buffer_info destructor.
       */
      ~buffer_info();

      /**
       * Clears the specified properties from this buffer_info.
       *
       * \param props The properties to be cleared.
       */
      void clear(property_t props = ~0);

      /**
       * Return the value of the specified property. If the value is
       * not present in then an invalid_argument exception will be
       * raised.
       *
       * \param property_t The property to return.
       * \throws 
       */
      value_t get(property_t prop) const;

      /**
       * Test whether the specified properties are available. Note
       * that you can test for the presence of multiple properties by
       * combining them with the bitwise or operator ('|').
       *
       * \param props The properties to test.
       * \return bool true if the specified properties are present.
       */
      bool has(property_t props) const;

      /**
       * Sets the specified property to value.
       * 
       * \param prop The property to set.
       * \param value The new value for the property.
       */
      void set(property_t prop, value_t value);

      /**
       * Write this buffer info to an output stream.
       *
       * \param os A reference to the ostream to write to.
       */
      void write(std::ostream& os) const;

   private:

      /**
       * Return the bit number for the given property. prop must be
       * one of the legitimate properties or an invalid_argument
       * exception will be raised.
       *
       * \param prop The property.
       * \return The bit number for that property.
       */
      uint8_t bit(property_t prop) const;

   private:

      /**
       * Which properties are actually present?
       */
      property_t present_;

      /**
       * The properties held by this buffer_info object.
       */
      value_t props_[NOF_PROPS];

   };

   /**
    * operator to stream a buffer_info to an ostream.
    *
    * \param os The stream to write to.
    * \param info The buffer_info to be streamed.
    * \return A reference to the modified ostream.
    */
   std::ostream& operator<<(std::ostream& os, const buffer_info& info);

   /**
    * Alias for shared_ptr<buffer_info>.
    */
   typedef boost::shared_ptr<net::buffer_info> buffer_info_sptr;

   /**
    * Alias for shared_ptr<const buffer_info>.
    */
   typedef boost::shared_ptr<const net::buffer_info> const_buffer_info_sptr;

}

#endif // NET_BUFFER_INFO_HPP
