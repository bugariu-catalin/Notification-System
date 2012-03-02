#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "usb.h"
#include "opendevice.h"
#include "usbdrv/usbconfig.h"  /* device's VID/PID and names */
#include "test1.h"

static void usage(char *name)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s on ....... turn on\n", name);
    fprintf(stderr, "  %s off ...... turn off\n", name);
    fprintf(stderr, "  %s status ... ask current status\n", name);
    fprintf(stderr, "  %s sound [on|off] [1-250]\n", name);
}

int main(int argc, char **argv)
{
	usb_dev_handle      *handle = NULL;
	const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
	char                vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
	char                buffer[4];
	int                 cnt, vid, pid, isOn;

    usb_init();
    if(argc < 2){   /* we need at least one argument */
        usage(argv[0]);
        exit(1);
    }
    /* compute VID/PID from usbconfig.h so that there is a central source of information */
    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];
    /* The following function is in opendevice.c: */
    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
        exit(1);
    }

    if(strcasecmp(argv[1], "status") == 0){
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_GET_STATUS, 0, 0, buffer, sizeof(buffer), 5000);
        if(cnt < 1){
            if(cnt < 0){
                fprintf(stderr, "USB error: %s\n", usb_strerror());
            }else{
                fprintf(stderr, "only %d bytes received.\n", cnt);
            }
        }else{
        	printf("status=%d\n", buffer[0]);
        	printf("sound=%d\n", buffer[1]);
        	printf("sound_tc=%d\n", buffer[2]);
        }
    }else if(strcasecmp(argv[1], "start") == 0){
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, CUSTOM_RQ_SET_START, 0, 0, buffer, 0, 5000);
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
    }else if(strcasecmp(argv[1], "stop") == 0){
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, CUSTOM_RQ_SET_STOP, 0, 0, buffer, 0, 5000);
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
    }else if(strcasecmp(argv[1], "test") == 0){
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, CUSTOM_RQ_SET_TEST, 0, 0, buffer, 0, 5000);
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
    }else if(strcasecmp(argv[1], "sound") == 0){
    	int sound = 0;
    	if (strcasecmp(argv[2],"on")==0) {
   			sound = atoi(argv[3]);
    	} else if (strcasecmp(argv[2], "off")) {
    		sound = 0;
    	}
    	if (sound<0) sound = 0;
    	if (sound>250) sound = 250;
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, CUSTOM_RQ_SET_BUZZER, sound, 0, buffer, 0, 5000);
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
    }else{
        usage(argv[0]);
        exit(1);
    }
    usb_close(handle);
    return 0;
}
