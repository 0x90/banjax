/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_AIRTIME_METRIC_ACTUAL_HPP
#define METRICS_AIRTIME_METRIC_ACTUAL_HPP

#include <abstract_metric.hpp>
#include <net/encoding.hpp>
#include <map>

namespace metrics {

   /**
    * airtime_metric_actual is the average of the actual metric values
    * seen during the measurement period. This requires that the
    * kernel support the NICTA vendor extension to radiotap.
    */
   class airtime_metric_actual : public abstract_metric {
   public:

      /**
       * airtime_metric_actual constructor.
       *
       * \param enc A non-null pointer to the encoding.
       * \param rts_cts_threshold Use RTS/CTS when rts_cts_threshold <= test frame size
       */
      explicit airtime_metric_actual();

      /**
       * airtime_metric_actual copy constuctor.
       *
       * \param other The other airtime_metric_actual to initialize from.
       */
      airtime_metric_actual(const airtime_metric_actual& other);

      /**
       * airtime_metric_actual assignment operator.
       *
       * \param other The other airtime_metric_actual to assign from.
       * \return A reference to this airtime_metric_actual.
       */
      airtime_metric_actual& operator=(const airtime_metric_actual& other);

      /**
       * airtime_metric_actual destructor.
       */
     virtual ~airtime_metric_actual();

      /**
       * Add a frame to the airtime_metric_actual and update the airtime_metric_actual statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this airtime_metric_actual
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A pointer to a new airtime_metric_actual instance.
       */
      virtual airtime_metric_actual *clone() const;

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
       * The sum of the metric values.
       */
      uint_least32_t airtime_sum_;

      /**
       * Count of packets attempted.
       */
      uint32_t packets_;

      /**
       * The average metric value.
       */
      double metric_;

   };

}

#endif // METRICS_AIRTIME_METRIC_ACTUAL_HPP
