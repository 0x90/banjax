/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_UTILIZATION_METRIC_HPP
#define METRICS_UTILIZATION_METRIC_HPP

#include <metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * utilization_metric measures IP payload throughput.
    */
   class utilization_metric : public metric {
   public:

      /**
       * utilization_metric constructor.
       */
      utilization_metric();

      /**
       * utilization_metric copy constuctor.
       *
       * \param other The other utilization_metric to initialize from.
       */
      utilization_metric(const utilization_metric& other);

      /**
       * utilization_metric assignment operator.
       *
       * \param other The other utilization_metric to assign from.
       * \return A reference to this utilization_metric.
       */
      utilization_metric& operator=(const utilization_metric& other);

      /**
       * utilization_metric destructor.
       */
     virtual ~utilization_metric();

      /**
       * Add a frame to the utilization_metric and update the utilization_metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this utilization_metric
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new utilization_metric instance.
       */
      virtual utilization_metric *clone() const;

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
       * Number of payload octets transmitted in last second.
       */
      uint32_t packet_octets_;

   };

}

#endif // METRICS_UTILIZATION_METRIC_HPP
