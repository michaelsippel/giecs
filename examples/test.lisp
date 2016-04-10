#! ../vm_linux64


# thats a funky map replacement :D
nop
	(declare a 20)
	(declare test '(eval (if (eq (resw a) 20) '(printi 100) '(printi 200))))
	(declare printa '(printi (resw a)))

	(printa)
	(test)

	(setw a 23)
	(printa)
	(test)

	(exit 0)

