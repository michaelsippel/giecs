progn
(
	(declare 0 defvar '(macro (name val)
		(declare 0 name val)))

	(declare 0 defmacro '(macro (name plist def)
		(defvar name '(macro plist def))))

	(declare 0 defun '(macro (name plist def)
		(defvar name '(function plist def))))
)

