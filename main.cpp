#undef __STRICT_ANSI__
#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)
#include <windows.h>
#include <fstream>
#include "callbacks.h"
#include "packets.h"
#include <vector>
#include <math.h>
unsigned int initial_time;
unsigned int time;

float degrees_to_radians(float degrees){
    return degrees * 0.017453298768179;
}
float radians_to_degrees(float radians){
    return radians * 57.295800025114424159;
}

class Vector3Float{
public:
    float x;
    float y;
    float z;
    Vector3Float(float _x, float _y, float _z){
        this->x = _x;
        this->y = _y;
        this->z = _z;
    }
    float Magnitude(){
        return sqrt( pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2) );
    }
    float Yaw(){
        return radians_to_degrees( atan2(this->y, this->x) );
    }


};

class Vector3Long{
public:
    long long int x;
    long long int y;
    long long int z;
    Vector3Long(long long int  _x, long long int _y, long long int _z){
        this->x = _x;
        this->y = _y;
        this->z = _z;
    }
    float Magnitude(){
        return sqrt( pow((float)(this->x), 2) + pow((float)(this->y), 2) + pow((float)(this->z), 2) );
    }
    float Yaw(){
        return radians_to_degrees( atan2((float)(this->y), (float)(this->x)) );
    }

};



class Airship{
public:
    unsigned long long GUID = 550351414016;
    unsigned int b1 = 0;
    unsigned int field_c = 555509968;
    Vector3Long position = Vector3Long(550361296274, 550355927040, 13039826);
    Vector3Float velocity = Vector3Float(10,0,0);
    float rot = 270.0;
    Vector3Long station_pos = Vector3Long(550358024192, 550355927040, 11730944);
    float pathrotation = 270.0;
    unsigned int field_54 = 573148656;
    Vector3Long target_pos = Vector3Long(550361300992, 550359203840, 13041664);
    uint8_t flight_stage = 3;
    uint8_t unk_char1 = 0x9F;
    uint8_t unk_char2 = 0x0A;
    uint8_t unk_char3 = 0x4D;
    unsigned int b2 = 0;
    Airship(){}
    Airship(unsigned long long guid, unsigned long long int x, unsigned long long int y, signed long long int z){
        this->GUID = guid;
        this->position.x = x;
        this->position.y = y;
        this->position.z = z;
    }
    void Tick(unsigned int time_delta_milliseconds){
        const float UNITS_PER_BLOCK = 65536.0;

//        float blocks_per_second_x = this->velocity.x;
//        float blocks_per_millisecond_x = blocks_per_second_x / 1000;
//        this->position.x += (long long int)(UNITS_PER_BLOCK * blocks_per_millisecond_x * (float)time_delta_milliseconds);

        this->position.x += (long long int)(UNITS_PER_BLOCK * (this->velocity.x / 1000.0) * (float)time_delta_milliseconds);
        this->position.y += (long long int)(UNITS_PER_BLOCK * (this->velocity.y / 1000.0) * (float)time_delta_milliseconds);
        this->position.z += (long long int)(UNITS_PER_BLOCK * (this->velocity.z / 1000.0) * (float)time_delta_milliseconds);
    }
    void Turn(float degrees){
        float velocity_magnitude = this->velocity.Magnitude();
        float velocity_angle = this->velocity.Yaw();
        velocity_angle += degrees;
        this->velocity.x = cos(degrees_to_radians(velocity_angle)) * velocity_magnitude;
        this->velocity.y = sin(degrees_to_radians(velocity_angle)) * velocity_magnitude;
        this->rot = velocity_angle - 90; //Cube World's angles seem funny.

    }
};

class AirshipPacket{
public:
    int PACKET_ID = 3;
    int LEN = 1;
    Airship airship;
    AirshipPacket(Airship* a){
        memcpy(&this->airship, a, 0x78);
    }
    void Add(SOCKET socket){
        AddPacket(socket, (char*)this, sizeof(AirshipPacket));
    }
};

std::vector<Airship*> airships;

void CreateAirships(unsigned int num){
    unsigned long long start_guid = 0x50000000000L;
    unsigned long long spawn_x = 550361296274;
    unsigned long long spawn_y = 550355927040;

    signed long long spawn_z = 13039826 + 65536 * 150;

    for (unsigned int i = 0; i<num; i++){
        unsigned long long this_guid = start_guid + i;


        long long x_offset = rand() % 65536;
        x_offset -= 65536/2;
        x_offset *= 8000;

        long long y_offset = rand() % 65536;
        y_offset -= 65536/2;
        y_offset *= 8000;

        long long z_offset = rand() % 65536;
        //z_offset -= 65536/2;
        z_offset *= 10;

        float angle = (float)(rand() % 360);

        Airship* a = new Airship(this_guid, spawn_x+x_offset, spawn_y+y_offset, spawn_z+z_offset);

        a->Turn(angle);

        airships.push_back(a);
    }
}


unsigned int base;
void SendAirshipPacket(SOCKET socket){
    unsigned int current_time = timeGetTime();
    unsigned int time_delta_milliseconds = current_time - time;
    for (Airship* airship : airships){

        //printf("%llu %llu %llu\n", airship->position.x, airship->position.y, airship->position.z );

        if ((current_time-initial_time) > 15000){
            float angle = rand() % 10000;
            angle /= 10000; //between 0 and 1
            angle -= 0.5; //between -0.5 and 0.5
            angle /= 10.0; //between -0.05 and 0.05
            airship->Turn(angle*(float)time_delta_milliseconds);
            airship->Tick(time_delta_milliseconds);
        }

        time = current_time;


        //AirshipPacket a_pkt = AirshipPacket(airship);
        //a_pkt.Add(socket);
    }


    int len = airships.size();
    int pid = 3;
    int i = 0;
    char packet_data[8 + sizeof(Airship) * len] = {0};
    memcpy(packet_data, &pid, 4);
    memcpy(packet_data+4, &len, 4);
    for (Airship* a_ptr : airships){
        memcpy(packet_data + 8 + sizeof(Airship) * (i%len), a_ptr, sizeof(Airship));
        i++;
    }
    AddPacket(socket, packet_data, 8 + sizeof(Airship) * len);





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
            srand(4526);
            CreateAirships(50);


            time = timeGetTime();
            initial_time = time;
            PacketsInit();

            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return true;
}
