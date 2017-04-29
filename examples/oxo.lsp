































;;;;;;;;;;;;;;;;;;

(defun init()
 (setq board (list "E" "1" "2" "3" "4" "5" "6" "7" "8" "9")))

(defun val(n)
 (nth n board))

(defun nth(n l)
  (cond 
    ((eq n 0) (car l))
    (t (nth (- n 1) (cdr l))) ))

(defun set-nth (list n val)
  (if (> n 0)
    (cons (car list) (set-nth (cdr list) (- n 1) val))
    (cons val (cdr list)) ))

(defun newline()
  (insert-string "\n"))

(defun draw()
  (beginning-of-buffer)
  (set-mark)
  (repeat 10 next-line)
  (kill-region)
  (beginning-of-buffer)
  (insert-string (concat (list " " (val 1) " | " (val 2) " | " (val 3) "\n")))
  (insert-string "----------\n")
  (insert-string (concat (list " " (val 4) " | " (val 5) " | " (val 6) "\n")))
  (insert-string "----------\n")
  (insert-string (concat (list " " (val 7) " | " (val 8) " | " (val 9) "\n")))
  (repeat 5 newline)
  (beginning-of-buffer))

(setq wins (list 
  (list 1 2 3)
  (list 4 5 6)
  (list 7 8 9)
  (list 1 4 7)
  (list 2 5 8)
  (list 3 6 9) 
  (list 1 5 9) 
  (list 3 5 7) ))

(defun check_win_line(w p)
  (and (eq p (val (nth 0 w))) (eq p (val (nth 1 w))) (eq p (val (nth 2 w)))) )

(defun check_for_win(l p)
  (cond
    ((eq l ()) nil)
    ((check_win_line (car l) p) t)
    (t (check_for_win (cdr l) p)) ))

(defun game_not_won()
  (and (not (check_for_win wins "X")) (not (check_for_win wins "O"))) )

(defun get-move()
  (setq m (input "Your move: " ""))
  (string->number m))

(defun find_free(b)
  (cond
    ((and (not (eq "X" (car b))) (not (eq "O" (car b))) (not (eq "E" (car b)))) (string->number (car b)))
    (t (find_free (cdr b))) ))

;; just find first empty square
(defun computer_move()
    (setq board (set-nth board (find_free board) "O")) )

(defun play()
  (draw)
  (display)
  (if (game_not_won)
    (setq board (set-nth board (get-move) "X")))
  (if (game_not_won)
  (progn
    (computer_move)
    (play))) )

(progn 
  (init)
  (play))

