CC = cl
L = link
CFLAGS = /nologo /W3 /DDLL_EXPORTS

build: so_stdio.obj
    $(L) /nologo /dll /out:so_stdio.dll so_stdio.obj

so_stdio.obj: so_stdio.c
    $(CC) $(CFLAGS) /c $**

clean:
    del /Q /F *.obj *.lib *.dll