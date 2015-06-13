MS_PPI
======

repo for MS PPI
=======

The folders/applications that are in this repo have the following purpose and legacy:

Arduino/Teensy - This is where the application that lives on the Teensy resides

gstStreamManager - This is the visual studio project for an application showing how the GSTStreamer manager class works

gstStreamManager_mac - As above but the OSX version.

localTotem - This is the application that runs on the totem itself. It draws the video feed into an FBO which is then stretched across the 4 displays.

remoteUser - This is the application that shows the remote user experience. It draws the video into a cylinder and includes hot-keys to trigger events and Wizard-of-Oz the application

sharedCode - This contains the code for the gstStreamManager class itself

streaming - Not currently being used

VideoLAN - Not currently being used


# Totem
Totem for MS

## openFrameworks Addons

Clone the following addons to ```OF_0.8.4_ROOT/addons/```

- [ofxGstRTP](https://github.com/studiostudio/ofxGstRTP)
- [ofxGStreamer](https://github.com/arturoc/ofxGStreamer)
- [ofxNice](https://github.com/arturoc/ofxNice)
- [ofxSnappy](https://github.com/studiostudio/ofxSnappy)
- [ofxDepthStreamCompression](http://github.com/studiostudio/ofxDepthStreamCompression)
- [ofxEchoCancel](http://github.com/studiostudio/ofxEchoCancel)
- [ofxJSON](https://github.com/jefftimesten/ofxJSON)

This addons requires the v0.8.4 branch that targets ofxOSC in openFrameworks v0.8.4
- [ofxOSCSync](https://github.com/studiostudio/ofxOSCSync/tree/v0.8.4) 
   ```
    git clone https://github.com/studiostudio/ofxOSCSync
    
    git fetch origin
    
    git checkout v0.8.4
```


##GStreamer install
To Install GStreamer on Windows do the following:

Download GStreamer v1.0. To install it under windows use the 1.2.2 packages located http://gstreamer.freedesktop.org/data/pkg/windows/1.2.2/ any later version from http://gstreamer.freedesktop.org/data/pkg/windows should work but haven't been tested yet. It is important to install the complete version when prompted by the installer.  It will install all nessicary files for development and all extra packages. 
gstreamer-1.0-x86-1.2.2.msi installs the necessary libraries for applications to run.
gstreamer-1.0-devel-x86-1.2.2.msi installs the development files needed to compile gst applications.

After installing those 2 packages, you need to add the path to the bin directory of the installation to the PATH environment variable. In windows 8 open the control panel and search for environment variable and select Edit the System Environmental Variables.  This should open the System Properties menu.  Select the 'Environmental Variables' button.   Scroll and find the PATH variable and select edit and add the following to the end:

;C:\gstreamer\1.0\x86\bin

Or pointing to the bin directory of your GStreamer installation.  Also insure that GSTREAMER_1_0_ROOT_X86 is defined as C:\gstreamer\1.0\x86\ or the location you installed GStreamer.

This will require you to logout for the System Environmental Variables to take effect

Adding GStreamer to your OF Application

You must use the Package Generator aka yee old Project Generator from /devApps to propertly setup the VS2012 project.  

You must select the following addons when creating a new project

- ofxDepthStreamCompression
- ofxEchoCancel
- ofxGStreamer
- ofxGstRTP
- ofxGui
- ofxNetwork
- ofxNice
- ofxOsc
- ofxSnappy
- ofxXmlSettings
- ofxXMPP



Editing ofxGstRTPConstants.h

You will need to disable NICE and EchoCancel in ofxGstRTPConstants.h as those features cause issues on Windows.

```
/*
 * ofxGstRTPConstants.h
 *
 *  Created on: Oct 9, 2013
 *      Author: arturo
 */
 
 #ifndef OFXGSTRTPCONSTANTS_H_
 #define OFXGSTRTPCONSTANTS_H_
 
 /// when enabled the addon will have dependencies with ofxXMPP and ofxNice
 /// if disabled ofxGstXMPPRTP won't work and connection will need to be done
 /// through ofxGstServer/client to a specific port and IP address
 #define ENABLE_NAT_TRANSVERSAL 0
 
 /// when enabled the addon will have dependencies with ofxEchoCancel on linux
 /// the pulse server can do echo cancellation natively so there's no need to enable
 /// this flag
 #define ENABLE_ECHO_CANCEL 0
 
 #endif /* OFXGSTRTPCONSTANTS_H_ */
```
 
 

ofxNiceAgent.h might need the following line at the top:

```#define uint uint32_t```

You’re ready to build!
