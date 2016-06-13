#! ../vm_linux64

progn
(
	(load "../stdlib/define.lisp")
	(load "../stdlib/system.lisp")

	(defun factorial (n)
		(eval (if (eq n 1)
			'(quote 1)
			'(* (factorial (- n 1)) n))))

	(write stdout "factorial of 8 is "	18)
	(printi (factorial 8))

	(exit 0)
)

