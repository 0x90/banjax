/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2013 NICTA
 *
 */

#ifndef METRICS_SATURATION_METRIC_HPP
#define METRICS_SATURATION_METRIC_HPP

#include <metrics/metric.hpp>

namespace metrics {

	/**
	 * saturation_metric computes the total time used by ACK frames.
	 */
	class saturation_metric : public metric {
	public:

		/**
		 * saturation_metric constructor.
		 *
		 * \param name The name this metric uses when writing results.
		 */
		saturation_metric(const std::string& name = "saturation");

		/**
		 * saturation_metric copy constuctor.
		 *
		 * \param other The other saturation_metric to initialize from.
		 */
		saturation_metric(const saturation_metric& other);

		/**
		 * saturation_metric assignment operator.
		 *
		 * \param other The other saturation_metric to assign from.
		 * \return A reference to this saturation_metric.
		 */
		saturation_metric& operator=(const saturation_metric& other);

		/**
		 * saturation_metric destructor.
		 */
	  virtual ~saturation_metric();

		/**
		 * Add a frame to the saturation_metric and update the saturation_metric statistics.
		 *
		 * \param b A shared_pointer to the buffer containing the frame.
		 */
		virtual void add(net::buffer_sptr b);

		/**
		 * Return a pointer to a clone (deep copy) of this saturation_metric
		 * instance. The clone is allocated on the heap using new and
		 * the caller is responsible for ensuring it is deleted.
		 *
		 * \return A pointer to a new saturation_metric instance.
		 */
		virtual saturation_metric *clone() const;

      /**
       * Compute the metric.
       *
       * \param time The 64 bit MAC time for the end of the time period.
       * \param delta_us The time (in microseconds)  since the start of the time period.
       * \return The value of this metric as a double.
       */
      virtual double compute(uint64_t time, uint32_t delta_us);

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
		 * Name of this metric.
		 */
		std::string name_;

      /**
       * Number of packets we couldn't process.
       */
      uint32_t bad_packets_;

      /**
       * Number of RX packets seen by this metric.
       */
      uint32_t rx_packets_;

      /**
       * Time (in microseconds) spent receiving stuff (e.g. ack frames).
       */
      uint32_t rx_time_;

      /**
       * Number of TX packets seen by this metric.
       */
      uint32_t tx_packets_;

      /**
       * Time (in microseconds) spent actually sending packets.
       */
      uint32_t tx_time_;

      /**
       * Have we computed a valid metric?
       */
      bool valid_;

		/**
		 * Value of this metric.
		 */
		double saturation_;

		/**
		 * Additional debugging output so we can "show our workings".
		 */
		std::string debug_;

	};

}

#endif // METRICS_SATURATION_METRIC_HPP
