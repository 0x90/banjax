/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_ETT_METRIC_HPP
#define METRICS_ETT_METRIC_HPP

#include <metric.hpp>

namespace metrics {

   /**
    * ett_metric is the new ELC metric.
    */
   class ett_metric : public metric {
   public:

      /**
       * ett_metric constructor.
       */
      explicit ett_metric();

      /**
       * ett_metric copy constuctor.
       *
       * \param other The other ett_metric to initialize from.
       */
      ett_metric(const ett_metric& other);

      /**
       * ett_metric assignment operator.
       *
       * \param other The other ett_metric to assign from.
       * \return A reference to this ett_metric.
       */
      ett_metric& operator=(const ett_metric& other);

      /**
       * ett_metric destructor.
       */
     virtual ~ett_metric();

      /**
       * Add a frame to the ett_metric and update the ett_metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this ett_metric
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new ett_metric instance.
       */
      virtual ett_metric *clone() const;

      /**
       * Compute the ELC metric.
       *
       * \param delta_us The time (in microseconds) over which to compute the metric.
       * \return The value of this metric as a double.
       */
      virtual double compute(uint32_t delta_us);

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

   };

}

#endif // METRICS_ETT_METRIC_HPP
