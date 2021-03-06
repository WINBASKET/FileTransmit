/*
 * This file is generated by gdbus-codegen, do not modify it.
 *
 * The license of this code is the same as for the D-Bus interface description
 * it was derived from. Note that it links to GLib, so must comply with the
 * LGPL linking clauses.
 */

#ifndef __FTDBUS_H__
#define __FTDBUS_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.hwangsaeul.Gaeul2.FileTransmitAgent */

#define FT_DBUS_TYPE_FILE_TRANSMIT_AGENT (ft_dbus_file_transmit_agent_get_type ())
#define FT_DBUS_FILE_TRANSMIT_AGENT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT, ftDbusFileTransmitAgent))
#define FT_DBUS_IS_FILE_TRANSMIT_AGENT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT))
#define FT_DBUS_FILE_TRANSMIT_AGENT_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT, ftDbusFileTransmitAgentIface))

struct _ftDbusFileTransmitAgent;
typedef struct _ftDbusFileTransmitAgent ftDbusFileTransmitAgent;
typedef struct _ftDbusFileTransmitAgentIface ftDbusFileTransmitAgentIface;

struct _ftDbusFileTransmitAgentIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_send) (
    ftDbusFileTransmitAgent *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_path);

  void (*receive) (
    ftDbusFileTransmitAgent *object,
    const gchar *arg_path);

};

GType ft_dbus_file_transmit_agent_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *ft_dbus_file_transmit_agent_interface_info (void);
guint ft_dbus_file_transmit_agent_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void ft_dbus_file_transmit_agent_complete_send (
    ftDbusFileTransmitAgent *object,
    GDBusMethodInvocation *invocation,
    guint id);



/* D-Bus signal emissions functions: */
void ft_dbus_file_transmit_agent_emit_receive (
    ftDbusFileTransmitAgent *object,
    const gchar *arg_path);



/* D-Bus method calls: */
void ft_dbus_file_transmit_agent_call_send (
    ftDbusFileTransmitAgent *proxy,
    const gchar *arg_path,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean ft_dbus_file_transmit_agent_call_send_finish (
    ftDbusFileTransmitAgent *proxy,
    guint *out_id,
    GAsyncResult *res,
    GError **error);

gboolean ft_dbus_file_transmit_agent_call_send_sync (
    ftDbusFileTransmitAgent *proxy,
    const gchar *arg_path,
    guint *out_id,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_PROXY (ft_dbus_file_transmit_agent_proxy_get_type ())
#define FT_DBUS_FILE_TRANSMIT_AGENT_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_PROXY, ftDbusFileTransmitAgentProxy))
#define FT_DBUS_FILE_TRANSMIT_AGENT_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_PROXY, ftDbusFileTransmitAgentProxyClass))
#define FT_DBUS_FILE_TRANSMIT_AGENT_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_PROXY, ftDbusFileTransmitAgentProxyClass))
#define FT_DBUS_IS_FILE_TRANSMIT_AGENT_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_PROXY))
#define FT_DBUS_IS_FILE_TRANSMIT_AGENT_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_PROXY))

typedef struct _ftDbusFileTransmitAgentProxy ftDbusFileTransmitAgentProxy;
typedef struct _ftDbusFileTransmitAgentProxyClass ftDbusFileTransmitAgentProxyClass;
typedef struct _ftDbusFileTransmitAgentProxyPrivate ftDbusFileTransmitAgentProxyPrivate;

struct _ftDbusFileTransmitAgentProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  ftDbusFileTransmitAgentProxyPrivate *priv;
};

struct _ftDbusFileTransmitAgentProxyClass
{
  GDBusProxyClass parent_class;
};

GType ft_dbus_file_transmit_agent_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ftDbusFileTransmitAgentProxy, g_object_unref)
#endif

void ft_dbus_file_transmit_agent_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
ftDbusFileTransmitAgent *ft_dbus_file_transmit_agent_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
ftDbusFileTransmitAgent *ft_dbus_file_transmit_agent_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void ft_dbus_file_transmit_agent_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
ftDbusFileTransmitAgent *ft_dbus_file_transmit_agent_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
ftDbusFileTransmitAgent *ft_dbus_file_transmit_agent_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_SKELETON (ft_dbus_file_transmit_agent_skeleton_get_type ())
#define FT_DBUS_FILE_TRANSMIT_AGENT_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_SKELETON, ftDbusFileTransmitAgentSkeleton))
#define FT_DBUS_FILE_TRANSMIT_AGENT_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_SKELETON, ftDbusFileTransmitAgentSkeletonClass))
#define FT_DBUS_FILE_TRANSMIT_AGENT_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_SKELETON, ftDbusFileTransmitAgentSkeletonClass))
#define FT_DBUS_IS_FILE_TRANSMIT_AGENT_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_SKELETON))
#define FT_DBUS_IS_FILE_TRANSMIT_AGENT_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), FT_DBUS_TYPE_FILE_TRANSMIT_AGENT_SKELETON))

typedef struct _ftDbusFileTransmitAgentSkeleton ftDbusFileTransmitAgentSkeleton;
typedef struct _ftDbusFileTransmitAgentSkeletonClass ftDbusFileTransmitAgentSkeletonClass;
typedef struct _ftDbusFileTransmitAgentSkeletonPrivate ftDbusFileTransmitAgentSkeletonPrivate;

struct _ftDbusFileTransmitAgentSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  ftDbusFileTransmitAgentSkeletonPrivate *priv;
};

struct _ftDbusFileTransmitAgentSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType ft_dbus_file_transmit_agent_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ftDbusFileTransmitAgentSkeleton, g_object_unref)
#endif

ftDbusFileTransmitAgent *ft_dbus_file_transmit_agent_skeleton_new (void);


G_END_DECLS

#endif /* __FTDBUS_H__ */
