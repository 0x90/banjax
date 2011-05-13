/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_ELC_LINK_METRIC_HPP
#define METRICS_ELC_LINK_METRIC_HPP

#include <link_metric.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * elc_link_metric represents an ELC link metric between two stations.
    */
   class elc_link_metric : public link_metric {
   public:

      /**
       * elc_link_metric constructor. Creates a new uni-directional elc_link_metric between
       * "from" and "to".
       *
       * \param to The address of the receiving side of this elc_link_metric.
       * \param from The address of the sending side of this elc_link_metric.
       * \param rts_cts_threshold Use RTS/CTS when rts_cts_threshold <= frame size
       */
      explicit elc_link_metric(const net::eui_48& to, const net::eui_48& from, uint16_t rts_cts_threshold);

      /**
       * elc_link_metric destructor.
       */
     virtual ~elc_link_metric();

      /**
       * Add a frame to the elc_link_metric and update the elc_link_metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Compute and return the link metric.
       *
       * \return A double specifying the link metric value.
       */
      virtual double metric() const;

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      virtual void write(std::ostream& os) const;

   private:

      /**
       * Compute the time taken to successfully send packet b.
       *
       * \param b A shared_ptr to the buffer containing the L2 frame.
       * \return The time, in microseconds, to transfer this packet.
       */
      double packet_succ_time(net::buffer_sptr b) const;

      /**
       * Compute the time taken by failing to send packet b.
       *
       * \param b A shared_ptr to the buffer containing the L2 frame.
       * \return The time, in microseconds, to transfer this packet.
       */
      double packet_fail_time(net::buffer_sptr b) const;

      /**
       * Return the average contention window time for transmission
       * attempt txc.
       *
       * \param txc The number of the transmission attempt.
       * \return The time, in microseconds, that will be waited on average.
       */
      double avg_contention_time(uint8_t txc) const;

      /**
       * Return the contention window time for transmission attempt
       * txc.
       *
       * \param txc The number of the transmission attempt.
       * \return The time, in microseconds, used for the contention window.
       */
      double max_contention_time(uint8_t txc) const;

      /**
       * Compute the time taken to successfully send frame b. This
       * includes inter-frame spacing, acknowledgment and
       * the RTS/CTS if necessary.
       *
       * \param b A shared_ptr to the buffer.
       * \return The time, in microseconds, necessary to send the frame.
       */
      double frame_succ_time(net::buffer_sptr b) const;

      /**
       * Compute the time taken to unsuccessfully send frame b. This
       * includes inter-frame spacing, ACKTIMEOUT and the RTS/CTS if
       * necessary.
       *
       * \param b A shared_ptr to the buffer.
       * \return The time, in microseconds, used by the failed exchange.
       */
      double frame_fail_time(net::buffer_sptr b) const;

      /**
       * Return the amount of time taken by the RTS/CTS exchange.
       *
       * \param frame_sz The size of the data frame.
       * \return The time, in microseconds, used  by the RTS/CTS exchange.
       */
      double rts_cts_time(uint32_t frame_sz) const;

      /**
       * Return the ACK rate for a given rate.
       *
       * \param rate_Kbs The data rate in units of 1Kb/s.
       * \return The acknowledgment data rate in units of 1Kb/s.
       */
      uint32_t ack_rate(uint32_t rate_Kbs) const;

   private:

      /**
       * The receiver side of the elc_link_metric.
       */
      net::eui_48 to_;

      /**
       * The sender side of the elc_link_metric.
       */
      net::eui_48 from_;

      /**
       * The RTS/CTS threshold.
       */
      uint16_t rts_cts_threshold_;

      /**
       * The number of successful packet deliveries.
       */
      uint32_t n_pkt_succ_;

      /**
       * The cumulative airtime for successful packet deliveries.
       */
      double t_pkt_succ_;

      /**
       * The cumulative airtime for failed packet deliveries.
       */
      double t_pkt_fail_;

      /**
       * The cumulative size for packets sent on this elc_link_metric.
       */
      uint32_t packet_octets_;

      /**
       * The number of packets sent on this elc_link_metric.
       */
      uint32_t packet_count_;
   };

}

#endif // METRICS_ELC_LINK_METRIC_HPP
