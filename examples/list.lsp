;; -*-Lisp-*-

;; one list
(list 1 2 3)
(1 2 3)

;; list of lists
(list (list 1 2 3) (list 4 5 6) (list 7 8 9))
=> ((1 2 3) (4 5 6) (7 8 9))

(setq tl (list (list 1 2 3) (list 4 5 6) (list 7 8 9)))
=> ((1 2 3) (4 5 6) (7 8 9))

;; does not work
(append tl (list 10 11 12))
=> ((1 2 3) (4 5 6) (7 8 9) 10 11 12)

;; create a single value which is a list
(cons (list 10 11 12) ())
=> ((10 11 12))

;; we can extend the list  of lists by treating the final arg to append as a single object
(append tl (cons (list 10 11 12) ()) )
=> ((1 2 3) (4 5 6) (7 8 9) (10 11 12))

(defun conslist (l nl)
   (append l (cons nl ())) )

(conslist tl (list 10 11 12))
=> ((1 2 3) (4 5 6) (7 8 9) (10 11 12))
