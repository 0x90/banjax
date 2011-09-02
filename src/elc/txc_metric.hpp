/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_TXC_METRIC_HPP
#define METRICS_TXC_METRIC_HPP

#include <abstract_metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * txc_metric reports the packet delivery ratio.
    */
   class txc_metric : public abstract_metric {
   public:

      /**
       * txc_metric default constructor.
       *
       * \param enc The encoding used.
       */
      txc_metric();

      /**
       * txc_metric copy constuctor.
       *
       * \param other The other txc_metric to initialize from.
       */
      txc_metric(const txc_metric& other);

      /**
       * txc_metric assignment operator.
       *
       * \param other The other txc_metric to assign from.
       * \return A reference to this txc_metric.
       */
      txc_metric& operator=(const txc_metric& other);

      /**
       * txc_metric destructor.
       */
     virtual ~txc_metric();

      /**
       * Add a frame to the txc_metric and update the txc_metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this txc_metric
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new txc_metric instance.
       */
      virtual txc_metric *clone() const;

      /**
       * Compute the metric.
       *
       * \param delta_us The time (in microseconds) over which to compute the metric.
       * \return The value of this metric as a double.
       */
      virtual double compute(uint32_t delta_us);

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
       * The total number of transmission attempts.
       */
      uint_least32_t packets_;

      /**
       * The total number of frame transmissions.
       */
      uint_least32_t frames_;

      /**
       * The frame count stash.
       */
      double frms_;

      /**
       * The packet count stash.
       */
      double pkts_;

      /**
       * The average TXC value.
       */
      double txc_;

   };

}

#endif // METRICS_TXC_METRIC_HPP
