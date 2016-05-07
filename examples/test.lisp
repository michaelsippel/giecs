#! ../vm_linux64

lmap nop
(
	(declare factorial '(function (n)
		(eval (if (eq (resw n) 1)
			'(quote 1)
			'(*
				(resw n)
				(factorial (+ (resw n) -1)))))))


	(printi (factorial 8))
	(printi 123)
	(exit 0)
)

