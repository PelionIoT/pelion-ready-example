// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include "mbed.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "simple-mbed-cloud-client.h"
#include "key-config-manager/kcm_status.h"
#include "key-config-manager/key_config_manager.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "EthernetInterface.h"

// Placeholder to hardware that trigger events (timer, button, etc)
Ticker timer;

// Placeholder for storage
SDBlockDevice sd(PTE3, PTE1, PTE2, PTE4);
FATFileSystem fs("sd");

// Pointers to the resources that will be created in main_application().
static MbedCloudClientResource* pattern_ptr;

// Pointer to mbedClient, used for calling close function.
static SimpleMbedCloudClient *client;

static bool button_pressed = false;

void button_press() {
    button_pressed = true;
}

void pattern_updated(const char *) {
    printf("PUT received, new value: %s\n", pattern_ptr->get_value().c_str());
    // Placeholder for PUT action
}

void blink_callback(void *) {
    String pattern_str = pattern_ptr->get_value();
    const char *pattern = pattern_str.c_str();
    printf("POST received. LED pattern = %s\n", pattern);
    // Placeholder for POST action
    // The pattern is something like 500:200:500, so parse that.
}

void button_callback(const M2MBase& object, const NoticationDeliveryStatus status)
{
    printf("Button notification. Callback: (%s)\n", object.uri_path());
    // Placeholder for GET
}


int main(void)
{
    // Requires DAPLink 245+ (https://github.com/ARMmbed/DAPLink/pull/364)
    // Older versions: workaround to prevent possible deletion of credentials:
    wait(2);

    // Misc OS setup
    srand(time(NULL));

    // Placeholder for network
    EthernetInterface net;

    printf("Start Simple Mbed Cloud Client\n");

    // Initialize SD card
    int status = sd.init();
    if (status != BD_ERROR_OK) {
        printf("Failed to init SD card\r\n");
        return -1;
    }

    // Mount the file system (reformatting on failure)
    status = fs.mount(&sd);
    if (status) {
        printf("Failed to mount FAT file system, reformatting...\r\n");
        status = fs.reformat(&sd);
        if (status) {
            printf("Failed to reformat FAT file system\r\n");
            return -1;
        } else {
            printf("Reformat and mount complete\r\n");
        }
    }

    printf("Connecting to the network using Ethernet...\n");

    status = net.connect();
    if (status) {
        printf("Connection to Network Failed %d!\n", status);
        return -1;
    } else {
        const char *ip_addr  = net.get_ip_address();
        printf("Connected successfully\n");
        printf("IP address %s\n", ip_addr);
    }

    SimpleMbedCloudClient mbedClient(&net);
    // Save pointer to mbedClient so that other functions can access it.
    client = &mbedClient;

    status = mbedClient.init();
    if (status) {
        return -1;
    }

    printf("Client initialized\r\n");

    // Mbed Cloud Client resource setup
    MbedCloudClientResource *button = mbedClient.create_resource("3200/0/5501", "button_resource");
    button->set_value("0");
    button->methods(M2MMethod::GET);
    button->observable(true);
    button->attach_notification_callback(button_callback);

    MbedCloudClientResource *pattern = mbedClient.create_resource("3201/0/5853", "pattern_resource");
    pattern->set_value("500:500:500:500");
    pattern->methods(M2MMethod::GET | M2MMethod::PUT);
    pattern->attach_put_callback(pattern_updated);
    pattern_ptr = pattern;

    MbedCloudClientResource *blink = mbedClient.create_resource("3201/0/5850", "blink_resource");
    blink->methods(M2MMethod::POST);
    blink->attach_post_callback(blink_callback);

    mbedClient.register_and_connect();

    // Wait for client to finish registering
    while (!mbedClient.is_client_registered()) {
        wait_ms(100);
    }

    // Placeholder for callback to update local resource when GET comes.
    timer.attach(&button_press, 5.0);

    // Check if client is registering or registered, if true sleep and repeat.
    while (mbedClient.is_register_called()) {
        static int button_count = 0;
        wait_ms(100);

        if (button_pressed) {
            button_pressed = false;
            printf("Simulated button clicked %d times\r\n", ++button_count);
            button->set_value(button_count);
        }
    }

    // Client unregistered, exit program.
    return 0;
}
