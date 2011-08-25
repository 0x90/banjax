#!/usr/bin/guile
!#

(call/cc
 (lambda (break)
   (let ((first (read)))
     (let filter ((l first) (last 0))
       (cond ((and (list? l) (= (length l) 3))
              (display (cons (- (cadr l) last) l))
              (newline)
              (filter (read) (cadr l)))
             (else
              (break l)))))))
