#! ../vm_linux64

lmap nop
(
	(declare factorial '(function (n)
		(eval (if (eq n 1)
			'(quote 1)
			'(* (factorial (- n 1)) n)
		))
	))

	(printi (factorial 8))
	(printi 123)
	(exit 0)
)

