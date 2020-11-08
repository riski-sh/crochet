security.a:
	mkdir -p $(OBJDIR)/security/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c chart.c -o $(OBJDIR)/security/chart.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c security.c -o $(OBJDIR)/security/security.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c analysis.c -o $(OBJDIR)/security/analysis.o
	ar rcs $(OBJDIR)/security/security.a $(OBJDIR)/security/chart.o $(OBJDIR)/security/security.o $(OBJDIR)/security/analysis.o
