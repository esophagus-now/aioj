ifeq ($(OS),Windows_NT)
	CSC = csc
else
	CSC = mcs
endif

aioj.exe: aioj.cs
	$(CSC) -t:exe -out:aioj.exe aioj.cs

primordialj.exe: arthur_whitney_deobfuscated.c
	gcc -g -Wall -fno-diagnostics-show-caret -o primordialj.exe arthur_whitney_deobfuscated.c

clean:
	rm -rf *.exe
	rm -rf *.dll
