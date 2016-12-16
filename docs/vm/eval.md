# Evaluation
This is the part where the pure data stored in the virtual memory is interpreted as programs.

## Low-Level Functions
Low-Level-functions are basic operations for the concatenated programs and are implemented as procedures that can be executed natively on the host machine.
These functions are marked with a length of -1, followed by a host-sized-word used by the vm as pointer to the actual function.

## The EVAL-function
The eval-function applies parameters partially recursively until a low-level function is reached and then called.

### Algorithm

The first word on the given stackframe is interpreted as the function to call. All following data after this pointer is considered as arguments passed to the called function.
Having the pointer to the function to call, we can look at the "definition" of the function which is structured like the following:

1. first word: following length (of 2.+3.)
2. second word: function to call
3. rest: parameters for the function

Since lowlevel functions have a length of -1, we can check this special case and call the lowlevel function and give the current stack frame as parameters.
Otherwise we copy the "definition" (without the length) to the current stackframe and evaluate the whole thing again until we reach a lowlevel function.

#### Pseudocode
```
eval (stack):
	pop pointer to function from stack:
		ptr <- stack.pop

	check if function is lowlevel:
		l <- resolve_word(ptr)
		if l = -1:
			call the lowlevel function
		else:
			push l bytes from (ptr+wordsize) to stack
			eval(stack)
```

### Examples
TODO
