;;
;; miscellaneous editor macros and lisp code etc
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
