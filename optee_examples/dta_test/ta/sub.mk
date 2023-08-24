global-incdirs-y += include
srcs-y += dta_test.c
srcs-y += func_handlers.c
srcs-y += func_handlers_time.c
srcs-y += func_handlers_random.c
srcs-y += func_handlers_storage.c
srcs-y += func_handlers_acipher.c

################################################################
#
# DTA support
#
################################################################

srcs-y += ditto_ta.c
srcs-y += func_extended.c

# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes