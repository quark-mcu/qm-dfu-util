AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = dfu-util-qda

dfu_util_qda_CFLAGS = -Wall -Wextra -DUSE_QDA -I./qda/
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
		quirks.c \
		quirks.h

if WINDOWS_BUILD
dfu_util_qda_SOURCES += qda/serial_io_windows.c
dfu_util_qda_LDFLAGS = -static
else
dfu_util_qda_SOURCES += qda/serial_io.c
endif