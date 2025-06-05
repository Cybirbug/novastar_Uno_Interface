//NOVASTAR BLACKOUT CONTROLLER
// JUNE 2025 NICK MARSTOL
// for use at the palace
//takes artnet uni 5 addy 512 and will blackout or enable the novastar 1000 screen controller
//credit due to library maintainers etc...

#include "defines.h"

#define ART_NET_PORT 6454
// Opcodes
#define ART_POLL 0x2000
#define ART_POLL_REPLY 0x2100
#define ART_DMX 0x5000
#define ART_SYNC 0x5200
#define ART_PROTOCOL_VER 14
// Buffers
#define MAX_BUFFER_ARTNET 530
// Packet
#define ART_NET_ID "Art-Net\0"

bool screenOn = true;

EthernetUDP udp;

typedef union {
    struct{
    char ID[8]; //"Art-Net"
    uint16_t OpCode; // See Doc. Table 1 - OpCodes eg. 0x5000 OpOutput / OpDmx
    uint16_t version; // 0x0e00 (aka 14)
    uint8_t seq; // monotonic counter
    uint8_t physical; // 0x00
    uint8_t subUni; // low universe (0-255)
    uint8_t net; // high universe (not used)
    uint16_t length; // data length (2 - 512)
    uint8_t data[512]; // universe data
}__attribute__((packed));
uint8_t raw[MAX_BUFFER_ARTNET];
} ArtnetDmxPacket;


ArtnetDmxPacket packet;

//55 aa 00 00 fe 00 00 00 00 00 01 00 04 00 00 13 02 00 03 00 70 56
byte normal[22] = { 0x55, 0xaa, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x13, 0x02, 0x00, 0x03, 0x00, 0x70, 0x56 };

//55 aa 00 00 fe 00 00 00 00 00 01 00 04 00 00 13 02 00 05 00 72 56
byte blackout[22] = { 0x55, 0xaa, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x13, 0x02, 0x00, 0x05, 0x00, 0x72, 0x56 };

//55 aa 00 00 fe 00 00 00 00 00 01 00 12 00 02 13 03 00 00 00 00 7e 56
byte hdmi1[23] = { 0x55, 0xaa, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0x00, 0x02, 0x13, 0x03, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x56 };
//55 aa 00 00 fe 00 00 00 00 00 01 00 12 00 02 13 03 00 02 00 00 80 56

byte dvi1[23] = { 0x55, 0xaa, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0x00, 0x02, 0x13, 0x03, 0x00, 0x00, 0x00, 0x00, 0x80, 0x56 };

// A UDP instance to let us send and receive packets over UDP
EthernetClient tcp;



void connect() {
    Serial.println("conencting...");
    if (tcp.connect("2.0.0.10", 5200)) {
        Serial.println("Connected to v1000");
    }
    else {
        Serial.println("connection failed");
    }
}

void handleArtnetDMXPacket() {
   // Serial.print("got data: ");
   // Serial.println(packet.raw[529]);
    if (packet.raw[529] > 127) {
        if (screenOn == false){
            tcp.write(normal, sizeof(normal));
            screenOn = true;
        }
    }
    else {
        if (screenOn == true) {
            tcp.write(blackout, sizeof(blackout));
            screenOn = false;
        }
    }

}

void parsePacket() {//incoming packet callback
    //Serial.println("got packet");
    udp.readBytes(packet.raw, udp.available());
    //for (int i = 0; i < MAX_BUFFER_ARTNET; i++) {
    //    Serial.print(i);
    //    Serial.print(" - ");
    //    Serial.println(packet.raw[i]);
    //}
        switch (packet.OpCode)
        {
        case ART_DMX: {
            // Serial.println("dmx packet");
            if(packet.subUni == 5) handleArtnetDMXPacket();
            break;
        }
        case ART_POLL: {
            // Serial.println("artnet poll packet");
            //replyPollPacket(packet.remoteIP());
            break;
        }
        default:
            break;
        }

}

void setup()
{
    Serial.begin(115200);
    Serial.println("starting");

    Ethernet.begin(mac[0], ip);

    Serial.println(F("\nStarting connection to server..."));
    connect();
    udp.begin(ART_NET_PORT);
  
    Serial.println("end of setup");
}

void loop()
{
    if (udp.parsePacket()) { 
       // Serial.println("udp data");
        parsePacket(); 
    }

    if (tcp.connected() == SnSR::CLOSED) {
        connect();
    }
}
