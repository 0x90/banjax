#!/opt/local/bin/guile
!#

(define t (+ t-ctrl t-ctrl-ifs t-ctrl-delta t-data t-data-ifs t-data-cw t-mgmt t-mgmt-ifs t-mgmt-cw))
(define n (+ n-ctrl n-mgmt n-data))

(define ctrl (exact->inexact (/ t-ctrl n-ctrl)))
(define ctrl-ifs (exact->inexact (/ t-ctrl-ifs n-ctrl)))
(define ctrl-delta (exact->inexact (/ t-ctrl-delta n-ctrl)))

(define mgmt (exact->inexact (/ t-mgmt n-mgmt)))
(define mgmt-ifs (exact->inexact (/ t-mgmt-ifs n-mgmt)))
(define mgmt-cw (exact->inexact (/ t-mgmt-cw n-mgmt)))

(define data (exact->inexact (/ t-data n-data)))
(define data-ifs (exact->inexact (/ t-data-ifs n-data)))
(define data-cw (exact->inexact (/ t-data-cw n-data)))

(define iperf (exact->inexact (/ t-iperf n-iperf)))
