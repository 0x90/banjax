/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2010 Steve Glass
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

#ifndef NET_PHYSICAL_WNIC_HPP
#define NET_PHYSICAL_WNIC_HPP

#include <net/wnic.hpp>

namespace net {

   /**
    * Interface for physical wnic instances. A physical wnic has a
    * MAC address and operates on a specific radio channel as well as
    * supporting the read/write services of the wnic interface.
    */
   class physical_wnic : public wnic {
   public:

      /**
       * Virtual destrutor for physial_wnic instances.
       */
      virtual ~physical_wnic();

      /**
       * Return the address for this wnic instance.
       *
       * \return An eui_48 specifying the wnic's MAC address.
       */
      virtual eui_48 addr() const = 0;

      /**
       * Sets the address for this wnic instance.
       *
       * \param addr An eui_48 specifying the wnic's MAC address.
       */
      virtual void addr(const eui_48& addr) = 0;

      /**
       * Return the channel on which this wnic operates.
       *
       * \return A channel object.
       */
      virtual channel current_channel() const = 0;

      /**
       * Sets the channel on whih this wnic operates.
       *
       * \param chan A channel object specifying the new channel.
       */
      virtual void current_channel(const channel& chan) = 0;

   protected:

      /**
       * Default constructor for physical_wnic.
       */
      physical_wnic();

   };

}

#endif // NET_PHYSICAL_WNIC_HPP
