/*
 * USBASPLOADER.C
 * 
 * This is the USBaspLoader Software-Exit feature
 * hostside software.
 * It can be used to trigger an USBaspLoader to
 * exit bootloader mode and startup its firmware.
 * 
 * This is version 20150808T1200ZSB
 *
 * Stephan Baerwolf (matrixstorm@gmx.de), Schwansee 2015
 * for http://matrixstorm.com/avr/tinyusbboard/
 * (please contact me at least before commercial use)
 */

#define __MAIN_C_d98e3c32094c4feba6311b74c71782c6

#include "usbhelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>
#include <errno.h>

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */
#define USBDEV_VENDOR_NAME	"www.fischl.de"
#define USBDEV_DEVICE_NAME	"USBasp"
#include "usbasp.h"

#ifdef MYDEBUG
#	define fdebugf(file, args...) fprintf(file, ##args)
#else
#	define fdebugf(file, args...) 
#endif


static void usage(char *name)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s help\n\n", name);
    fprintf(stderr, "  %s list\n\n", name);
    fprintf(stderr, "  %s exit [busID [deviceID]]\n\n", name);
}


static int listdevicesMAP(const struct usb_bus *bus, const struct usb_device *dev, const usb_dev_handle *handle, const uint16_t vendor, const uint16_t product, const char *vendorName, const char *productName, const char *serial, void* userparameter) {
  if (strcmp(vendorName, USBDEV_VENDOR_NAME) == 0) {
    if (strcmp(productName, USBDEV_DEVICE_NAME) == 0) {
      int *counter = userparameter;
      (*counter)++;
      fprintf(stderr, "\tBUS %03i DEVICE %03i: ID %04x:%04x USBasp device (programmer or loader)\n", (int)bus->location, (int)dev->devnum, (int)vendor, (int)product);
    }
  }
  return 0;
}
static void listdevices(const uint32_t busID, const uint8_t deviceID) {
  int counter = 0;
    fprintf(stderr, "compatible devices:\n");

    /* the following must return 0 - ensure that with mapping function */
    usbScanDevice(NULL, busID, deviceID, USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT, listdevicesMAP, &counter);

    if (counter<=0) {
      fprintf(stderr, "\tno suitable device(s) found!\n");
    }
    fflush(stderr);
}


int main(int argc, char **argv) {
    usb_dev_handle      *handle = NULL;
    unsigned char       buffer[32 + 1 + 1];
    int                 nBytes, txReset=0;
    uint32_t		busID;
    uint32_t		deviceID;

    usbhelper_initialize();
    
    if (argc > 2) busID=atol(argv[2]);
    else busID=0xffffffff;
    
    if (argc > 3) deviceID=atoi(argv[3]);
    else deviceID=0xff;

    
    if (argc < 2) {
      usage(argv[0]);
    }else if(strcmp(argv[1], "help") == 0){
      usage(argv[0]);
    }else if(strcmp(argv[1], "list") == 0){
      listdevices(busID, deviceID);
    }else if(strcmp(argv[1], "exit") == 0){
      // connect to the USBASP
      if(usbOpenDevice(&handle, busID, deviceID, USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT, USBDEV_VENDOR_NAME, USBDEV_DEVICE_NAME, "") != 0) {
	  fprintf(stderr, "Could not find USB device \"usbasp\" with vid=0x%x pid=0x%x\n", USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT);
	  exit(1);
      }
      fdebugf(stderr,"%i: connecting to usbasp...\n",__LINE__);
      nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, USBASP_FUNC_CONNECT, 0, 0, (char *)buffer, sizeof(buffer), 5000);
      if (nBytes >= 0) {
	fdebugf(stderr,"%i: Bytes = %i\n", __LINE__, nBytes);
	fdebugf(stderr,"%i: requesting capablilies...\n",__LINE__);
	nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, USBASP_FUNC_GETCAPABILITIES, 0x0000, 0x0000, (char *)buffer, sizeof(buffer), 5000);
	if (nBytes >= 0) {
	  fdebugf(stderr,"%i: Bytes = %i\n", __LINE__, nBytes);
	  if (nBytes==0) {
	    fdebugf(stderr,"%i: sending resetting sequence...\n",__LINE__);
	    nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, USBASP_FUNC_TRANSMIT, 0xffff, 0xffff, (char *)buffer, sizeof(buffer), 5000);
	    if (nBytes >= 0) {
	      txReset++;
#ifdef MYDEBUG
	      int i;
	      fdebugf(stderr,"%i: Bytes = %i, ( ", __LINE__, nBytes);
	      for (i=0;i<nBytes;i++) fdebugf(stderr,"0x%02x ",buffer[nBytes-(i+1)]);
	      fdebugf(stderr,")\n");
#endif
	    } else fdebugf(stderr,"%i: %s\n",__LINE__, strerror(errno));
	  } else {
#ifdef MYDEBUG
	    int i;
	    fdebugf(stderr, "%i: no USBaspLoader ( ", __LINE__);
	    for (i=0;i<nBytes;i++) fdebugf(stderr,"0x%02x ",buffer[nBytes-(i+1)]);
	    fdebugf(stderr,")\n");
#endif
	  }
	  fdebugf(stderr,"%i: disconnecting...\n",__LINE__);
	  nBytes = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, USBASP_FUNC_DISCONNECT, 0, 0, (char *)buffer, sizeof(buffer), 5000);
	  if (nBytes >= 0) {
	    if (txReset) {
	      fdebugf(stderr,"%i: Bytes = %i\n", __LINE__, nBytes);
	    } else {
	      fdebugf(stderr,"%i: Bytes = %i (no exitable usbasp detected)\n", __LINE__, nBytes);
	    }
	  } else {
	    // in case the reset command was send - the usb pipe will be broken right after disconnecting-cmd
	    if (txReset) {
	      fdebugf(stderr,"%i: Bytes = %i (usbasploader left)\n", __LINE__, nBytes);
	    } else {
	      fdebugf(stderr,"%i: %s\n",__LINE__, strerror(errno));
	    }
	  }
	} else fdebugf(stderr,"%i: %s\n",__LINE__, strerror(errno));
      } else fdebugf(stderr,"%i: %s\n",__LINE__, strerror(errno));
     } else {
          usage(argv[0]);
     }

    if (handle) usb_close(handle);
    handle=NULL;
    usbhelper_finalize();
    return 0;
}
