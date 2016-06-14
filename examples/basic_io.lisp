#! ../vm_linux64

progn
(
	(load "../stdlib/define.lisp")
	(load "../stdlib/system.lisp")
	(load "../stdlib/string.lisp")
	(load "../stdlib/malloc.lisp")

	(defvar buf1 (malloc 16))
	(defvar buf2 (malloc 16))

	(prints "Enter your name: ")
	(read stdin buf1 16)

	(prints "favorite color: ")
	(read stdin buf2 16)

	(prints "\n\nyour name is ")
	(prints buf1)
	(prints "your favorite color is ")
	(prints buf2)

	(exit 0)
)

