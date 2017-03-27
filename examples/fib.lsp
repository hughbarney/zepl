;;
;; examples
;;

(defun fib (n)
  (cond ((< n 2) 1)
        (t (+ (fib (- n 1)) (fib (- n 2))))))

(defun sq (x)
  (* x x))

(fib 10)

(sq 27)

