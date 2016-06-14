#! ../vm_linux64

progn
(
	(load "../stdlib/define.lisp")
	(load "../stdlib/system.lisp")
	(load "../stdlib/string.lisp")

	(defun factorial (n)
		(eval (if (eqw n 1)
			'(quote 1)
			'(* (factorial (- n 1)) n))))

	(prints "factorial of 8 is ")
	(printi (factorial 8))

	(exit 0)
)

