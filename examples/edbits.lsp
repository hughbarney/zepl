;;
;; left over versions of miscellaneous editor macros and lisp code
;;

;; version using cond
(defun kill-to-eol()
  (cond 
    ((eq "\n" (get-char)) 
       (delete))
    (t 
       (set-mark)
       (end-of-line)
       (kill-region)) ))

(defun tryif(b)
  (if (eq b t) (message "TRUE") (message "FALSE")))



;; find the start of the s-expression
;; this version suffers from 2 problems
;; 1. it cant handle comments (as they are at the start of a line
;; 2. it cant handle single lines sometimes
;; this version also needs the (skip_string_backwards) function
(defun find_start_p()
  (setq kyy (get-char))
  (cond 
    ((and (eq 0 (get-point)) (not (eq kyy "("))) -1) 
    ((and (eq lb_count rb_count) (> lb_count 0)) (get-point))
    ((eq kyy "\"") (backward-char) (skip_string_backwards) (find_start_p))
    ((eq kyy "(") (setq lb_count (+ lb_count 1)) (backward-char) (find_start_p))
    ((eq kyy ")") (setq rb_count (+ rb_count 1)) (backward-char) (find_start_p))
    (t (backward-char) (find_start_p)) ))


;; skip backward out of a quoted string from a point inside the string
(defun skip_string_backwards()
  (setq ky (get-char))
  (cond
    ((eq 0 (get-point)) nil)
    ((eq ky "\"") (backward-char) (if (eq "\\" (get-char)) (progn (backward-char) (skip_string_backwards))) t)
    (t (backward-char) (skip_string_backwards)) ))

