# Zepl 

A tiny Emacs Editor core with a tiny lisp extention language in less than 2500 lines of C.

Zepl is a Zep[8] based editor with a lisp extension language. The lisp extension language was derived from Tiny-Lisp[7].

> A designer knows he has achieved perfection not when there is nothing left to add, but when there is nothing left to take away.
> -- <cite>Antoine de Saint-Exupery</cite>

## Goals of Zep Emacs

* Provide just enough editing features to be able to make small changes to files
* Consist of a Tiny Editor core with as much of the editor as possible being implemented in lisp extensions
* Provide a tiny experimental platform for playing around with Lisp

## Why the name Zepl ?

Zepl is based on the Zep editor with a Read-Evaluate-Print-Loop (REPL).
Hence Zepl is a pun on Repl.

## Starting Zep

Zepl can only open one file at a time.  The filename to edit must be specified on the command line.

    $ zepl filename

The one file limitation will be removed in the future once more lisp interaction code has been developed.

## Basic Zepl Key Bindings
    C-A   begining-of-line
    C-B   backward-character
    C-D   delete-char
    C-E   end-of-line
    C-F   forward Character
    C-I   handle-tab
    C-J   newline
    C-M   Carrage Return
    C-N   next line
    C-P   previous line
    C-S   search-forwards
    C-V   Page Down
    C-X   CTRL-X command prefix

    esc-<   Start of file
    esc->   End of file
    esc-v   Page Up
	esc-]   evaluate-block

    ^X^C  Exit. Any unsaved files will require confirmation.
    ^X^S  Save current buffer to disk, using the buffer's filename as the name of

    Home  Beginning-of-line
    End   End-of-line
    Del   Delete character under cursor
    Left  Move left
    Right Move point right
    Up    Move to the previous line
    Down  Move to the next line
    Backspace delete caharacter on the left

## Key Bindings Implemented in Lisp

    C-K   kill-to-eol
    C-x ? describe-key
	Esc-a duplicate-line

### Searching
    C-S enters the search prompt, where you type the search string
    BACKSPACE - will reduce the search string, any other character will extend it
    C-S at the search prompt will search forward, will wrap at end of the buffer
    ESC will escape from the search prompt and return to the point of the match
    C-G abort the search and return to point before the search started

### Copying and moving
    C-<spacebar> Set mark at current position
    ^W     Delete region
    ^Y     Yank back kill buffer at cursor
    esc-w  Copy Region
    esc-k  Kill Region

A region is defined as the area between this mark and the current cursor position. The kill buffer is the text which has been most recently deleted or copied.

Generally, the procedure for copying or moving text is:
1. Mark out region using M-<spacebar> at the beginning and move the cursor to the end.
2. Delete it (with ^W) or copy it (with M-W) into the kill buffer.
3. Move the cursor to the desired location and yank it back (with ^Y).

## Lisp Interaction

Type a lisp function into the editor.

for example:

    1: --------------
    2: (defun factorial (n)
    3:   (cond ((= n 0) 1)
    4:     (t (* n (factorial (- n 1))))))
    5:--------------

Place the cursor at the beginning of line 1 and set a mark (hit control-spacebar).

Now move the cursot to line 5 and evaluate the block of code (hit escape followed by ])

Zepl will pass the code to lisp for it to be evaluated.
 
    <Lambda (n)>

Now call factorial in the same way (mark the start of the code, move to the end of the code and hit escape-])

    (factorial 6)

    720

## zepl.rc file

