# Zepl 

A tiny Emacs Editor core with a tiny lisp extention language in less than 2500 lines of C.

Zepl is a Zep[8] based editor with a lisp extension language. The lisp extension language was derived from Tiny-Lisp[7] by Matthias Pirstitz.


![zepl screenshot](https://github.com/hughbarney/zepl/blob/master/screenshots/Screenshot%202017-04-01%20at%2018.25.03.png)


> A designer knows he has achieved perfection not when there is nothing left to add, but when there is nothing left to take away.
> -- <cite>Antoine de Saint-Exupery</cite>

## Goals of Zepl Emacs

* Provide a reference implementation to a standard way to embed a lisp interpretter to an application
* Provide just enough editing features to be able to make small changes to files
* Consist of a Tiny Editor core with as much of the editor as possible being implemented in lisp extensions
* Provide a tiny experimental platform for playing around with Lisp and Editor extensions written in Tiny-Lisp

* The result has totally surpassed my expectations. I have now been
  able to implement some substantial parts of the Editor in Lisp. The
  Tiny-Lisp codebase has proved very flexible. With the extensions I
  have added I have been able to write a basic version of Naughts and
  Crosses (Tic-Tac-Toe) that runs in the Editor window itself.


## Why the name Zepl ?

Zepl is based on the Zep editor with a Read-Evaluate-Print-Loop (REPL).
Hence Zepl is a pun on Repl.

## Starting Zepl

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

    c-k   kill-to-eol
    c-s   Search
    c-x ? describe-key
    c-]   find and evaluate last s-expression
    esc-a duplicate-line
    esc-g gotoline
   

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

There are two ways to interract with Tiny-Lisp within Zepl.

* You can use C-] to find the last s-expression above the cursor and send it to be evaluated.
* You can mark a region and send the whole region to be evaluated.

### Lisp Interaction - finding and evaluating the last s-expression

This works in almost the same way as GNU Emacs in the scratch buffer.


### Lisp Interaction - mark and evaluating a region

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

```lisp
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
    
    ;; concatenate a list of strings
    (defun concat(args)
      (cond
        ((null args) "")
        ((null (cdr args)) (car args))
        (t (string.append (car args) (concat (cdr args))))))
    
    (defun duplicate_line()
      (beginning-of-line)
      (set-mark)
      (next-line)
      (beginning-of-line)
      (copy-region)
      (yank)
      (previous-line))
    
    ;; kill to end of line, uses if and progn
    (defun kill-to-eol()
      (if (eq "\n" (get-char))
        (progn
          (delete))
        (progn
          (set-mark)
          (end-of-line)
          (kill-region))))
    
    ;; prompt for a keystroke then show its name
    (defun describe-key()
      (prompt "Describe Key: " "")
      (setq key (get-key))
      (cond
        ((not (eq key "")) (message key))
        (t (message (concat (list (get-key-name) " runs command " (get-key-funcname)))))))
    
    (set-key "esc-a" "(duplicate_line)")
    (set-key "c-k" "(kill-to-eol)")
    (set-key "c-x ?" "(describe-key)")
    
    ;; example: this keystroke will fail as we have not defined (dobee)
    ;; the error will be displayed on the message line
    (set-key "esc-b" "(dobee)")
```

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
	(string.append "string1" "string2"      # concatenate 2 strings returning a new string
        (string.substring string n1 n2          # return a substring of string from ref n1 to n2
	(string->number s)                      # return a number converted from the string, eg "99" => 99
        (number->string n)                      # return a strung representation of the number, eg 99.56 => "99.56"

	(load "filename")                       # load and evaluate the lisp file
	(message "the text of the message")     # set the message line
	(set-key "name" "(function-name)"       # specify a key binding
	(prompt "prompt message" "response")    # display the prompt in the command line, pass in "" for response.
	                                        # pass in a previous response with a no empty string
											
	(get-char)                              # return the character at the current position in the file
	(get-key)                               # wait for a key press, return the key or "" if the key was a command key
	(get-key-name)                          # return the name of the key pressed eg: c-k for control-k.
	                                          only valid immediatley after a call to (get-key)
	(get-key-funcname)                      # return the name of the function bound to the key
	                                          only valid immediatley after a call to (get-key)

	(getch)                                 # calls the c function getch and returns the keystroke
                                                # blocks until a key is pressed

	(insert-string "string")                # insert the string into the buffer at the current location
	(set-point 1234)                        # set the point to the value specified
	(get-point)                             # returns the current point
	(set-key "key-name" "(lisp-func)")      # binds a key to a lisp function, see keynames see "Keys Names below"
	(prompt)                                # prompts for a value on the command line and returns the response
	(eval-block)                            # passes the marked region to be evaluated by lisp, displays the output



	(search-forward 100 "accelerate")       # search forward from the point value passed in for the string supplied
                                                # returns -1 if string is found or the point value of the match
	(display)                               # calls the display function so that the screen is updated
	(refresh)     


## Key Names

The following keynames are used for user key bindings

* "c-a" to "c-z"                 Control-A to Control-Z  (Control-X, I and M are reserved)
* "c-x c-a" to "c-x c-z"         Control-X folowed by Control-A to Control-Z
* "esc-a" to "esc-z"             Escape-A to Escape-Z

When using (set-key) the keyname must be supplied the formats above.
The lisp function must be enclosed in brackets ().

Examples:
```lisp
   
    (set-key "esc-a" "(duplicate_line)")
    (set-key "c-k" "(kill-to-eol)")
    (set-key "c-x ?" "(describe-key)")
```

Key bindings cane be checked using describe-key (c-x ?).
This is implemented in Lisp in the zepl.rc file.


## Swapping C code with Lisp

The intension is to enable as much as possible to be implemented in lisp rather than C code.
A good exmaple of this was the function to kill to the end of line. This was originally implemented as:
   
```c
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
```

But is now the following lisp code in the configuration file.

```lisp
    (defun kill-to-eol()
       (cond 
          ((eq "\n" (get-char)) 
             (delete))
          (t 
             (set-mark)
             (end-of-line)
             (kill-region)) ))
    
    (set-key "c-k" "(kill-to-eol)")
```

## Interface to Lisp Interpretter

In order to be able to easily embed a Lisp interpretter into an application a small set of interface functions are required.

```c
     void init_lisp();                               /* setup up the lisp environment, that subsequent calls to */
	                                             /* call_lisp() and load_file() will use and where the state */
						     /* will be persisted between calls. */

     char *output = call_lisp(char *input);          /* passes a string to the lisp interpretter and captures   */
	                                             /* the resulting output in char *output. */
						     /* The advantage of this approach is that the application can */
						     /* retrieve any size output from the interpretter and not worry */
						     /* about allocating a buffer */

     char *output = load_file(char *filename);      /* opens and passes the contents file to the lisp interpretter */
```

## Lisp Functions needed for future enhancements

```lisp
    (string.ref string n)                 ;; return character (as a string) at position n in the string
    (number->string n)                    ;; return a string representation of number n
    (string.find str s)                   ;; find start of string s in string str
```

## Future Editor Features

    Make Zepl handle multiple files       ;; this is fairly easy to do as I have 3 other editors that
	                                  ;; implement this in about 120 lines.


## Copying
  Zepl is released to the public domain.
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
