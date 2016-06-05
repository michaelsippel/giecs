#! ../vm_linux64

progn
(
	(declare 0 defvar '(macro (name val)
		(declare 0 name val)))

	(declare 0 defmacro '(macro (name plist def)
		(defvar name '(macro plist def))))

	(declare 0 defun '(macro (name plist def)
		(defvar name '(function plist def))))

	(defun read (fd buf len)
		(syscall 3 fd buf len 0 0))

	(defun write (fd buf len)
		(syscall 4 fd buf len 0 0))

	(defun open (path oflags mode)
		(syscall 5 path oflags mode 0 0))

	(defun close (fd)
		(syscall 6 fd 0 0 0 0))

	(defvar stdin 0)
	(defvar stdout 1)
	(defvar stderr 2)

	(defvar mptr 30000)
	(defun malloc (size)
		(resw 'mptr))
#		(progn
#			((setw 'mptr (+ mptr size))
#			(return mptr)))

	(printi (malloc 10))

	(defun factorial (n)
		(eval (if (eq n 1)
			'(quote 1)
			'(* (factorial (- n 1)) n))))

	(defvar a 8)
	(write stdout "\ndeclared a: " 14)
	(printi a)

	(write stdout "factorial of a: " 17)
	(printi (factorial 8))

	(write stdout "\ncurrent pid: " 15)
	(printi (syscall 20 0 0 0 0 0))

	(write stdout "\n" 2)

	(defvar buf (malloc 12))
	(printi (resw 'buf))
	(setw (+ buf 0) 0)
	(setw (+ buf 4) 0)
	(setw (+ buf 8) 0)

	(exit 0)

	(write stdout "Enter your name: " 18)
	(read stdin buf 12)
	(write stdout "Your name is " 14)
	(write stdout buf 12)

	(exit 0)
)

