add_library( exchanges exchanges.c coinbase.c oanda.c)
target_link_libraries( exchanges httpws security)
