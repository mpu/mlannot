# mlannot - OCaml annotation extractor for VIM #

COMPILING + INSTALLING
----------------------
It is as usual.

    make; sudo make install

USAGE
-----
You can use it directly with the following command:

    mlannot file char_number

Where char\_number is the position of the expression you want to know
the type and where file can be either a file name ending with .ml or
.annot (this is useful when writing dirty vim scripts).

If you want to use it in a vim script you should add some dirty lines
like this in your vimrc:

    " VIM integration with mlannot
    function! GetMLAnnot()
      let l:filepos = line2byte(line('.')) + col('.') - 1
      let l:msg     = system("mlannot " . bufname('%') . " " .  l:filepos)
      echomsg substitute(l:msg, "\n", '', 'g') " remove trailing newline
    endfunction
    au FileType omlet nm ;t :call GetMLAnnot()<CR>

I assume here that you have installed mlannot in your path, so you
can call it simply, otherwise, modify the script above accordingly.

TODO
----
It would be nice to be able to delete some annotations from the
output.

BUGS
----
What's this ?

You can report them via github.

vim: set tw=70:
