;;
;; The following functions and macros are implemented within the tiny-lisp code
;;


  (setq list (lambda args args))
  
  (setq defmacro (macro (name params . body)
    (list (quote setq) name (list (quote macro) params . body))))
  
  (defmacro defun (name params . body)
    (list (quote setq) name (list (quote lambda) params . body)))
  
  (defun null (x) (eq x nil))
  
  (defun map1 (func xs)
    (if (null xs)
        nil
        (cons (func (car xs))
              (map1 func (cdr xs)))))
  
  (defmacro and args
    (cond ((null args) t)
          ((null (cdr args)) (car args))
          (t (list (quote if) (car args) (cons (quote and) (cdr args))))))

  (defmacro or args 
    (if (null args)
        nil
        (cons (quote cond) (map1 list args))))

  (defun not (x) (if x nil t))

  (defun consp (x) (not (atom x)))
  (defun listp (x) (or (null x) (consp x)))

  (defun zerop (x) (= x 0))
  
  (defun equal (x y)
    (or (and (atom x) (atom y)
             (eq x y))
        (and (not (atom x)) (not (atom y))
             (equal (car x) (car y))
             (equal (cdr x) (cdr y)))))
  
  (defun nth (n xs)
    (if (zerop n)
        (car xs)
        (nth (- n 1) (cdr xs))))

  (defun append (xs y)
    (if (null xs)
        y
        (cons (car xs) (append (cdr xs) y))))
