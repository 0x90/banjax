/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_METRIC_GROUP_HPP
#define METRICS_METRIC_GROUP_HPP

#include <metric.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>
#include <list>

namespace metrics {

   /**
    * metric_group represents an aggregate metric for a link.
    */
   class metric_group : public metric {
   public:

      /**
       * metric_group default constructor.
       */
      metric_group();

      /**
       * metric_group copy constructor.
       *
       * \param other The other metric_group to initialize from.
       */
      metric_group(const metric_group& other);

      /**
       * metric_group assignment operator.
       *
       * \param other The other metric_group to initialize from.
       * \return A reference to this metric_group.
       */
      metric_group& operator=(const metric_group& other);

      /**
       * metric_group destructor.
       */
      virtual ~metric_group();

      /**
       * Add a metric to this group.
       *
       * \param m A metric_sptr pointing to the link_metric.
       */
      virtual void add(metric_sptr m);

      /**
       * Add a frame to the metric_group and update the metric_group statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (a deep copy) of this
       * metric_group instance. The clone is allocated on the heap
       * using new and the caller is responsible for ensuring it is
       * deleted.
       *
       * \return A poiner to a new metric_group instance.
       */
      virtual metric_group *clone() const;

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
       * Alias for list<metric_sptr>.
       */
      typedef std::list<metric_sptr> metric_list;

      /**
       * The receiver side of the metric_group.
       */
      metric_list metrics_;

   };

   /**
    * Alias for shared_ptr<metric_group>.
    */
   typedef boost::shared_ptr<metric_group> metric_group_sptr;

}

#endif // METRICS_METRIC_GROUP_HPP
