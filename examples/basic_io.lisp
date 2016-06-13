#! ../vm_linux64

progn
(
	(load "../stdlib/define.lisp")
	(load "../stdlib/system.lisp")

	(defvar buf1 (malloc 16))
	(defvar buf2 (malloc 16))

	(write stdout "Enter your name: " 18)
	(read stdin buf1 16)

	(write stdout "favorite color: " 17)
	(read stdin buf2 16)

	(write stdout "\n\nyour name is " 16)
	(write stdout buf1 16)
	(write stdout "your favorite color is " 23)
	(write stdout buf2 16)

	(exit 0)
)

