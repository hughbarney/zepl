
;;
;;  FIRST ATTEMPT
;;

;; private: concatenate a list of strings into sr 
(defun concati(sr sl)
  (cond 
    ((eq sl ()) sr)
    (t (concati (string.append sr (car sl)) (cdr sl)))))

;; public interface - concatenate a list of strings
(defun concats(l)
  (concati "" l))

;; example: (concats (list "abcd" "gghh" "gggh"))

;;
;; SECOND ATTEMPT
;;

;; method2 - single function, better !
(defun concata(args)
 (cond
   ((null args) "")
   ((null (cdr args)) (car args))
   (t (string.append (car args) (concata (cdr args))))))


(concats (list "abcd" " " "ghhj"))

(concata (list "abcd" " " "ghhj"))

;; would like to get to following syntax
(concat "abcd" " " "ghhj"  ...)

