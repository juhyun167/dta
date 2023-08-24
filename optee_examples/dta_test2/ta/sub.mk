global-incdirs-y += include
srcs-y += dta_test2.c
srcs-y += func_aes.c

################################################################
#
# DTA support
#
################################################################

srcs-y += ditto_ta.c
srcs-y += func_extended.c

# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes