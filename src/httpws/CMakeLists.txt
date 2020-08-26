add_library(httpws wss.c http.c base64.c)
set_target_properties(httpws PROPERTIES RULE_MESSAGES OFF)

