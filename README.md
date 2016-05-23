# mbedLPC1768

This is the board support package for the MBED LPC1768 board.  Here is a quick guide to getting started:

1.  Download the Stratify Labs SDK (Eclipse, gcc compiler, and Stratify Link) from:  http://stratifylabs.co/download/
2.  Download the zip file of this project (or fork the project, your choice)
3.  Import the project into your Eclipse workspace (if you are new to that, follow this guide: http://agile.csc.ncsu.edu/SEMaterials/tutorials/import_export/)
4.  Build the bootloader and the OS in Eclipse
5.  Connect a mini-B USB cable to the MBED and drop the bootloader file (mbedLPC1768/build_release_usb_boot/mbedLPC1768.bin) in the MBED mounted drive then press the MBED reset button (you should see one of the 4 blue leds flash a few times whent the bootloader starts)
6.  Now connect the LPC1768 USB port directly to your computer using the image below ![Preview](https://github.com/StratifyLabs/mbedLPC1768/blob/master/doc/mbedLPC1768-USBConnections.JPG "mbed LPC1768 USB Connections")
7.  Finally, use Stratify Link to program the OS as shown below


