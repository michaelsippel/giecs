#! ../vm_linux64

lmap nop
(
	(declare defmacro '(macro (name plist def)
		(declare name '(macro plist def))))

	(declare defun '(macro (name plist def)
		(declare name '(function plist def))))

	(defun factorial (n)
		(eval (if (eq n 1)
			'(quote 1)
			'(* (factorial (- n 1)) n)
		))
	)

	(defun read (fd buf len)
		(syscall 3 fd buf len 0 0))

	(defun write (fd buf len)
		(syscall 4 fd buf len 0 0))

	(write 1 "Hallo\n" 7)

	(declare a 8)
	(prints "\ndeclared a: ")
	(printi a)

	(prints "factorial of a: ")
	(printi (factorial 8))

	(prints "\ncurrent pid: ")
	(printi (syscall 20 0 0 0 0 0))

	(prints "\n")

	(exit 0)
)

