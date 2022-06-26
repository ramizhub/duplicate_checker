DC: duplicate_checker.c
	gcc duplicate_checker.c `pkg-config --libs glib-2.0` `pkg-config --libs openssl` `pkg-config --cflags glib-2.0` `pkg-config --cflags openssl`