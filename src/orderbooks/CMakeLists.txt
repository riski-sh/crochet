add_library( orderbooks coinbase.c book.c)
target_link_libraries( orderbooks pprint )
set_target_properties(orderbooks PROPERTIES RULE_MESSAGES OFF)


