#undef __STRICT_ANSI__
#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)
#include <windows.h>
#include <fstream>
#include "callbacks.h"
#include "packets.h"
unsigned int initial_time;
unsigned int time;

class Airship{
public:
    unsigned long long GUID = 550351414016;
    unsigned int b1 = 0;
    unsigned int field_c = 555509968;
    unsigned long long int pos_x = 550361296274;
    unsigned long long int pos_y = 550355927040;
    unsigned long long int pos_z = 13039826;
    float vel_x = 0.0;
    float vel_y = 0.0;
    float vel_z = 0.0;
    float rot = 270.0;
    unsigned long long int station_x = 550358024192;
    unsigned long long int station_y = 550355927040;
    unsigned long long int station_z = 11730944;
    float pathrotation = 270.0;
    unsigned int field_54 = 573148656;
    unsigned long long int target_x = 550361300992;
    unsigned long long int target_y = 550359203840;
    unsigned long long int target_z = 13041664;
    uint8_t flight_stage = 2;
    uint8_t unk_char1 = 0x9F;
    uint8_t unk_char2 = 0x0A;
    uint8_t unk_char3 = 0x4D;
    unsigned int b2 = 0;
};

Airship airship;



unsigned int base;
void SendAirshipPacket(SOCKET socket){
    unsigned int pkt_size = 0x4 + 0x4 + 0x78;

    unsigned int pid = 3;
    unsigned int len = 1;


    char buf[pkt_size] = {0};

    unsigned int current_time = timeGetTime();
    unsigned int time_delta_milliseconds = current_time - time;

    float BLOCKS_PER_SECOND = 10.0;
    float BLOCKS_PER_MILLISECOND = BLOCKS_PER_SECOND / 1000;
    float UNITS_PER_BLOCK = 65536.0;

    if ((current_time-initial_time) > 90000){
        airship.pos_x += (long long int)(UNITS_PER_BLOCK * BLOCKS_PER_MILLISECOND * (float)time_delta_milliseconds);
        airship.vel_x = BLOCKS_PER_SECOND;
        airship.vel_y = 0;
        airship.vel_z = 0;

    }

    time = current_time;

    memcpy(buf, (char*)&pid, 4);
    memcpy(buf+4, (char*)&len, 4);
    memcpy(buf+8, (char*)&airship, 0x78);




//    char buf[pkt_size] = {0x03, 0x00, 0x00, 0x00, //pid
//                          0x01, 0x00, 0x00, 0x00, //len
//
//    0x00, 0x23, 0x80, 0x23, 0x80, 0x00, 0x00, 0x00, //GUID
//
//    0x00, 0x00, 0x00, 0x00, //unk_bool 1
//    0xD0, 0x68, 0x1C, 0x21,  //field_c
//
//    0x92, 0xED, 0x16, 0x24, 0x80, 0x00, 0x00, 0x00, //pos x
//    0x00, 0x00, 0xC5, 0x23, 0x80, 0x00, 0x00, 0x00, //y
//    0xD2, 0xF8, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, //z
//
//    0x66, 0x16, 0x15, 0xC1, 0x00, 0x00, 0x00, 0x00, 0xAE, 0x4E, 0x68, 0xC0, //vel
//
//    0x64, 0x00, 0x87, 0x43, //rot
//
//    0x00, 0x00, 0xE5, 0x23, 0x80, 0x00, 0x00, 0x00, //station
//    0x00, 0x00, 0xC5, 0x23, 0x80, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0xB3, 0x00, 0x00, 0x00, 0x00, 0x00,
//
//    0x00, 0x00, 0x87, 0x43, //pathrotation
//    0xF0, 0x8D, 0x29, 0x22, //field_54
//
//    0x00, 0x00, 0x17, 0x24, 0x80, 0x00, 0x00, 0x00, //target
//    0x00, 0x00, 0xF7, 0x23, 0x80, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00,
//
//    0x02, 0x9F, 0x0A, 0x4D,//flight stage +
//
//    0x00, 0x00, 0x00, 0x00 //unk bool 3
//
//    };

    AddPacket(socket, buf, pkt_size);
}

void __stdcall no_shenanigans HandleReadyToSend(SOCKET socket){
    SendAirshipPacket(socket);

    SendQueuedPackets(socket);
}


void __stdcall no_shenanigans HandlePlayerDisconnect(SOCKET socket){
    PurgeSocket(socket);
}


DWORD WINAPI no_shenanigans RegisterCallbacks(){

        RegisterCallback("RegisterReadyToSendCallback", HandleReadyToSend);
        RegisterCallback("RegisterPlayerDisconnectCallback", HandlePlayerDisconnect);

        return 0;
}

extern "C" no_shenanigans bool APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (unsigned int)GetModuleHandle(NULL);
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            time = timeGetTime();
            initial_time = time;
            airship.pos_z += 65536 * 150;
            PacketsInit();

            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return true;
}
