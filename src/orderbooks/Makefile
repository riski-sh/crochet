orderbooks.a:
	mkdir -p $(OBJDIR)/orderbooks/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c book.c -o $(OBJDIR)/orderbooks/book.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c coinbase.c -o $(OBJDIR)/orderbooks/coinbase.o
	ar rcs $(OBJDIR)/orderbooks/orderbooks.a $(OBJDIR)/orderbooks/book.o $(OBJDIR)/orderbooks/coinbase.o
