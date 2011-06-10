/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2008-2011 Steve Glass
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

#ifndef NET_BUFFER_HPP
#define NET_BUFFER_HPP

#include <net/eui_48.hpp>
#include <net/buffer_info.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <stdint.h>
#include <iosfwd>

namespace net {

   /**
    * buffer is an interface to an in-memory buffer. Accesses to the
    * buffer contents are bounds-checked and an out_of_range exception
    * thrown whenever an access is attempted to memory outside the
    * buffer bounds.
    */
   class buffer : public boost::noncopyable {
   public:

      /**
       * (Virtual)  buffer destructor.
       */
      virtual ~buffer();

       /**
       * Return a pointer to the beginning of the buffer.
       *
       * \return A non-null pointer to the buffer memory.
       */
      virtual const uint8_t *data() const = 0;

      /**
       * Return the size of this buffer in octets.
       *
       * \return The size of the buffer.
       */
      virtual size_t data_size() const = 0;

      /**
       * Returns the buffer_info associated with this buffer. This is
       * the non-const accessor which allows callers to modify the
       * returned buffer_info.
       *
       * \return A (possibly null) buffer_info_sptr.
       */
      virtual buffer_info_sptr info() = 0;

      /**
       * Returns the buffer_info associated with this buffer. This is
       * the const accessor which does not allow callers to modify the
       * returned buffer_info.
       *
       * \return A const_buffer_info_sptr.
       */
      virtual const_buffer_info_sptr info() const = 0;

      /**
       * Read a MAC address (in eui_48 format) from the specified
       * position. The specified value must lie wholly within the
       * buffer or else an out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \return An eui_48 containing the address value at position i.
       */
      virtual eui_48 read_mac(size_t i) const = 0;

      /**
       * Return a pointer to the beginning of a block from
       * [begin,end). The specified block must lie wholly within the
       * buffer or else an out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \return A uint32_t containing the value at position i.
       */
      virtual const uint8_t *read_octets(size_t begin, size_t end) const = 0;

      /**
       * Read an octet from the buffer at the specified position. The
       * specified position must be wholly within the buffer or else
       * an out_of_bounds exception will be raised.
       *
       * \param i The offset from which to read.
       * \return A uint8_t containing the value at position i.
       */
      virtual uint8_t read_u8(size_t i) const = 0;

      /**
       * Read an uinsigned 16 bit value at the specified position. The
       * value is read in network (big-endian) format. The specified
       * value must be wholly within the buffer or else an
       * out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \return A uint16_t containing the value at position i.
       */
      virtual uint16_t read_u16(size_t i) const = 0;

      /**
       * Read an uinsigned 16 bit value at the specified position. The
       * value is read in host (little-endian) format. The specified
       * value must be wholly within the buffer or else an
       * out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \return A uint16_t containing the value at position i.
       */
      virtual uint16_t read_u16_le(size_t i) const = 0;

      /**
       * Read an uinsigned 32 bit value at the specified position. The
       * value is read in network (big-endian) format. The specified
       * value must be wholly within the buffer or else an
       * out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \return A uint32_t containing the value at position i.
       */
      virtual uint32_t read_u32(size_t i) const = 0;

      /**
       * Read an uinsigned 32 bit value at the specified position. The
       * value is read in host (little-endian) format. The specified
       * value must be wholly within the buffer or else an
       * out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \return A uint32_t containing the value at position i
       */
      virtual uint32_t read_u32_le(size_t i) const = 0;

      /**
       * Write a MAC address (in eui_48 format) at the specified
       * position. The specified value must lie wholly within the
       * buffer or else an out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \param mac The eui_48 value to write.
       */
      virtual void write_mac(size_t i, eui_48 mac) = 0;

      /**
       * Writes an octet string into the buffer at positions
       * [begin,end). The specified block must lie wholly within the
       * buffer or else an out_of_bounds exception will be raised.
       *
       * \param i The offset from which tor read.
       * \param octets A non-NULL pointer to octet string to write.
       */
      virtual void write_octets(size_t begin, size_t end, const uint8_t octets[]) = 0;

      /**
       * Write an octet into the buffer at the specified position. The
       * specified position must be wholly within the buffer or else
       * an out_of_bounds exception will be raised.
       *
       * \param i The offset from which to read.
       * \param u The uint8_t value to write.
       */
      virtual void write_u8(size_t i, uint8_t u) = 0;

      /**
       * Write a uint16_t into the buffer at the specified position in
       * network (big-endian) order. The specified position must be
       * wholly within the buffer or else an out_of_bounds exception
       * will be raised.
       *
       * \param i The offset from which to read.
       * \param u The uint16_t value to write.
       */
      virtual void write_u16(size_t i, uint16_t u) = 0;

      /**
       * Write a uint16_t into the buffer at the specified position in
       * host (little-endian) order. The specified position must be
       * wholly within the buffer or else an out_of_bounds exception
       * will be raised.
       *
       * \param i The offset from which to read.
       * \param u The uin16_t value to write.
       */
      virtual void write_u16_le(size_t i, uint16_t u) = 0;

      /**
       * Write a uint32_t into the buffer at the specified position in
       * network (big-endian) order. The specified position must be
       * wholly within the buffer or else an out_of_bounds exception
       * will be raised.
       *
       * \param i The offset from which to read.
       * \param u The uint32_t value to write.
       */
      virtual void write_u32(size_t i, uint32_t u) = 0;

      /**
       * Write a uint32_t into the buffer at the specified position in
       * host (little-endian) order. The specified position must be
       * wholly within the buffer or else an out_of_bounds exception
       * will be raised.
       *
       * \param i The offset from which to read.
       * \param u The uint32_t value to write.
       */
      virtual void write_u32_le(size_t i, uint32_t u) = 0;

      /**
       * Write this buffer in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      virtual void write(std::ostream& os) const;

   protected:

      /**
       * Default constructor for the buffer interface.
       */
      buffer();

   };

   /**
    * operator to stream a buffer to an ostream.
    *
    * \param os The stream to write to.
    * \param buf The buffer to be streamed.
    * \return A reference to the modified ostream.
    */
   std::ostream& operator<<(std::ostream& os, const buffer& buf);

   /**
    * Alias for shared_ptr<buffer>.
    */
   typedef boost::shared_ptr<buffer> buffer_sptr;

   /**
    * Alias for shared_ptr<const buffer>.
    */
   typedef boost::shared_ptr<const buffer> const_buffer_sptr;

}

#endif // NET_BUFFER_HPP
