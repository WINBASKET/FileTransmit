#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "ftDbus.h"
#include <gtk/gtk.h>

GtkWidget *path_result;

 void Callback_Send(GtkWidget *widget, gpointer data)
{

	GtkWidget *path_in_entry = (GtkWidget *)data;


	ftDbusFileTransmitAgent *proxy;
	GError *error;
	guint *id;

	const gchar *entry_text;

	entry_text = gtk_entry_get_text(GTK_ENTRY(path_in_entry));
	
	gchar *arg_path = entry_text; //"client.c";

	g_print("Send file name : %s\n", arg_path);
	error = NULL;

	//error = NULL;
	proxy = ft_dbus_file_transmit_agent_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
			"org.hwangsaeul.Gaeul2", "/org/hwangsaeul/Gaeul2/FileTransmitAgent", NULL, &error);
	
	ft_dbus_file_transmit_agent_call_send_sync(proxy, arg_path, &id, NULL, &error);

	//gchar *tmp = *buf;
	GPOINTER_TO_INT(id);

	g_print("Srt process id: %d\n", GPOINTER_TO_INT(id));
	
	gchar *response;

    response = g_strdup_printf("file send srt pid : %d", id);

	gtk_entry_set_text(GTK_ENTRY(path_result), response);

	g_object_unref(proxy);
}

static gboolean
on_handle_dbus_receive (ftDbusFileTransmitAgent *interface, const gchar *arg_path, gpointer data)
{

	GtkWidget *path_out_entry = (GtkWidget *)data;

	gchar *response;

    response = g_strdup_printf("File download : %s", arg_path);


	gtk_entry_set_text(GTK_ENTRY(path_out_entry), response);

	g_print("recieve %s\n",arg_path);

    return TRUE;
}
static void Callback_Receive(GtkWidget *widget, gpointer data)
{
	ftDbusFileTransmitAgent *proxy;

	GtkWidget *path_out_entry = (GtkWidget *)data;

	gtk_entry_set_text(GTK_ENTRY(path_out_entry), "Listen waiting");

	GError *error;

	error = NULL;

	proxy = ft_dbus_file_transmit_agent_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
			"org.hwangsaeul.Gaeul2", "/org/hwangsaeul/Gaeul2/FileTransmitAgent", NULL, &error);
	
    //interface = ft_dbus_file_transmit_agent_skeleton_new();

	g_signal_connect(proxy, "receive", G_CALLBACK (on_handle_dbus_receive), data);
	g_print("signal connet\n");
}


static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	g_print("delete event occured\n");
	gtk_main_quit();
	return TRUE;

}

static void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();

}


int main(int argc, char *argv[])
{
	//--widget variables
	GtkWidget *window;
	GtkWidget *fixed;
	GtkWidget *send_button, *recieve_button, *path_in_entry, *path_out_entry;
	
	//---arguments are parsed
	gtk_init(&argc, &argv);

	//---create new window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	//---sets the border width of the window
	//gtk_container_set_border_width(GTK_CONTAINER(window), 100);
	gtk_window_set_title(GTK_WINDOW(window), "Dbus_Test");
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 150);
	

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), fixed);

	//when the window is given the 'deleted_evnet' signal,
	// we ask it to call the delete_event() function
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);

	//---connect the "destory" event to a signal handler
	g_signal_connect(G_OBJECT(window), "destory", G_CALLBACK(destroy), NULL);

	
	//---creates a new button with the label "funtion"
	send_button = gtk_button_new_with_label("send");	
	gtk_widget_set_size_request(send_button, 80,30);
	gtk_fixed_put(GTK_FIXED(fixed), send_button, 30, 30);  //position

	recieve_button = gtk_button_new_with_label("recieve");
	gtk_widget_set_size_request(recieve_button, 80,30);
	gtk_fixed_put(GTK_FIXED(fixed), recieve_button, 30, 100); 

	//---creates a new edit with the label "funtion"
	path_in_entry = gtk_entry_new();
	gtk_entry_set_max_length((GtkEntry *)path_in_entry, 100);
	gtk_widget_set_size_request(path_in_entry, 300,30);
	gtk_fixed_put(GTK_FIXED(fixed), path_in_entry, 150, 30);
		

	path_out_entry = gtk_entry_new();
	gtk_entry_set_max_length((GtkEntry *)path_out_entry, 100);
	gtk_widget_set_size_request(path_out_entry, 300,30);
	gtk_fixed_put(GTK_FIXED(fixed), path_out_entry, 150, 100);

	path_result = gtk_entry_new();
	gtk_entry_set_max_length((GtkEntry *)path_result, 100);
	gtk_widget_set_size_request(path_result, 300,30);
	gtk_fixed_put(GTK_FIXED(fixed), path_result, 150, 150);
	

	//---when the button receives the "clicked" signal, it will call the function(no argument, NULL)
	g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(Callback_Send), path_in_entry);
	
	//---when the button receives the "clicked" signal, it will call the function(no argument, NULL)
	g_signal_connect(G_OBJECT(recieve_button), "clicked", G_CALLBACK(Callback_Receive), path_out_entry);

	gtk_entry_set_text(GTK_ENTRY(path_in_entry), "client.c");

	gtk_widget_show_all(window);

	gtk_main();

	
	/*
	ftDbusFt *proxy;
	GError *error;
	gchar **buf;

	error = NULL;
	proxy = ft_dbus_ft_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
			"org.cwj.test", "/org/cwj/test/ft", NULL, &error);

	ft_dbus_ft_call_register_sync(proxy, 1, buf, NULL, &error);
	g_print("resp: %s\n", *buf);

	g_object_unref(proxy);
	*/
	return 0;
}
