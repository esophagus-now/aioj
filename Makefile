ifeq ($(OS),Windows_NT)
	CSC = csc
else
	CSC = mcs
endif

aioj.exe: aioj.cs
	$(CSC) -t:exe -out:aioj.exe aioj.cs
