#! ../vm_linux64

progn
(
;	(load "../stdlib/system.lisp")
;	(load "../stdlib/string.lisp")
;	(load "../stdlib/malloc.lisp")

;	(defun factorial (n)
;		(if (eqw n 1)
;			'1
;			(* (factorial (- n 1)) n)))

;	(prints "factorial of 8 is ")
;	(printi (factorial 8))

;	(defvar pid (fork))

;	(if (eqw pid 0)
;		(progn
;			((prints "I'm child!\n")
;			(exit 123)))
;		(progn
;			((prints "I'm parent!\n")
;			(defvar status 0)
;			(waitpid pid 'status 0))))

;	(exit 0)
	(declare 0 test '(function (x) (+ x x)))
	(printi (test 4))

	(syscall 1 0 0 0 0 0)
)

