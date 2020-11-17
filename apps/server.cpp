#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <srt.h>
#include <srt/udt.h>
#include <iostream>
#include <iterator>
#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <csignal>
#include <thread>
#include <chrono>
#include <cassert>
#include <sys/stat.h>

#include "ftDbus.h"

static gboolean
on_handle_hello_world (ftDbusFt *interface, GDBusMethodInvocation *invocation,
					const gint mode, gpointer user_data)
{
	gchar *response;
	response = g_strdup_printf("Hello world %d!!.", mode);
	ft_dbus_ft_complete_register(interface, invocation, response);
	g_print("%s\n", response);
	g_free (response);
	return TRUE;
}

static void
on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	ftDbusFt *interface;
	GError *error;

	interface = ft_dbus_ft_skeleton_new();
	g_signal_connect (interface, "handle-register", G_CALLBACK (on_handle_hello_world), NULL);
	error = NULL;
	!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface), connection, "/org/cwj/test/ft", &error);
}

int main()
{
	GMainLoop *loop;

	{
		srt_logging::LogLevel::type loglevel = srt_logging::LogLevel::error;
		srt_setloglevel(loglevel);
	}

	loop = g_main_loop_new (NULL, FALSE);

	g_bus_own_name(G_BUS_TYPE_SESSION, "org.cwj.test", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
				on_name_acquired, NULL, NULL, NULL);

	g_main_loop_run (loop);

	return 0;
}
