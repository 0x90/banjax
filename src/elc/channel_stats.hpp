/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_CHANNEL_STATS_HPP
#define METRICS_CHANNEL_STATS_HPP

#include <abstract_metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * channel_stats measures IP payload goodput from the iper
    * program. This is the "gold standard" in that it tells us exactly
    * what the upper layer achieved in goodput. A quirk is that we
    * report the goodput using the MAC layer figure (in octets/s) so
    * that other wireless metrics do not have to account of LLC/IP/UDP
    * headers when they estimate link capacity.
    */
   class channel_stats : public abstract_metric {
   public:

      /**
       * channel_stats constructor.
       */
      channel_stats();

      /**
       * channel_stats copy constuctor.
       *
       * \param other The other channel_stats to initialize from.
       */
      channel_stats(const channel_stats& other);

      /**
       * channel_stats assignment operator.
       *
       * \param other The other channel_stats to assign from.
       * \return A reference to this channel_stats.
       */
      channel_stats& operator=(const channel_stats& other);

      /**
       * channel_stats destructor.
       */
     virtual ~channel_stats();

      /**
       * Add a frame to the channel_stats and update the channel_stats statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this channel_stats
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new channel_stats instance.
       */
      virtual channel_stats *clone() const;

      /**
       * Compute and return the ELC metric.
       *
       * \return A double specifying the metric value.
       */
      virtual double metric() const;

      /**
       * Resets this metric to its initial state.
       */
      virtual void reset();

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      virtual void write(std::ostream& os) const;

   private:

      /**
       * Number of MAC layer octets successfully sent in last second.
       */
      uint_least32_t frame_octets_;
      /**
       * Number of iperf octets transmitted successfully in last second.
       */
      uint_least32_t packet_octets_;

      /**
       * Total number of iperf octets transmitted successfully.
       */
      uint_least32_t packet_octets_total_;

      /**
       * Number of 802.11 control frames
       */
      uint_least32_t ctrl_;

      /**
       * Number of 802.11 data frames
       */
      uint_least32_t data_;

      /**
       * Number of 802.11 management frames
       */
      uint_least32_t mgmt_;

      /**
       * Count of iperf UDP packets.
       */
      uint_least32_t iperf_;

      /**
       * Count of IP packets for non-iperf traffic.
       */
      uint_least32_t other_;

      /**
       * Estimate of time taken for contention.
       */
      uint_least32_t t_contention_;

      /**
       * Time taken by 802.11 control frames
       */
      uint_least32_t t_ctrl_;

      /**
       * Time taken by 802.11 data frames.
       */
      uint_least32_t t_data_;

      /**
       * Time taken by 802.11 mgmt frames.
       */
      uint_least32_t t_mgmt_;

      /**
       * Time taken by interframe spacing.
       */
      uint_least32_t t_ifs_;

   };

}

#endif // METRICS_CHANNEL_STATS_HPP
