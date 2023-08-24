global-incdirs-y += include
srcs-y += test1_ta.c

################################################################
#
# DTA support
#
################################################################

srcs-y += ditto_ta.c
srcs-y += func_extended.c

# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes
