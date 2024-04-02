
---
# Update:

* Can only work with SDIO 1 bit (don't know why).

* Long file name only work when adding ccsbcs.c in ../Third_Party/FatFs/src/option. Then change _USE_LFN to (1) in ../FATFS/Target/ffconf.h

* The device can handle difference applications, now I only focus on SD card reader application:
## Application selection menu:
![image](Photo/10.jpg)

---
### [SD card browser] Root folder:
![image](Photo/11.jpg)

---
### The app can handle unlimit sub-folder, can read text and show JPEG image folder/subfolder:

Sub-folder:
![image](Photo/16.jpg)
subsub-folder:
![image](Photo/17.jpg)

---
Showing JPEG image:
![image](Photo/04.jpg)
![image](Photo/08.jpg)


Reading text, use navigate button to see the rest of the text:
![image](Photo/12.jpg)
![image](Photo/06.jpg)
![image](Photo/05.jpg)

---
# Test with CANBus (loopback mode) & RTC:

![image](Photo/09.jpg)

---

The CANBus app will create a file in SD card:
![image](Photo/13.jpg)

---

The file in SD card can be delete with hitting button LEFT,
then user can choose to delete (hit button LEFT again) or cancel (press button RIGHT).
![image](Photo/14.jpg)

---

Data logged inside the file:
![image](Photo/15.jpg)
