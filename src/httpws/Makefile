httpws.a:
	mkdir -p $(OBJDIR)/httpws/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c base64.c -o $(OBJDIR)/httpws/base64.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c http11.c -o $(OBJDIR)/httpws/http11.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c session.c -o $(OBJDIR)/httpws/session.o
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c wss.c -o $(OBJDIR)/httpws/wss.o
	ar rcs $(OBJDIR)/httpws/httpws.a $(OBJDIR)/httpws/base64.o $(OBJDIR)/httpws/http11.o $(OBJDIR)/httpws/session.o $(OBJDIR)/httpws/wss.o
