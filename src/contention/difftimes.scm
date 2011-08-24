#!/usr/bin/guile
!#

(let ((first (read)))
  (let (filter (l first) (last 0))
    (cond ((eof-object? l)
           #f)
          ((and (list? l) (= (length l) 3))
           (display (list l (- (cadr l) last)))
           (filter (read) (cadr 1))))))
        
