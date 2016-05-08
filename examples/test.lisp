#! ../vm_linux64

lmap nop
(
	(declare factorial '(function (n)
		(eval (if (eq n 1)
			'(quote 1)
			'(* (factorial (- n 1)) n)
		))
	))


	(declare a 8)
	(prints "\ndeclared a: ")
	(printi a)

	(prints "factorial of a: ")
	(printi (factorial 8))

	(prints "\n")

	(exit 0)
)

