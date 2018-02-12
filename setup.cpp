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


///////////
// INCLUDES
///////////

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "mbed.h"
#include "setup.h"
#include "memory_tests.h"
#include "EthernetInterface.h" // Networking interface include
#include "simple-mbed-cloud-client.h"
#include "pal.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "factory_configurator_client.h"

#include "SDBlockDevice.h"
#include "FATFileSystem.h"

// Network interface
#include "EthernetInterface.h"
EthernetInterface eth;

////////////////////////////////////////
// PLATFORM SPECIFIC DEFINES & FUNCTIONS
////////////////////////////////////////
#define DEFAULT_FIRMWARE_PATH       "/sd/firmware"

#include "mbed_trace.h"
#define TRACE_GROUP "exam"


// Define led on/off
#ifdef TARGET_STM
#define LED_ON (true)
#else // #ifdef TARGET_STM
#define LED_ON (false)
#endif // #ifdef TARGET_STM

#define LED_OFF (!LED_ON)

DigitalOut  led(MBED_CONF_APP_LED_PINNAME, LED_OFF);
InterruptIn button(MBED_CONF_APP_BUTTON_PINNAME);

static bool button_pressed = false;
static void button_press(void);

// Block device and Filesystem
SDBlockDevice sd(PTE3, PTE1, PTE2, PTE4);
FATFileSystem fs("sd", &sd);


Thread resource_thread;

NetworkInterface* network_interface = NULL;


void button_press(void)
{
    button_pressed = true;
}

/////////////////////////
// SETUP.H IMPLEMENTATION
/////////////////////////
int initPlatform()
{
    /* Explicit declaration to catch Block Device initialization errors. */
    int sd_ret = sd.init();

    if(sd_ret != BD_ERROR_OK) {
        tr_error("initPlatform() - sd.init() failed with %d\n", sd_ret);
        printf("SD card initialization failed. Verify that SD-card is attached.\n");
        return -1;
    }
    tr_debug("initPlatform() - BlockDevice init OK.\n");

    if(MBED_CONF_APP_BUTTON_PINNAME != NC) {
        button.fall(&button_press);
    }

    return 0;
}

bool rmFirmwareImages()
{
    palStatus_t status = PAL_SUCCESS;
    status = pal_fsRmFiles(DEFAULT_FIRMWARE_PATH);
    if(status == PAL_SUCCESS) {
        printf("Firmware storage erased.\n");
    } else if (status == PAL_ERR_FS_NO_PATH) {
        printf("Firmware path not found/does not exist.\n");
    } else {
        printf("Firmware storage erasing failed with %" PRId32, status);
        return false;
    }
    return true;
}

int reformat_storage()
{
    int reformat_result = -1;
    printf("Autoformatting the storage.\n");
    if (1/*sd*/) {
        // TODO
        // reformat_result = fs.reformat(sd);
        if (reformat_result != 0) {
            printf("Autoformatting failed with error %d\n", reformat_result);
        }
    }
    return reformat_result;
}

int run_application(int(*function)(void))
{
    // application_init() runs the following initializations:
    //  1. trace initialization
    //  2. platform initialization
    //  3. print memory statistics if MBED_HEAP_STATS_ENABLED is defined
    //  4. FCC initialization.
    if (!application_init()) {
        printf("Initialization failed, exiting application!\n");
        return 1;
    }
    return function();
}

// Helper function, could be moved someone sd
void print_MAC(NetworkInterface* network_interface, bool log_messages) {
#if MBED_CONF_APP_NETWORK_INTERFACE != CELLULAR_ONBOARD
    const char *mac_addr = network_interface->get_mac_address();
    if (mac_addr == NULL) {
        if (log_messages) {
            printf("ERROR - No MAC address\n");
        }
        return;
    }
    if (log_messages) {
        printf("MAC address %s\n", mac_addr);
    }
#endif
}

