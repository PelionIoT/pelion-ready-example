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

#define LED_OFF                     1

DigitalOut  led(LED_RED, LED_OFF);
InterruptIn button(BUTTON1);

// Pointers to the resources that will be created in main_application().
static MbedCloudClientResource* pattern_ptr;

// Pointer to mbedClient, used for calling close function.
static SimpleMbedCloudClient *client;

static bool button_pressed = false;

void button_press() {
    button_pressed = true;
}

void pattern_updated(const char *) {
    printf("PUT received, new value: %s\n", pattern_ptr->get_value());
}

void blink_callback(void *) {
    const char *pattern = pattern_ptr->get_value();
    printf("LED pattern = %s\n", pattern);
    // The pattern is something like 500:200:500, so parse that.
    // LED blinking is done while parsing.
    led = !led;
    while (*pattern != '\0') {
        // Wait for requested time.
        wait_ms(atoi(pattern));
        led = !led;
        // Search for next value.
        pattern = strchr(pattern, ':');
        if(!pattern) {
            break; // while
        }
        pattern++;
    }

    led = LED_OFF;
}

void button_notification_status_callback(const M2MBase& object, const NoticationDeliveryStatus status)
{
    switch(status) {
        case NOTIFICATION_STATUS_BUILD_ERROR:
            printf("Notification callback: (%s) error when building CoAP message\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_RESEND_QUEUE_FULL:
            printf("Notification callback: (%s) CoAP resend queue full\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_SENT:
            printf("Notification callback: (%s) Notification sent to server\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_DELIVERED:
            printf("Notification callback: (%s) Notification delivered\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_SEND_FAILED:
            printf("Notification callback: (%s) Notification sending failed\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_SUBSCRIBED:
            printf("Notification callback: (%s) subscribed\n", object.uri_path());
            break;
        case NOTIFICATION_STATUS_UNSUBSCRIBED:
            printf("Notification callback: (%s) subscription removed\n", object.uri_path());
            break;
        default:
            break;
    }
}

// This function is called when a POST request is received for resource 5000/0/1.
void unregister_cb(void *)
{
    printf("Unregister resource executed\n");
    client->close();
}

// This function is called when a POST request is received for resource 5000/0/2.
void factory_reset_cb(void *)
{
    printf("Factory reset resource executed\n");
    client->close();
    kcm_status_e kcm_status = kcm_factory_reset();
    if (kcm_status != KCM_STATUS_SUCCESS) {
        printf("Failed to do factory reset - %d\n", kcm_status);
    } else {
        printf("Factory reset completed. Now restart the device\n");
    }
}

int main(void)
{
    // IOTMORF-1712: DAPLINK starts the previous application during flashing a new binary
    // This is workaround to prevent possible deletion of credentials or storage corruption
    // while replacing the application binary.
    wait(2);

    // Misc OS setup
    srand(time(NULL));

    EthernetInterface net;
    SDBlockDevice sd(PTE3, PTE1, PTE2, PTE4);
    FATFileSystem fs("sd");

    printf("Start Simple Mbed Cloud Client\n");

    // Initialize button interrupt
    button.fall(&button_press);

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
    button->attach_notification(M2MMethod::GET, (void*)button_notification_status_callback);

    MbedCloudClientResource *pattern = mbedClient.create_resource("3201/0/5853", "pattern_resource");
    pattern->set_value("500:500:500:500");
    pattern->methods(M2MMethod::GET | M2MMethod::PUT);
    pattern->attach(M2MMethod::PUT, (void*)pattern_updated);
    pattern_ptr = pattern;

    MbedCloudClientResource *blink = mbedClient.create_resource("3201/0/5850", "blink_resource");
    blink->methods(M2MMethod::POST);
    blink->attach(M2MMethod::POST, (void*)blink_callback);

    MbedCloudClientResource *unregister = mbedClient.create_resource("5000/0/1", "unregister");
    unregister->methods(M2MMethod::POST);
    unregister->attach(M2MMethod::POST, (void*)unregister_cb);

    MbedCloudClientResource *factoryReset = mbedClient.create_resource("5000/0/2", "factory_reset");
    factoryReset->methods(M2MMethod::POST);
    factoryReset->attach(M2MMethod::POST, (void*)factory_reset_cb);

    mbedClient.register_and_connect();

    // Check if client is registering or registered, if true sleep and repeat.
    while (mbedClient.is_register_called()) {
        static int button_count = 0;
        wait_ms(100);

        if (button_pressed) {
            button_pressed = false;
            printf("Button clicked %d times\r\n", ++button_count);
            button->set_value(button_count);
        }
    }

    // Client unregistered, exit program.
    return 0;
}
