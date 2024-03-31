
---
# Update:

* Can only work with SDIO 1 bit (don't know why).

* Long file name only work when adding ccsbcs.c in ../Third_Party/FatFs/src/option. Then change _USE_LFN to (1) in ../FATFS/Target/ffconf.h

* The device can handle difference applications, now I only focus on SD card reader application:
## Application selection menu:
![image](Photo/09.jpg)

---
### Root folder:
![image](Photo/11.jpg)

---
### The app can handle unlimit sub-folder, can read text and show JPEG image folder/subfolder:

Sub-folder:
![image](Photo/10.jpg)
subsub-folder:
![image](Photo/07.jpg)

---
Showing JPEG image:
![image](Photo/04.jpg)
![image](Photo/08.jpg)


Reading text, use navigate button to see the rest of the text:
![image](Photo/12.jpg)
![image](Photo/06.jpg)
![image](Photo/05.jpg)

---
# Test with CANBus (loopback) & RTC:

![image](Photo/01.jpg)

---

![image](Photo/02.jpg)

---

![image](Photo/03.jpg)
