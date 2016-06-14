
progn
(
	(defun strlen (str)
		(eval (if (eqb (resb str) (eqw 0 1))
			'(quote 0)
			'(+ (strlen (+ str 1)) 1))))

	(defun prints (str)
		(write stdout str (strlen str)))
)

