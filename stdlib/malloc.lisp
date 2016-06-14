
progn
(
	# a very simple malloc :D
	(defvar mptr 30000)
	(defun malloc (size)
		(progn 
			((resw 'mptr)
			(setw 'mptr (+ mptr size)))))
)

