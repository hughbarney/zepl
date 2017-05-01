; O | X | O
;----------
; O | X | X
;----------
; O | 8 | X








 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
  
 
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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

(defun newline_and_space()
  (insert-string "\n "))

;; prompt for string and return response, handle backspace, cr and c-g
(defun inputat(ln q response)
  (gotoline ln)
  (beginning-of-line)
  (kill-to-eol)
  (insert-string (concat q response))
  (message "")
  (display)
  (setq key (getch))
  (cond
    ((eq key "\n") response)
    ((is_ctl_g key) "")
    ((is_backspace key) (inputat ln q (shrink response)))
    ((is_control_char key) (inputat ln q response))
    (t (inputat ln q (string.append response key)))  ))

(defun draw()
  (beginning-of-buffer)
  (set-mark)
  (repeat 10 next-line)
  (kill-region)
  (beginning-of-buffer)
  (insert-string (concat " " (val 1) " | " (val 2) " | " (val 3) "\n"))
  (insert-string "----------\n")
  (insert-string (concat " " (val 4) " | " (val 5) " | " (val 6) "\n"))
  (insert-string "----------\n")
  (insert-string (concat " " (val 7) " | " (val 8) " | " (val 9) "\n"))
  (repeat 5 newline_and_space)
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

(defun game_not_over()
  (and (not (check_for_win wins "X")) (not (check_for_win wins "O")) (not (board_full (cdr board))) ))

(defun get-move()
  (setq m (inputat 7 "Your move: " ""))
  (setq m (string->number m))
  (if (or (> m 9) (< m 1)) (progn (msg "Please select a free cell between 1 and 9" t) (get-move)))
  (if (not (is_free m)) (progn (msg "That cell is taken" t) (get-move)))
  m)

(defun find_free(b)
  (cond
    ((and (not (eq "X" (car b))) (not (eq "O" (car b))) (not (eq "E" (car b)))) (string->number (car b)))
    (t (find_free (cdr b))) ))

(defun is_free(n)
  (and (not (eq "X" (val n))) (not (eq "O" (val n))) (not (eq "E" (val n))) ))

(defun not_taken(v)
  (and (not (eq "X" v)) (not (eq "O" v)) (not (eq "E" v)) ))

(defun board_full(brd)
  (cond
    ((eq brd ()) t)
    ((not_taken (car brd)) nil)
    (t (board_full (cdr brd))) ))

(defun msg(s pause)
  (if pause
  (progn
    (print_message (concat s " - press a key to continue "))
    (getch))
  (progn
    (print_message s)) ))

(defun print_message(s)
  (clearline 7)
  (insert-string s)
  (message "")
  (display))

(defun clearline(ln)
  (gotoline ln)
  (beginning-of-line)  
  (kill-to-eol) )

;; just find first empty square
(defun computer_move()
    (setq board (set-nth board (find_free board) "O")) )

(defun show_result()
  (draw)
  (cond
    ((check_for_win wins "X") (msg "X wins !" t))
    ((check_for_win wins "O") (msg "O wins !" t))
    (t (msg "Draw !" t)) ))

(defun play_again()
  (setq m (inputat 8 "Play again (y or n) ? " ""))
  (or (eq m "y") (eq m "Y")) )

(defun play()
  (draw)
  (display)
  (if (game_not_over) (setq board (set-nth board (get-move) "X")) (show_result))
  (if (game_not_over) (progn (computer_move) (play)) (show_result)) )

(defun oxo()
  (init)
  (play)
  (if (play_again)
    (oxo)
  (progn 
    (msg "Thank you for playing" nil)
    (clearline 8)
    (message "")) ))

