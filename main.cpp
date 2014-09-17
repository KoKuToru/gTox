#include "toxcore/toxcore/tox.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>

#define SLEEP_TIME 50000
#define BOOTSTRAP_ADDRESS "144.76.60.215"
#define BOOTSTRAP_PORT 33445
const char* BOOTSTRAP_KEY = "04119E835DF3E78BACF0F84235B300546AF8B936F035185E2A8E9E0A67C8924F";

void hex_string_to_bin(const char *in, uint8_t *out) {
    int pos = 0;
    while(in[pos] != 0) {
        char dummy[3] = {0};
        dummy[0] = in[pos+0];
        dummy[1] = in[pos+1];
        out[pos/2] = strtol(dummy, 0, 16);
        pos += 2;
    }
}

Tox *my_tox;
void MyFriendRequestCallback(Tox*tox, const uint8_t* public_key, const uint8_t* data, uint16_t length, void* userdata) {
  std::cout << "got request" << std::endl;
  tox_add_friend_norequest(my_tox, public_key);
}

void MyFriendMessageCallback(Tox *tox, int32_t friendnumber, const uint8_t * message, uint16_t length, void *userdata) {
   std::cout << message << std::endl;
}


int main(int argc, const char *argv[]) {
    uint8_t *pub_key = new uint8_t[TOX_CLIENT_ID_SIZE];
    hex_string_to_bin(BOOTSTRAP_KEY, pub_key);

    Tox_Options options;
    options.ipv6enabled = true;
    options.udp_disabled = false;
    options.proxy_enabled = false;
    
    my_tox = tox_new(&options);
    
    std::ifstream oi("save.bin");
    uint32_t fileSizei;
    oi.read((char*)&fileSizei, 4);
    uint8_t *datai = new uint8_t[fileSizei];
    oi.read((char*)datai, fileSizei);
    
    tox_load(my_tox, datai, fileSizei);

    /* Register the callbacks */
    tox_callback_friend_request(my_tox, MyFriendRequestCallback, NULL);
    tox_callback_friend_message(my_tox, MyFriendMessageCallback, NULL);


    /* Define or load some user details for the sake of it */
    std::string name = "ImoutoBot";
    tox_set_name(my_tox, (const uint8_t*)name.c_str(), name.size()); // Sets the username
    //tox_set_status_message(my_tox, uint8_t *status, uint16_t length); // status message is a string the user can set
    tox_set_user_status(my_tox, TOX_USERSTATUS_NONE); // user status is pre-defined ints for "online", "offline" etc.



    if (!tox_bootstrap_from_address(my_tox, BOOTSTRAP_ADDRESS, BOOTSTRAP_PORT, (const uint8_t*)pub_key)) { // connect to a bootstrap to get into the network
        std::cout << "COULDN'T START" << std::endl;
    }
    
    
    uint32_t fileSize = tox_size(my_tox);
    uint8_t *data = new uint8_t[fileSize];
    std::ofstream o("save.bin");
    tox_save(my_tox, data);
    o.write((const char*)&fileSize, sizeof(uint32_t));
    o.write((const char*)data, fileSize);
    

uint8_t friendAddress[TOX_FRIEND_ADDRESS_SIZE];
    tox_get_address(my_tox, friendAddress);
    
    std::cout << "Tox ID: ";
    for(int i = 0; i < TOX_FRIEND_ADDRESS_SIZE; ++i) {
     int a = friendAddress[i];
     std::cout << ("0123456789ABCDEF"[(a>>4)&0x0F]);
     std::cout << ("0123456789ABCDEF"[a&0x0F]);
     }
     std::cout << std::endl;

    std::cout << "STARTED" << std::endl;

    while (1) {
        tox_do(my_tox); // will call the callback functions defined and registered


        usleep(SLEEP_TIME); // sleep for cpu usage, tox_wait() can be used instead for blocking
    }


    tox_kill(my_tox);
    return 0;
}
