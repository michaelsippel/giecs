progn
(
	(defun exit (err)
		(syscall 1 err 0 0 0 0))

	(defun read (fd buf len)
		(syscall 3 fd buf len 0 0))

	(defun write (fd buf len)
		(syscall 4 fd buf len 0 0))

	(defun open (path oflags mode)
		(syscall 5 path oflags mode 0 0))

	(defun close (fd)
		(syscall 6 fd 0 0 0 0))

	(defun getpid ()
		(syscall 20 0 0 0 0 0))

	(defvar stdin 0)
	(defvar stdout 1)
	(defvar stderr 2)

	# a very simple malloc :D
	(defvar mptr 30000)
	(defun malloc (size)
		(progn 
			((resw 'mptr)
			(setw 'mptr (+ mptr size))
		)))
)