The sample zepl.rc file should be placed into your HOME directory
The example shows how the editor can be extended.

    ;;
    ;; ZEPL a tiny Emacs editor core with a tiny lisp extension language
    ;; hughbarney AT googlemail.com
    ;;
    ;; The editor provides only basic buffer movement and edit functions
    ;; everything else is done by extending the user interface using the
    ;; lisp extension language. Functions can be bound to keys using set-key.
    ;; For example: (set-key "c-k" "(kill-to-eol)")
    ;; 
    ;; place zepl.rc in your home direcory and it is run when zepl starts up.
    ;;
    
    (defun repeat (n func)  
      (cond ((> n 0) (func) (repeat (- n 1) func))))
    
    (defun duplicate_line()
      (beginning-of-line)
      (set-mark)
      (next-line)
      (beginning-of-line)
      (copy-region)
      (yank)
      (previous-line))
    
    (defun kill-to-eol()
      (cond 
        ((eq "\n" (get-char)) 
           (delete))
        (t 
           (set-mark)
           (end-of-line)
           (kill-region)) ))
    
    ;;
    ;; prompt for a keystroke then show its name
    ;; (once we have string.append we can display both name and funcname in the message
    ;;
    (defun describe-key()
      (prompt "Describe Key: " "")
      (setq key (get-key))
      (cond
        ((not (eq key "")) (message key))
        (t (message (get-key-name)))))
    
    
    (set-key "esc-a" "(duplicate_line)")
    (set-key "c-k" "(kill-to-eol)")
    (set-key "c-x ?" "(describe-key)")
    

## Build in Editor functions that can be called through lisp.

	(beginning-of-buffer)                   # go to the beginning of the buffer
	(end-of-buffer)                         # go to the end of the buffer
	(beginning-of-line)                     # go to the beginning of the current line
	(end-of-line)                           # go to the end of the current line
	(forward-char)                          # move forward 1 character to the right
	(backward-char)
	(next-line)
	(previous-line)
	(set-mark)
	(delete)
	(copy-region)
	(kill-region)
	(yank)
	(backspace)
	(page-down)
	(page-up)
	(save-buffer)
	(exit)

	(string? symbol)                        # return true if symbol is a string
	(load "filename")                       # load and evaluate the lisp file
	(message "the text of the message")     # set the message line
	(set-key "name" "(function-name)"       # specify a key binding
	(prompt "prompt message" "response")    # display the prompt in the command line, pass in "" for response.
	                                        # pass in a previous response with a no empty string
											
	(get-char)                              # return the character at the current position in the file
	(get-key)                               # wait for a key press, return the key or "" if the key was a command key
	(get-key-name)                          # return the name of the key pressed eg: c-k for control-k.
	(get-key-funcname)                      # return the name of the function bound to the key


## Swapping C code with Lisp

The intension is to enable as much as possible to be implemented in lisp rather than C code.
A good exmaple of this was the function to kill to the end of line. This was originally implemented as:
   
    void killtoeol()
    {
    	/* point = start of empty line or last char in file */
    	if (*(ptr(curbp, curbp->b_point)) == 0xa || (curbp->b_point + 1 == ((curbp->b_ebuf - curbp->b_buf) - (curbp->b_egap - curbp->b_gap))) ) {
    		delete();
    	} else {
    		curbp->b_mark = curbp->b_point;
    		lnend();
    		copy_cut(TRUE);
    	}
    }

But is now the following lisp code in the configuration file.

    (defun kill-to-eol()
       (cond 
          ((eq "\n" (get-char)) 
             (delete))
          (t 
             (set-mark)
             (end-of-line)
             (kill-region)) ))
    
    (set-key "c-k" "(kill-to-eol)")

## Known Issues

when evaluating load("lisp.lsp") in the buffer a recursive call gets setup between call_lisp(), load().
This results in the output buffer being reset.

To resolve this issue I need to rewite the Stream code in lisp.c so that the buffer is allocated
from memory.  Also so that a new instance of output stream is created for the load as well as for the
call_lisp().   The call_lisp() function will then me invoked as follows:

      jjjjj


## Copying
  Zep is released to the public domain.
  hughbarney AT gmail.com 2017

## References
    [1] Perfect Emacs - https://github.com/hughbarney/pEmacs
    [2] Anthony's Editor - https://github.com/hughbarney/Anthony-s-Editor
    [3] MG - https://github.com/rzalamena/mg
    [4] Jonathan Payne, Buffer-Gap: http://ned.rubyforge.org/doc/buffer-gap.txt
    [5] Anthony Howe,  http://ned.rubyforge.org/doc/editor-101.txt
    [6] Anthony Howe, http://ned.rubyforge.org/doc/editor-102.txt
    [7] Tiny-Lisp,  https://github.com/matp/tiny-lisp
    [8] Zep, https://github.com/hughbarney/zep
