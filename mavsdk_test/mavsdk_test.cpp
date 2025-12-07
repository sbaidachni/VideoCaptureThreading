// introducing MAVSDK
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <unistd.h>
#include <stdio.h>
#include <set>
#include <string>


using namespace mavsdk;

int main()
{
    mavsdk::Mavsdk mavsdk{mavsdk::Mavsdk::Configuration{mavsdk::ComponentType::GroundStation}};

    mavsdk::ConnectionResult connection_result = mavsdk.add_any_connection("serial:///dev/ttyAMA0:115200");

    if (connection_result != mavsdk::ConnectionResult::Success) {
        printf("Failed to connect to MAVLINK\n");
        return 1;
    }
    printf("Connection successful!\n");

    while (mavsdk.systems().size()==0)
    {
        printf("No MAVSDK system available\n");
        sleep(1);
    }

    auto system = mavsdk.systems().at(0);
    printf("System found!\n");
    
    if (!system->is_connected()) {
        printf("MAVLINK system not connected\n");
        return 1;
    }
    printf("System connected!\n");

    // Use MAVLink Passthrough instead of MavlinkDirect for lower-level access
    auto passthrough = mavsdk::MavlinkPassthrough{system};
    printf("MavlinkPassthrough created\n");

    // Subscribe to incoming messages using passthrough
    printf("Subscribing to messages via passthrough...\n");
    int message_count = 0;
    int heartbeat_count = 0;
    int rc_channels_count = 0;
    
    passthrough.subscribe_message(
        MAVLINK_MSG_ID_HEARTBEAT,
        [&heartbeat_count](const mavlink_message_t& message) {
            heartbeat_count++;
            mavlink_heartbeat_t heartbeat;
            mavlink_msg_heartbeat_decode(&message, &heartbeat);
            printf("HEARTBEAT #%d: type=%d, autopilot=%d, base_mode=%d, system_status=%d\n",
                   heartbeat_count, heartbeat.type, heartbeat.autopilot, 
                   heartbeat.base_mode, heartbeat.system_status);
        }
    );
    
    passthrough.subscribe_message(
        MAVLINK_MSG_ID_RC_CHANNELS,
        [&rc_channels_count](const mavlink_message_t& message) {
            rc_channels_count++;
            mavlink_rc_channels_t rc;
            mavlink_msg_rc_channels_decode(&message, &rc);
            printf("RC_CHANNELS #%d: chan1=%d, chan2=%d, chan3=%d, chan4=%d, chan5=%d, chan9=%d\n",
                   rc_channels_count, rc.chan1_raw, rc.chan2_raw, rc.chan3_raw, 
                   rc.chan4_raw, rc.chan5_raw, rc.chan9_raw);
        }
    );
    
    int i=0;
    while (i++<60) 
    {
        if (i % 10 == 0) {
            printf("Waiting... %d/60 (total: %d, heartbeats: %d, RC: %d)\n", 
                   i, message_count, heartbeat_count, rc_channels_count);
        }
        sleep(1);
    }
    
    printf("\nFinal count - Total: %d, Heartbeats: %d, RC_CHANNELS: %d\n", 
           message_count, heartbeat_count, rc_channels_count);

    return 0;
}