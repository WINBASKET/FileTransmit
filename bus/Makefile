.PHONY: all

#=============================================================================
# gen bus interface & sample source

FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0 gtk+-2.0)

# gen stub
#gen:
#	gdbus-codegen --generate-c-code ftDbus --c-namespace ftDbus --interface-prefix org.hwangsaeul.Gaeul2 org.hwangsaeul.Gaeul2.FileTransmitAgent.xml


all: dbus_client_example

dbus_client_example: dbus_client_example.o ftDbus.o
	gcc -o $@ $^ $(FLAGS)
dbus_client_example.o: dbus_client_example.c
	gcc -o $@ $^ -c $(FLAGS)
ftDbus.o: ftDbus.c
	gcc -o $@ $^ -c $(FLAGS)

gen : 
	gdbus-codegen --generate-c-code ftDbus --c-namespace ftDbus --interface-prefix org.hwangsaeul.Gaeul2 org.hwangsaeul.Gaeul2.FileTransmitAgent.xml

clean:
	rm -f dbus_client_example.o dbus_client_example
	rm -f ftDbus.*




