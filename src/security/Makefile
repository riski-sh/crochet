security.a:
	mkdir -p $(OBJDIR)/security/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c chart.c -o $(OBJDIR)/security/chart.c
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c security.c -o $(OBJDIR)/security/security.c
	ar rcs $(OBJDIR)/security/security.a $(OBJDIR)/security/chart.o $(OBJDIR)/security/security.o
