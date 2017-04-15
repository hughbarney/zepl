

(defun describe-key()
  (prompt "Describe Key: " "")
  (setq key (get-key))
  (cond
    ((not (eq key "")) (message key))
    (t (message (get-key-name)))))


(set-key "c-x ?" "(describe-key)")

