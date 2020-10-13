main.bin : exchanges.a ffjson.a finmath.a globals.a hashmap.a \
	httpws.a orderbooks.a pprint.a security.a io.a
	$(CC) $(CFLAGS) $(WFLAGS) $(FFLAGS) $(IFLAGS) $(LFLAGS) \
		-o $(OBJDIR)/crochet.bin \
		main.c \
		$(OBJDIR)/exchanges/exchanges.a \
		$(OBJDIR)/ffjson/ffjson.a \
		$(OBJDIR)/finmath/finmath.a \
		$(OBJDIR)/globals/globals.a \
		$(OBJDIR)/hashmap/hashmap.a \
		$(OBJDIR)/httpws/httpws.a \
		$(OBJDIR)/orderbooks/orderbooks.a \
		$(OBJDIR)/pprint/pprint.a \
		$(OBJDIR)/security/security.a \
		$(OBJDIR)/io/io.a

exchanges.a :
	@$(MAKE) -C exchanges

ffjson.a :
	@$(MAKE) -C ffjson

finmath.a :
	@$(MAKE) -C finmath

globals.a :
	@$(MAKE) -C  globals

hashmap.a :
	@$(MAKE) -C hashmap

httpws.a :
	@$(MAKE) -C httpws

orderbooks.a :
	@$(MAKE) -C orderbooks

pprint.a :
	@$(MAKE) -C pprint

security.a :
	@$(MAKE) -C security

io.a :
	@$(MAKE) -C io
