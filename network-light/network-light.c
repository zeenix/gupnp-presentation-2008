#include <libgupnp/gupnp-root-device.h>
#include <libgupnp/gupnp-service.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>

#define DESCRIPTION_DOC "network-light-desc.xml"
#define DIMMING_SERVICE "urn:schemas-upnp-org:service:Dimming:1"
#define SWITCH_SERVICE "urn:schemas-upnp-org:service:SwitchPower:1"

/* State variables */
gboolean light_status;
gint     light_load_level;

/* Action handlers */
void
on_get_status (GUPnPService       *service,
               GUPnPServiceAction *action,
               gpointer            user_data)
{
        gupnp_service_action_set (action,
                                  "ResultStatus",
                                  G_TYPE_BOOLEAN,
                                  light_status,
                                  NULL);

        gupnp_service_action_return (action);
}

void
on_get_target (GUPnPService       *service,
               GUPnPServiceAction *action,
               gpointer            user_data)
{
        gupnp_service_action_set (action,
                                  "RetTargetValue",
                                  G_TYPE_BOOLEAN,
                                  light_status,
                                  NULL);

        gupnp_service_action_return (action);
}

void
on_set_target (GUPnPService       *service,
               GUPnPServiceAction *action,
               gpointer            user_data)
{
        gupnp_service_action_get (action,
                                  "NewTargetValue",
                                  G_TYPE_BOOLEAN,
                                  &light_status,
                                  NULL);
        gupnp_service_action_return (action);
        
        gupnp_service_notify (GUPNP_SERVICE (service),
                              "Status",
                              G_TYPE_BOOLEAN,
                              light_status,
                              NULL);
}

void
on_get_load_level_status (GUPnPService       *service,
                          GUPnPServiceAction *action,
                          gpointer            user_data)
{
        gupnp_service_action_set (action,
                                  "retLoadlevelStatus",
                                  G_TYPE_UINT,
                                  light_load_level,
                                  NULL);

        gupnp_service_action_return (action);
}

void
on_get_load_level_target (GUPnPService       *service,
                          GUPnPServiceAction *action,
                          gpointer            user_data)
{
        gupnp_service_action_set (action,
                                  "retLoadlevelTarget",
                                  G_TYPE_UINT,
                                  light_load_level,
                                  NULL);

        gupnp_service_action_return (action);
}

void
on_set_load_level_target (GUPnPService       *service,
                          GUPnPServiceAction *action,
                          gpointer            user_data)
{
        guint load_level;

        gupnp_service_action_get (action,
                                  "newLoadlevelTarget",
                                  G_TYPE_UINT,
                                  &load_level,
                                  NULL);
        gupnp_service_action_return (action);

        light_load_level = CLAMP (load_level, 0, 100);
        
        gupnp_service_notify (service,
                              "LoadLevelStatus",
                              G_TYPE_UINT,
                              light_load_level,
                              NULL);

}

gint
main (void)
{
        GUPnPContext     *context;
        GUPnPRootDevice  *dev;
        GUPnPServiceInfo *switch_power;
        GUPnPServiceInfo *dimming;
        GMainLoop        *main_loop;
        xmlDoc           *doc;

        g_thread_init (NULL);
        g_type_init ();

        main_loop = g_main_loop_new (NULL, FALSE);
        
        context = gupnp_context_new (NULL,      /* GMainContext */
                                     NULL,      /* Host IP      */
                                     0,         /* Host Port    */
                                     NULL);     /* GError       */
        gupnp_context_host_path (context,
                                 ".",           /* Local path      */
                                 "");           /* Web Server path */

        /* Create root device */
        doc = xmlParseFile (DESCRIPTION_DOC);
        dev = gupnp_root_device_new (context, doc, "/" DESCRIPTION_DOC);

        /* Free doc when root device is destroyed */
        g_object_weak_ref (G_OBJECT (dev), (GWeakNotify) xmlFreeDoc, doc);

        /* Connect the SwitchPower actions */
        switch_power = gupnp_device_info_get_service (GUPNP_DEVICE_INFO (dev),
                                                      SWITCH_SERVICE);
        gupnp_service_signals_autoconnect (GUPNP_SERVICE (switch_power),
                                           NULL,
                                           NULL);

        /* Connect the Dimming actions */
        dimming = gupnp_device_info_get_service (GUPNP_DEVICE_INFO (dev),
                                                 DIMMING_SERVICE);
        gupnp_service_signals_autoconnect (GUPNP_SERVICE (dimming),
                                           NULL,
                                           NULL);

        /* Run */
        gupnp_root_device_set_available (dev, TRUE);

        g_main_loop_run (main_loop);
        g_main_loop_unref (main_loop);

        g_object_unref (switch_power);
        g_object_unref (dimming);
        g_object_unref (dev);
        g_object_unref (context);
}

