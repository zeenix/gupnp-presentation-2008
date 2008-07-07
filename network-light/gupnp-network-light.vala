using GUPnP;

public class GUPnP.NetworkLight : RootDevice {
    private Service switch_power;
    private Service dimming;

    // State variables
    bool status;       // On/Off
    uint load_level;   // Dimming level (percentage)

    construct {
        Service service;
        
        service = (Service) this.get_service (
                                    "urn:schemas-upnp-org:service:SwitchPower");
        assert (service != null);

        // Connect action handlers 
        service.action_invoked["GetStatus"] += this.on_get_status;
        service.action_invoked["SetTarget"] += this.on_get_target;
        service.action_invoked["SetTarget"] += this.on_set_target;
        
        this.switch_power = service;

        service = (Service) this.get_service (
                                    "urn:schemas-upnp-org:service:Dimming");
        assert (service != null);
        
        service.action_invoked["GetLoadLevelStatus"] +=
                                this.on_get_load_level_status;
        service.action_invoked["GetLoadLevelTarget"] +=
                                this.on_get_load_level_target;
        service.action_invoked["SetLoadLevelTarget"] +=
                                this.on_set_load_level_target;
        
        this.dimming = service;
    }

    public NetworkLight (GUPnP.Context context) {
        this.context = context;
        this.resource_factory = ResourceFactory.get_default ();
        this.relative_location = "/network-light-desc.xml";

        this.available = true;
    }

    public static int main (string[] args) {
        GUPnP.Context context;

        Thread.init (null);
        
        try {
            context = new GUPnP.Context (null, /* GLib.MainContext */
                                         null, /* Host IP          */
                                         0);   /* Host Port        */
        } catch (Error err) {
            critical (err.message);
            return 1;
        }

        print ("Running on port %u\n", context.port);

        /* Host current directory */
        context.host_path (".", "");

        /* Let there be NetworkLight! */
        NetworkLight light = new NetworkLight (context);
        assert (light != null);

        light.available = true;

        MainLoop loop = new MainLoop (null, false);
        loop.run();

        return 0;
    }

    /* Action handlers */
    void on_get_status (Service       service,
                        ServiceAction action) {
        action.set ("ResultStatus",
                    typeof (bool),
                    this.status);
        action.return ();
    }


    void on_get_target (Service       service,
                       ServiceAction  action) {
        action.set ("RetTargetValue",
                    typeof (bool),
                    this.status);
        action.return ();
    }

    void on_set_target (Service       service,
                        ServiceAction action) {
        action.get ("NewTargetValue",
                    typeof (bool),
                    out this.status);
        action.return ();

        // Notif all subscribed control points
        service.notify ("Status",
                        typeof (bool),
                        this.status);
    }

    void on_get_load_level_status (Service       service,
                                   ServiceAction action) {
        action.set ("retLoadlevelStatus",
                    typeof (uint),
                    this.load_level);
        action.return ();
    }

    void on_get_load_level_target (Service       service,
                                   ServiceAction action) {
        action.set ("retLoadlevelTarget",
                    typeof (uint),
                    this.load_level);
        action.return ();
    }

    void on_set_load_level_target (Service       service,
                                   ServiceAction action) {
        uint load_level;

        action.get ("newLoadlevelTarget",
                    typeof (uint),
                    out load_level);
        action.return ();

        this.load_level = load_level.clamp (0, 100);

        // Notif all subscribed control points
        service.notify ("LoadLevelStatus",
                        typeof (uint),
                        this.load_level);
    }
}

