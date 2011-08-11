/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_TXC_METRIC_HPP
#define METRICS_PDR_METRIC_HPP

#include <abstract_metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * pdr_metric reports the packet delivery ratio.
    */
   class pdr_metric : public abstract_metric {
   public:

      /**
       * pdr_metric default constructor.
       *
       * \param enc The encoding used.
       */
      pdr_metric();

      /**
       * pdr_metric copy constuctor.
       *
       * \param other The other pdr_metric to initialize from.
       */
      pdr_metric(const pdr_metric& other);

      /**
       * pdr_metric assignment operator.
       *
       * \param other The other pdr_metric to assign from.
       * \return A reference to this pdr_metric.
       */
      pdr_metric& operator=(const pdr_metric& other);

      /**
       * pdr_metric destructor.
       */
     virtual ~pdr_metric();

      /**
       * Add a frame to the pdr_metric and update the pdr_metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this pdr_metric
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new pdr_metric instance.
       */
      virtual pdr_metric *clone() const;

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
       * The total number of attempted packet deliveries.
       */
      uint_least32_t attempts_;

      /**
       * The total number of successful packet deliveries.
       */
      uint_least32_t good_;

   };

}

#endif // METRICS_PDR_METRIC_HPP
