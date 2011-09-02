/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_RESIDUAL_HPP
#define METRICS_RESIDUAL_HPP

#include <metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * residual measures the channel idle/busy fraction and then
    * applies that to the metric passed to it in the constructor.
    */
   class residual : public metric {
   public:

      /**
       * residual constructor.
       *
       * \param name The name used for this metric when printing.
       * \param m A non-null pointer to a metric.
       */
      explicit residual(std::string name, metric_sptr m);

      /**
       * residual copy constuctor.
       *
       * \param other The other residual to initialize from.
       */
      residual(const residual& other);

      /**
       * residual assignment operator.
       *
       * \param other The other residual to assign from.
       * \return A reference to this residual.
       */
      residual& operator=(const residual& other);

      /**
       * residual destructor.
       */
     virtual ~residual();

      /**
       * Add a frame to the residual and update the residual statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this residual
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new residual instance.
       */
      virtual residual *clone() const;

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
       * The name of this residual metric.
       */
      std::string name_;

      /**
       * The metric for which we're computing the residual value.
       */
      metric_sptr m_;

      /**
       * The number of microseconds the channel has used since compute() was last called.
       */
      uint_least32_t busy_time_;

      /**
       * The value of this metric.
       */
      double residual_;


   };

}

#endif // METRICS_RESIDUAL_HPP
