/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_LINK_HPP
#define METRICS_LINK_HPP

#include <net/eui_48.hpp>
#include <dot11/frame.hpp>

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
       * \param from The address of the sending side of this link.
       * \param to The address of the receiving side of this link.
       * \param rts_cts_threshold Use RTS/CTS when frame <= frame size
       */
      explicit link(const net::eui_48& from, const net::eui_48& to, uint16_t rts_cts_threshold);

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
       * Add a frame to the link statistics.
       *
       * \param f A const reference to a frame.
       */
      void add(const dot11::frame& f);

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

      /*(
       * The sender side of the link.
       */
      net::eui_48 from_;

      /**
       * The receiver side of the link.
       */
      net::eui_48 to_;

      /**
       * The RTS/CTS threshold.
       */
      uint16_t rts_cts_threshold_;

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
