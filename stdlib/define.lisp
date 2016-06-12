progn
(
	(declare 0 defvar '(macro (name val)
		(declare 0 name val)))

	(declare 0 defmacro '(macro (name plist def)
		(declare 0 name '(macro plist def))))

	(declare 0 defun '(macro (name plist def)
		(declare 0 name '(function plist def))))
)

