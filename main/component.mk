#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

DEFAULT_CFLAGS = 						\
		-Wall 							\
		-Werror							\
		-Werror=unused-function			\
		-Werror=unused-but-set-variable	\
		-Werror=unused-variable			\
		-Werror=deprecated-declarations \
		-D__FREERTOS_ESP32__

CXXFLAGS += -std=c++17 $(DEFAULT_CFLAGS)	
CFLAGS += -std=c11 $(DEFAULT_CFLAGS)