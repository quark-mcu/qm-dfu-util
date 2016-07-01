AUTOMAKE_OPTIONS = subdir-objects
AM_CFLAGS = -Wall -Wextra
bin_PROGRAMS = dfu-util-qda

dfu_util_qda_CFLAGS = -DUSE_QDA -I./qda/
dfu_util_qda_SOURCES = main.c \
		portable.h \
		dfu_load.c \
		dfu_load.h \
		dfu_util_qda.c \
		dfu_util_qda.h \
		usb_dfu.h \
		dfu_file.c \
		dfu_file.h \
		qda/qda.c \
		qda/xmodem.c \
		qda/serial_io.c \
		quirks.c \
		quirks.h
