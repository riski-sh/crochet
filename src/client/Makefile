client.a:
	mkdir -p $(OBJDIR)/client/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c client.c -o $(OBJDIR)/client/client.o
	ar rcs $(OBJDIR)/client/client.a $(OBJDIR)/client/client.o
