exchanges.a:
	mkdir -p $(OBJDIR)/exchanges/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c coinbase.c -o $(OBJDIR)/exchanges/coinbase.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c exchanges.c -o $(OBJDIR)/exchanges/exchanges.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c oanda.c -o $(OBJDIR)/exchanges/oanda.o
	ar rcs $(OBJDIR)/exchanges/exchanges.a $(OBJDIR)/exchanges/coinbase.o $(OBJDIR)/exchanges/exchanges.o $(OBJDIR)/exchanges/oanda.o
