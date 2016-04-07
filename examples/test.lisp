#! ../vm_linux64


# thats a funky map replacement :D
genfn NOP -1
	(declare a 20)

	(declare end (quote (exit 0)))

	(declare test (quote
		(eval (if (eq (resw a) 0) 
					end
					NOP))))

	(declare dec (quote
		(setw a (+ (resw a) -1))))

	(declare loop (quote
		(genfn NOP -1
			(printi (resw a))
			(test)
			(dec)
			(loop))))

	(loop)

	(exit 0)

