progn
(
;	(load "system.lisp")
	
	(defun strlen (str)
		(if (eqb (resb str) (eqw 0 1))
			'0
			(+ (strlen (+ str 1)) 1)))

	(defun prints (str)
		(write stdout str (strlen str)))
)

