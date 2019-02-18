# Pelion Device Ready example - template application

(aka Simple Mbed Cloud Client template)

## Overview

This is a template application to showcase device management capabilities. It demonstrates how to create a simple application that can connect to the Pelion IoT Platform service, register resources and get ready to receive a firmware update.

It's intended to be forked and customized to add platform-specific features (such as sensors and actuators) and configure the connectivity and storage to work **out-of-the-box**. The template application works in **developer mode** by default.

There is a mirror version of the stable (master) template application on [this location](https://os.mbed.com/teams/mbed-os-examples/code/pelion-ready-example) to facilitate the fork and publish on https://os.mbed.com.

## Board specific example applications

  There are a number of applications that make usage of the Simple Pelion DM Client library.

  The Pelion [Quick-Start](https://cloud.mbed.com/quick-start) is an initiative to support Mbed Partner's platforms while delivering a great User Experience to Mbed Developers.

## Getting started with the application

This is a summary of the process for developers to get started and get a device connected to Pelion Device Management.

### Using Mbed Online IDE

1. Import the application into the Online IDE.
2. Add the API key to establish connection.
3. Install the developer certificate.
4. Compile and program.

### Using Mbed CLI

1. Import the application into your desktop:

    ```
    mbed import https://github.com/ARMmbed/pelion-ready-example
    cd pelion-ready-example
    ```

2. Configure the API key for your Pelion Portal account.

     If you don't have an API key available, then login in [Pelion IoT Platform portal](https://portal.mbedcloud.com/), navigate to 'Access Management', 'API keys' and create a new one. Then specify the API key as global `mbed` configuration:

    ```
    mbed config -G CLOUD_SDK_API_KEY <your-api-key>
    ```

3. Install the device management certificate:

    ```
    mbed dm init -d "company.com" --model-name "product-model" -q --force
    ```

4. Compile and program:

    ```
    mbed compile -t <toolchain> -m <target> -f
    ```

#### Update the application logic

The template example uses a ticker object to periodically fire a software interrupt to simulate button presses. Let’s say you want to make an actual button press.

By default, there is a Ticker object, which fires every five seconds and invokes a callback function:

```cpp
Ticker timer;
timer.attach(eventQueue.event(&fake_button_press), 5.0);
```

This callback function changes the `button_res` resource:

```cpp
void fake_button_press() {
    int v = button_res->get_value_int() + 1;

    button_res->set_value(v);

    printf("Simulated button clicked %d times\n", v);
}
```

If you want to change this to an actual button, here is how to do it:

1. Remove:

    ```cpp
    Ticker timer;
    timer.attach(eventQueue.event(&fake_button_press), 5.0);
    ```

2. Declare an `InterruptIn` object on the button, and attach the callback function to the `fall` handler:

    ```cpp
    InterruptIn btn(BUTTON1);
    btn.fall(eventQueue.event(&fake_button_press), 5.0);
    ```

3. Rename `fake_button_press` to `real_button_press`.

## Enabling firmware updates

Mbed OS 5.10 and Mbed CLI 1.8 simplifies the process to enable and perform Firmware Updates. Here is a summary on how to configure the device and verify its correct behaviour.

For full documentation about bootloaders and firmware update, read the following documents:

- [Introduccion to bootloaders](https://os.mbed.com/docs/latest/porting/bootloader.html)
- [Creating and using a bootloader](https://os.mbed.com/docs/latest/tutorials/bootloader.html)
- [Bootloader configuration in Mbed OS](https://os.mbed.com/docs/latest/tools/configuring-tools.html)
- [Mbed Bootloader for Pelion Device Management Client](https://github.com/ARMmbed/mbed-bootloader)
- [Updating devices with Arm Mbed CLI](https://os.mbed.com/docs/latest/tools/cli-update.html)

This is a summary to use Arm Mbed OS managed bootloaders.

#### Verifying that firmware update works

Follow these steps to generate a manifest, compile and perform a firmware update of your device:

1. Configure the API key for your Pelion account.

     If you don't have an API key available, then login in [Pelion IoT Platform portal](https://portal.mbedcloud.com/), navigate to 'Access Management', 'API keys' and create a new one. Then specify the API key as global `mbed` configuration:

    ```
    mbed config -G CLOUD_SDK_API_KEY <your-api-key>
    ```

2. Initialize the device management feature:

    ```
    mbed dm init -d "company.com" --model-name "product-model" -q --force
    ```

3. Compile the application, include the firware update credentials generated before, merge with the bootloader and program the device:

    ```
    mbed compile -t <toolchain> -m <target> -c -f
    ```

4. Open a serial terminal, verify the application boots and is able to register to the Device Management service. Write down the `<endpoint ID>`, as it's required to identify the device to perform a firmware update.

5. Update the firmware of the device through Mbed CLI:

    ```
    mbed dm update device -D <device ID> -t <toolchain> -m <target>
    ```

    Inspect the logs on the device to see the update progress. It should look similar to:

    ```
    Firmware download requested
    Authorization granted
    Downloading: [+++- ] 6 %
    ```

    When the download completes, the firmware is verified. If everything is OK, the firmware update is applied, the device reboots and attemps to connect to the Device Management service again. The `<endpoint ID>` should be preserved.

## Automated testing

The Simple Pelion Client provides Greentea tests to confirm your platform works as expected. The network and storage configuration is already defined in Mbed OS 5.10, but you may want to override the configuration in `mbed_app.json`.

For details on Simple Pelion Client testing, refer to the documentation [here](https://github.com/ARMmbed/simple-mbed-cloud-client#testing).

This template application contains a working application and tests passing for the `K64F` and `K66F` platforms.