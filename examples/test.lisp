#! ../vm_linux64


# thats a funky map replacement :D
nop
	(declare test '(function (x)
		(eval (if (eq (resw x) 0)
			'(printi 1234)
			'(test (+ (resw x) -1))))))

	(test 30)
	(exit 0)

