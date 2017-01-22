Alert system.
=============================
Can use cameras and external alarm sensors(PIR, Gas detector, etc).

If cameras used:
    - if detect motion;
    - save video;
    - send email.
    - after sleep time - send email again ReplyCount time.

Video include last BufferSize frames + LongBufferSize (long buffer - buffer with frames skips).

Also can be used as MJPEG server and server, which send crypted data from one instance to other.

If used external alarm sensors:
    - Read sensor data. (sensors are connected to serial port);
    - if (isAlarm) - send email.
    - TODO: send video???

PIR sensor connect to arduino

INSTALLATION
------------
    - edit CMakeLists.txt params.
    - mkdir BUILD -p && cd BUILD && cmake -DCMAKE_INSTALL_PREFIX=/bin/test ..  && make -j8 && sudo make install

Depends:
-      C++11
-      OpenCV 2.4.11
-      Qt-5
-      Cmake
-      Zlib, minizip
-      Boost for Telegram bot (COMPONENTS system iostreams)

install all dependency on deb- like system:
-    sudo apt-get install cmake
-    sudo apt-get install libssl-dev
-    sudo apt-get install libminizip-dev zlib1g-dev
-    sudo apt-get install libopencv-dev
-    sudo apt-get install qtmultimedia5-dev qtbase5-dev libqt5multimedia5-plugins
-    sudo apt-get install libqt5serialport5-dev libqt5serialport5
-    sudo apt-get install qtdeclarative5-dev
-    sudo apt-get install g++ make binutils libssl-dev libboost-system-dev libboost-iostreams-dev #for telegram
-    libqt5sql5-psql

-    kdelibs5-dev //for FindGStreamer.cmake or download from github

On windows build Zlib with patch or link with minizip (patch include minizip to zlib library):
-    //TODO
