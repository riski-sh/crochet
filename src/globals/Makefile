globals.a:
	mkdir -p $(OBJDIR)/globals/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c globals.c -o $(OBJDIR)/globals/globals.o
	ar rcs $(OBJDIR)/globals/globals.a $(OBJDIR)/globals/globals.o
