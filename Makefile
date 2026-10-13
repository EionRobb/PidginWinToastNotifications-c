PIDGIN_TREE_TOP ?= pidgin
WIN32_DEV_TOP ?= win32-dev

WIN32_CC ?= i686-w64-mingw32-gcc
WIN32_COMPILER ?= $(WIN32_CC)
WIN32_WINDRES ?= i686-w64-mingw32-windres
WIN32_DLLTOOL ?= i686-w64-mingw32-dlltool

WIN32_DEV_DIR ?= $(WIN32_DEV_TOP)
WIN32_PIDGIN_DIR ?= $(PIDGIN_TREE_TOP)

ifeq ($(OS),Windows_NT)
WIN32_DEV_DIR = ../win32-dev
WIN32_COMPILER = ${WIN32_DEV_DIR}/mingw-4.7.2/bin/gcc
WIN32_WINDRES = ${WIN32_DEV_DIR}/mingw-4.7.2/bin/windres
WIN32_DLLTOOL = ${WIN32_DEV_DIR}/mingw-4.7.2/bin/dlltool
WIN32_PIDGIN_DIR = ../pidgin-2.10.11

PROGFILES32 = ${ProgramFiles(x86)}
ifndef PROGFILES32
PROGFILES32 = $(PROGRAMFILES)
endif
endif

LIBPURPLE_CFLAGS = -I${WIN32_PIDGIN_DIR}/libpurple -I${WIN32_PIDGIN_DIR}/libpurple/win32 -DPURPLE_PLUGINS -DENABLE_NLS -DHAVE_ZLIB
PIDGIN_CFLAGS = -I${WIN32_PIDGIN_DIR}/pidgin -I${WIN32_PIDGIN_DIR}/pidgin/win32
GLIB_CFLAGS = -I${WIN32_DEV_DIR}/gtk_2_0/include/glib-2.0 -I${WIN32_DEV_DIR}/gtk_2_0/lib/glib-2.0/include

WIN32_CFLAGS = -I${WIN32_DEV_DIR}/gtk_2_0/include/glib-2.0 -I${WIN32_PIDGIN_DIR}/libpurple/win32 -I${WIN32_PIDGIN_DIR}/pidgin/win32 -I${WIN32_DEV_DIR}/gtk_2_0/include -I${WIN32_DEV_DIR}/gtk_2_0/include/glib-2.0 -I${WIN32_DEV_DIR}/gtk_2_0/lib/glib-2.0/include -Wno-format -I/usr/include/wine/windows -Wno-write-strings -Wno-strict-aliasing -I${WIN32_DEV_DIR}/gtk_2_0/include/gtk-2.0 -I${WIN32_DEV_DIR}/gtk_2_0/include/cairo -I${WIN32_DEV_DIR}/gtk_2_0/include/atk-1.0 -I${WIN32_DEV_DIR}/gtk_2_0/include/pango-1.0 -I${WIN32_DEV_DIR}/gtk_2_0/lib/gtk-2.0/include -Wno-attributes
WIN32_LIBS = -L${WIN32_DEV_DIR}/gtk_2_0/lib -L${WIN32_PIDGIN_DIR}/libpurple -L${WIN32_PIDGIN_DIR}/pidgin -lglib-2.0 -lgobject-2.0 -lgtk-win32-2.0 -lgdk-win32-2.0 -lintl -lws2_32 -lpurple -lole32 -lshell32 -luuid -L. -lpidgin -lgdk_pixbuf-2.0 -lgdi32

TARGET = PidginWinToastNotifications-eion.dll

all: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): Plugin/PidginWinToastNotifications.c Plugin/win_toast.c Plugin/PidginWinToastNotifications.h Plugin/win_toast.h
	${WIN32_COMPILER} -std=gnu99 ${LIBPURPLE_CFLAGS} ${PIDGIN_CFLAGS} ${GLIB_CFLAGS} ${WIN32_CFLAGS} -Wall -IPlugin -g -ggdb -O2 -pipe Plugin/PidginWinToastNotifications.c Plugin/win_toast.c -o $@ -shared -static-libgcc ${WIN32_LIBS} -mms-bitfields -Wl,--enable-auto-image-base -Wl,--warn-once

install: $(TARGET)
	install -m 0755 -p $(TARGET) "$(PROGFILES32)/Pidgin/plugins"
