/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_LEGACY_ELC_METRIC_HPP
#define METRICS_LEGACY_ELC_METRIC_HPP

#include <metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * legacy_elc_metric is the ELC metric from Jono's original paper.
    */
   class legacy_elc_metric : public metric {
   public:

      /**
       * legacy_elc_metric constructor.
       */
      legacy_elc_metric();

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
      uint32_t rates_Kbs_;

   };

}

#endif // METRICS_LEGACY_ELC_METRIC_HPP
