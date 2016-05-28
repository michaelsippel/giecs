#! ../vm_linux64

progn
(
	(declare defmacro '(macro (name plist def)
		(declare name '(macro plist def))))

	(declare defun '(macro (name plist def)
		(declare name '(function plist def))))

	(defun read (fd buf len)
		(syscall 3 fd buf len 0 0))

	(defun write (fd buf len)
		(syscall 4 fd buf len 0 0))

	(defun open (path oflags mode)
		(syscall 5 path oflags mode 0 0))

	(defun close (fd)
		(syscall 6 fd 0 0 0 0))

	(declare stdin 0)
	(declare stdout 1)
	(declare stderr 2)

	(declare mptr 30000)
	(defun malloc (size)
		(lmap nop
			(setw 'mptr (+ mptr size))
			(return mptr)))

	(defun factorial (n)
		(eval (if (eq n 1)
			'(quote 1)
			'(* (factorial (- n 1)) n)
		))
	)

	(declare a 8)
	(write stdout "\ndeclared a: " 14)
	(printi a)

	(write stdout "factorial of a: " 17)
	(printi (factorial 8))

	(write stdout "\ncurrent pid: " 15)
	(printi (syscall 20 0 0 0 0 0))

	(write stdout "\n" 2)

	(declare buf mptr)
	(setw (+ buf 0) 0)
	(setw (+ buf 4) 0)
	(setw (+ buf 8) 0)

#	(write stdout "Enter your name: " 18)
#	(read stdin buf 12)
#	(write stdout "Your name is " 14)
#	(write stdout buf 12)

	(exit 0)
)

