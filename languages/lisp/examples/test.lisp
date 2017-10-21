#! ../vm_linux64

progn
(
	(load "../stdlib/system.lisp")
;	(load "../stdlib/string.lisp")
;	(load "../stdlib/malloc.lisp")

	(defun square (x) (* x x))
	(printi (square 4))

;	(defun factorial (n)
;		(if (eqw n 1)
;			'1
;			(* (factorial (- n 1)) n)))
;		(eval (oif (eqw n 1)
;			'(quote 1)
;			'(* (factorial (- n 1)) n)
;		))
;	)

;	(printi (factorial 1))

;	(defvar pid (fork))
;
;	(if (eqw pid 0)
;		(progn
;			((prints "I'm child!\n")
;			(exit 123)))
;		(progn
;			((prints "I'm parent!\n")
;			(defvar status 0)
;			(waitpid pid 'status 0))))

	(exit 0)
)

