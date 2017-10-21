progn
(
	;;; basic macro stuff
	(declare 0 defvar '(macro (name val)
		(declare 0 name val)))

	(declare 0 defmacro '(macro (name plist def)
		(declare 0 name '(macro plist def))))

	(declare 0 defun '(macro (name plist def)
		(declare 0 name '(function plist def))))

	(defmacro if (c c1 c2)
		(eval (oif c 'c1 'c2)))

	;;; syscalls
	(defun exit (err)
		(syscall 1 err 0 0 0 0))

	(defun fork ()
		(syscall 2 0 0 0 0 0))

	(defun read (fd buf len)
		(syscall 3 fd buf len 0 0))

	(defun write (fd buf len)
		(syscall 4 fd buf len 0 0))

	(defun open (path oflags mode)
		(syscall 5 path oflags mode 0 0))

	(defun close (fd)
		(syscall 6 fd 0 0 0 0))

	(defun waitpid (pid status options)
		(syscall 7 pid status options 0 0))

	(defun getpid ()
		(syscall 20 0 0 0 0 0))

	(defvar stdin 0)
	(defvar stdout 1)
	(defvar stderr 2)
)

