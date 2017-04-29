;;
;; writing a string concatenation function / macro
;;


EXAMPLE 1 - using a function
============================

;; concatenate a list of strings
(defun concati(args)
  (cond
    ((null args) "")
    ((null (cdr args)) (car args))
    (t (string.append (car args) (concati (cdr args))))))


; when we call it we have to pass in a list, we can use a variable sequence of arguments

(concati "hhh-" "yyy:" "ttrr%" "yyyy")
error: #<Lambda (args)> expects at most 1 arguments

; create a list argument first

(concati (list "hhh-" "yyy:" "ttrr%" "yyyy"))
"hhh-yyy:ttrr%yyyy"


EXMAPLE 2 - using a macro
=========================

; lets suppose we have a list of strings args
(setq args (list "hhh" "yyy" "ttrr" "yyyy"))
=> ("hhh" "yyy" "ttrr" "yyyy")

; The key to the macro is:
(list (quote string.append) (car args) (cons (quote concat) (cdr args)))

; this translates to:
(string.append "hhh" (concat "yyy" "ttrr" "yyyy"))

; (concat "yyy" "ttrr" "yyyy") is then expanded again to:
(string.append "yyy" (concat "ttrr" "yyyy")


;;
(defmacro concat args
  (cond
    ((null args) "")
    ((null (cdr args)) (car args))
    (t (list (quote string.append) (car args) (cons (quote concat) (cdr args)))) ))

;; now we dont need to use the list keyword to make a list first
(concat "hhh-" "yyy:" "ttrr%" "yyyy")
"hhh-yyy:ttrr%yyyy"
