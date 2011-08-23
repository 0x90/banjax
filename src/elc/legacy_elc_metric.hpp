/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_LEGACY_ELC_METRIC_HPP
#define METRICS_LEGACY_ELC_METRIC_HPP

#include <abstract_metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * legacy_elc_metric is the ELC metric from Jono's original paper.
    */
   class legacy_elc_metric : public abstract_metric {
   public:

      /**
       * legacy_elc_metric constructor.
       *
       * \param enc The encoding used.
       */
      explicit legacy_elc_metric(net::encoding_sptr enc);

      /**
       * legacy_elc_metric copy constuctor.
       *
       * \param other The other legacy_elc_metric to initialize from.
       */
      legacy_elc_metric(const legacy_elc_metric& other);

      /**
       * legacy_elc_metric assignment operator.
       *
       * \param other The other legacy_elc_metric to assign from.
       * \return A reference to this legacy_elc_metric.
       */
      legacy_elc_metric& operator=(const legacy_elc_metric& other);

      /**
       * legacy_elc_metric destructor.
       */
      virtual ~legacy_elc_metric();

      /**
       * Add a frame to the legacy_elc_metric and update the legacy_elc_metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this legacy_elc_metric
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new legacy_elc_metric instance.
       */
      virtual legacy_elc_metric *clone() const;

      /**
       * Compute the metric and reset the internal state.
       *
       * \param delta_us The time (in microseconds) over which to compute the metric.
       */
      virtual void compute(uint32_t delta_us);

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      virtual void write(std::ostream& os) const;

   private:

      /**
       * Scan the standard rateset of the default encoding and return
       * the value which has the smallest difference to r.
       *
       * \param r The rate to find.
       * \return The value in rates which is closest to r.
       */
      uint32_t closest_rate(uint32_t r) const;

      /**
       * Compute the time it would take to successfully send a frame
       * of the given size at the specified rate. The time includes
       * the contention, RTS/CTS, interframe spacing and
       * acknowledgment.
       *
       * \param rate_Kbs The rate in units of Kbs.
       * \param frame_sz The size of the frame in octets.
       * \return The time taken (in microseconds).
       */
      uint32_t successful_tx_time(uint32_t rate_Kbs, uint16_t packet_sz) const;

   private:

      /**
       * The encoding used to compute the metric.
       */
      net::encoding_sptr enc_;

      /**
       * The total number of frame transmission attempts.
       */
      uint32_t frames_;

      /**
       * The total number of successfully delivered packets.
       */
      uint32_t packets_;

      /**
       * The total number of IP payload octets successfully delivered.
       */
      uint_least32_t packet_octets_;

      /**
       * Sum of the data rates used to send packets (used to compute average).
       */
      uint_least32_t rates_Kbs_sum_;

      /**
       * The current value of this ELC metric.
       */
      double elc_;

   };

}

#endif // METRICS_LEGACY_ELC_METRIC_HPP
