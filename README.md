# mbedLPC1768

For more information about licensing, see here: http://stratifylabs.co/download/

## Support

If you have any questions, please create an issueon Github.  I will answer them as soon as I can.

## Getting Started

This is the board support package for the MBED LPC1768 board.  Here is a quick guide to getting started:

- Download the Stratify Labs SDK (Eclipse, gcc compiler, and Stratify Link) from:  http://stratifylabs.co/download/
- Make sure your SDK is up to date with the latest build (link to guide coming soon)
- Download the zip file of this project (or fork the project, your choice)
- Import the project into your Eclipse workspace (if you are new to that, follow this guide: http://agile.csc.ncsu.edu/SEMaterials/tutorials/import_export/)
- Build the bootloader and the OS in Eclipse

![Preview](https://github.com/StratifyLabs/mbedLPC1768/blob/master/doc/build-options.png "Build Options")

- Connect a mini-B USB cable to the MBED and drop the bootloader file (mbedLPC1768/build_release_usb_boot/mbedLPC1768.bin) in the MBED mounted drive then press the MBED reset button (you should see one of the 4 blue leds flash a few times whent the bootloader starts)
- Now connect the LPC1768 USB port directly to your computer using the image below 

![Preview](https://github.com/StratifyLabs/mbedLPC1768/blob/master/doc/mbedLPC1768-USBConnections.JPG "mbed LPC1768 USB Connections")

- Finally, use Stratify Link to program the OS as shown below 

![Preview](https://github.com/StratifyLabs/mbedLPC1768/blob/master/doc/stratifyOS-program.png "Install Stratify OS")

Once the bootloader and OS are installed, you can proceed to install and run applications (without going through the above process a second time).

## Known Issues

### Limited Semihost Features
The MBED semihost filesystem API (for accessing "/home") is limited.  You can access the MBED filesystem using open(), read(), write(), close(), etc.  However, the API does not provide any directory access, so you can't list all the files in the directory.  A.  It is particularly bad if you have the MBED USB connected and the drive mounted.  If the drive is mounted, you will not be able to create files.

### UART Transport Layer

At one time, you could run the Stratify Link transport layer over the UART using the standard USB connection.  Changes to the MBED firmware have prevented this from working.  The MBED resets the target whenever the virtual COM port is closed.  This causes problems.  So for now, you have to directly connect to the LPC1768 USB port.  This works much faster and better than the UART anyway.
