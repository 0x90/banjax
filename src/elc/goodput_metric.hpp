/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_GOODPUT_METRIC_HPP
#define METRICS_GOODPUT_METRIC_HPP

#include <metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * goodput_metric measures IP payload goodput from the iper
    * program. This is the "gold standard" in that it tells us exactly
    * what the upper layer achieved in goodput. A quirk is that we
    * report the goodput using the MAC layer figure (in octets/s) so
    * that other wireless metrics do not have to account of LLC/IP/UDP
    * headers when they estimate link capacity.
    */
   class goodput_metric : public metric {
   public:

      /**
       * goodput_metric constructor.
       */
      goodput_metric();

      /**
       * goodput_metric copy constuctor.
       *
       * \param other The other goodput_metric to initialize from.
       */
      goodput_metric(const goodput_metric& other);

      /**
       * goodput_metric assignment operator.
       *
       * \param other The other goodput_metric to assign from.
       * \return A reference to this goodput_metric.
       */
      goodput_metric& operator=(const goodput_metric& other);

      /**
       * goodput_metric destructor.
       */
     virtual ~goodput_metric();

      /**
       * Add a frame to the goodput_metric and update the goodput_metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this goodput_metric
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new goodput_metric instance.
       */
      virtual goodput_metric *clone() const;

      /**
       * Compute the metric.
       *
       * \param delta_us The time (in microseconds) over which to compute the metric.
       */
      virtual void compute(uint32_t delta_us);

      /**
       * Reset the internal state of the metric.
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
      uint32_t frame_octets_;

      /**
       * Number of iperf octets transmitted successfully in last second.
       */
      uint32_t packet_octets_;

      /**
       * iperf goodput value.
       */
      double iperf_goodput_;

      /**
       * MAC layer goodput value.
       */
      double mac_goodput_;

   };

}

#endif // METRICS_GOODPUT_METRIC_HPP
