add_library( exchanges coinbase.c oanda.c )
target_link_libraries( exchanges httpws )
set_target_properties( exchanges PROPERTIES RULE_MESSAGES OFF)