bool init_connection()
{
    int connect_success = -1;
    bool log_messages = 1;

    srand(time(NULL));

    printf("Using Ethernet\n");

    network_interface = &eth;
    connect_success = eth.connect();

    if(connect_success == 0) {
        if (log_messages) {
            printf("Connected to Network successfully\n");
            print_MAC(network_interface, log_messages);
        }
    } else {
        if (log_messages) {
            print_MAC(network_interface, log_messages);
            printf("Connection to Network Failed %d!\n", connect_success);
        }
        return NULL;
    }
    const char *ip_addr  = network_interface->get_ip_address();
    if (ip_addr == NULL) {
        if (log_messages) {
            printf("ERROR - No IP address\n");
        }
        return NULL;
    }

    if (log_messages) {
        printf("IP address %s\n", ip_addr);
    }

    if(network_interface == NULL) {
        return false;
    }
    return true;
}

void* get_network_interface()
{
    return network_interface;
}


void toggle_led(void)
{
    if (MBED_CONF_APP_LED_PINNAME != NC) {
        led = !led;
    }
    else {
        printf("Virtual LED toggled\n");
    }
}

void led_off(void)
{
    if (MBED_CONF_APP_LED_PINNAME != NC) {
        led = LED_OFF;
    }
    else {
        printf("Virtual LED off\n");
    }
}

uint8_t button_clicked(void)
{
    if (button_pressed) {
        button_pressed = false;
        return true;
    }
    return false;
}

void do_wait(int timeout_ms)
{
    wait_ms(timeout_ms);
}


static bool application_init_mbed_trace(void)
{
    // Create mutex for tracing to avoid broken lines in logs
    if(!mbed_trace_helper_create_mutex()) {
        printf("ERROR - Mutex creation for mbed_trace failed!\n");
        return 1;
    }

    // Initialize mbed trace
    mbed_trace_init();
    mbed_trace_mutex_wait_function_set(mbed_trace_helper_mutex_wait);
    mbed_trace_mutex_release_function_set(mbed_trace_helper_mutex_release);

    return 0;
}

static void reset_storage(void)
{
    printf("Resets storage to an empty state.\n");
    fcc_status_e delete_status = fcc_storage_delete();
    if (delete_status != FCC_STATUS_SUCCESS) {
        printf("Failed to delete storage - %d\n", delete_status);
    }
}

static bool application_init_fcc(void)
{
    fcc_status_e status = fcc_init();
    if(status != FCC_STATUS_SUCCESS) {
        printf("fcc_init failed with status %d! - exit\n", status);
        return 1;
    }

    // This is designed to simplify user-experience by auto-formatting the
    // primary storage if no valid certificates exist.
    // This should never be used for any kind of production devices.
#ifndef MBED_CONF_APP_MCC_NO_AUTO_FORMAT
    status = fcc_verify_device_configured_4mbed_cloud();
    if (status != FCC_STATUS_SUCCESS) {
        if (reformat_storage() != 0) {
            return 1;
        }
    reset_storage();
    }
#endif

    // Resets storage to an empty state.
    // Use this function when you want to clear storage from all the factory-tool generated data and user data.
    // After this operation device must be injected again by using factory tool or developer certificate.
#ifdef RESET_STORAGE
    reset_storage();
#endif

    // Deletes existing firmware images from storage.
    // This deletes any existing firmware images during application startup.
    // This compilation flag is currently implemented only for mbed OS.
#ifdef RESET_FIRMWARE
    bool status_erase = rmFirmwareImages();
    if(status_erase == false) {
        return 1;
    }
#endif

#if MBED_CONF_APP_DEVELOPER_MODE == 1
    printf("Start developer flow\n");
    status = fcc_developer_flow();
    if (status == FCC_STATUS_KCM_FILE_EXIST_ERROR) {
        printf("Developer credentials already exists\n");
    } else if (status != FCC_STATUS_SUCCESS) {
        printf("Failed to load developer credentials - exit\n");
        return 1;
    }
#endif
    status = fcc_verify_device_configured_4mbed_cloud();
    if (status != FCC_STATUS_SUCCESS) {
        printf("Device not configured for mbed Cloud - exit\n");
        return 1;
    }

    return 0;
}

bool application_init(void)
{
    if (application_init_mbed_trace() != 0) {
        printf("Failed initializing mbed trace\n" );
        return false;
    }

    if(initPlatform() != 0) {
       printf("ERROR - initPlatform() failed!\n");
       return false;
    }

    printf("Start Simple Mbed Cloud Client\n");

    if (application_init_fcc() != 0) {
        printf("Failed initializing FCC\n" );
        return false;
    }

    return true;
}
