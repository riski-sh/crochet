add_library( exchanges coinbase.c oanda.c )
target_link_libraries( exchanges httpws )
