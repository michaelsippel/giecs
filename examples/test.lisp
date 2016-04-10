#! ../vm_linux64


# thats a funky map replacement :D
nop
	(declare a 20)

	(declare end '(exit 0))

	(declare test 
		'(eval (if (eq (resw a) 0) 
					end
					nop)))

	(declare dec '(setw a (+ (resw a) -1)))

	(declare loop
		'(nop
			(printi (resw a))
			(test)
			(dec)
			(loop)))

	(loop)

