finmath.a:
	mkdir -p $(OBJDIR)/finmath/
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) -c base_conversion.c -o $(OBJDIR)/finmath/base_conversion.o
	ar rcs $(OBJDIR)/finmath/finmath.a $(OBJDIR)/finmath/base_conversion.o
