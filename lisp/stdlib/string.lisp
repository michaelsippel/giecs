progn
(
;	(load "system.lisp")

	(defun strlen (str)
		(if (eqb (resb str) (resb "\0"))
			'0
			(+ (strlen (+ str 1)) 1)))

	(defun strcpy (dest src)
		(if (eqb (resb src) (resb "\0"))
			(setb dest (resb src))
			(progn
				((setb dest (resb src))
				(strcpy (+ dest 1) (+ src 1))))))

	(defun prints (str)
		(write stdout str (strlen str)))
)

