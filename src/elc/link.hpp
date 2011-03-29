/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_LINK_HPP
#define METRICS_LINK_HPP

#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * link represents the link between two stations.
    */
   class link {
   public:

      /**
       * link constructor. Creates a new uni-directional link between
       * "from" and "to".
       *
       * \param to The address of the receiving side of this link.
       * \param from The address of the sending side of this link.
       * \param rts_cts_threshold Use RTS/CTS when rts_cts_threshold <= frame size
       */
      explicit link(const net::eui_48& to, const net::eui_48& from, uint16_t rts_cts_threshold);

      /**
       * link copy-constructor. Initialize a new channel instance
       * with the same state as other.
       *
       * \param other A reference to the object to initialize from.
       */
      link(const link& other);

      /**
       * link assignment operator. Assign this mac address so that
       * it has the same value as other.
       *
       * \param other A reference to the object to initialize from.
       */
      link& operator=(const link& other);

      /**
       * link destructor.
       */
     ~link();

      /**
       * link equality comparison operator. Compares this MAC
       * address with rhs and returns true if this is equal to rhs;
       * otherwise returns false.
       *
       * \param rhs The link to compare against (RHS of expr).
       * \return true if the this is less than rhs; otherwise false.
       */
      bool operator==(const link& rhs) const;

      /**
       * link less than comparison operator. Compares this MAC
       * address with rhs and returns true if this is smaller than
       * rhs; otherwise returns false.
       *
       * \param rhs The link to compare against (RHS of expr).
       * \return true if the this is less than rhs; otherwise false.
       */
      bool operator<(const link& rhs) const;

      /**
       * Add a frame to the link and update the link statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      void add(net::buffer_sptr b);

      /**
       * Returns the hash value for the specified link object.
       *
       * \param A size_t containing the hash value.
       */
      std::size_t hash() const;

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      void write(std::ostream& os) const;

   private:

      /**
       * Compute and return the link quality metric.
       *
       * \return A double specifying the ELC value in b/us.
       */
      double metric() const;

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
       * \param A shared_ptr to the buffer.
       * \return The time, in microseconds, necessary to send the frame.
       */
      double frame_succ_time(net::buffer_sptr b) const;

      /**
       * Compute the time taken to unsuccessfully send frame b. This
       * includes inter-frame spacing, ACKTIMEOUT and the RTS/CTS if
       * necessary.
       *
       * \param A shared_ptr to the buffer.
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
       * The receiver side of the link.
       */
      net::eui_48 to_;

      /**
       * The sender side of the link.
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
       * The cumulative size for packets sent on this link.
       */
      uint32_t packet_octets_;

      /**
       * The number of packets sent on this link.
       */
      uint32_t packet_count_;
   };


   /**
    * Returns the hash value for the specified link object.
    *
    * \param l A const-reference to the object to hash.
    * \param A size_t containing the hash value.
    */
   size_t hash(const link& l);

   /**
    * operator to stream a link to an ostream.
    *
    * \param os The stream to write to.
    * \param addr The link to be streamed.
    * \return A reference to the modified ostream.
    */
   std::ostream& operator<<(std::ostream& os, const link& addr);

   /**
    * Alias for shared_ptr<link>.
    */
   typedef boost::shared_ptr<link> link_sptr;


}

#endif // METRICS_LINK_HPP
