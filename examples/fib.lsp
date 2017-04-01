;;
;; ZEPL a tiny editor core with a tiny lisp extension language
;; hughbarney AT googlemail.com
;;
;; To execute lisp code within a buffer: mark the top of the function
;; move to the end of the function.  Type Esc-] to evaluate the marked block 
;; see examples below
;;

(defun fib (n)
  (cond ((< n 2) 1)
        (t (+ (fib (- n 1)) (fib (- n 2))))))

(defun factorial (n)
  (cond ((= n 0) 1)
        (t (* n (factorial (- n 1))))))

#<Lambda (n)>

(factorial 6)

720

(defun sq (x)
  (* x x))

#<Lambda (x)>

(sq 27)

729


