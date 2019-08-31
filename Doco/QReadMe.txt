QReadMe

Website with project info
https://medium.com/@elkentaro/feathering-for-ssids-bd69ad41165a


Header not found error. 

https://stackoverflow.com/questions/50080260/arduino-ide-cant-find-esp8266wifi-h-file


When programming the NODEMCU card with the Arduino IDE, you need to customize it and you must have selected the correct card.

Open Arduino IDE and go to files and click on the preference in the Arduino IDE.

Add the following link to the Additional Manager URLS section: "http://arduino.esp8266.com/stable/package_esp8266com_index.json" and press the OK button.

Then click Tools> Board Manager. Type "ESP8266" in the text box to search and install the ESP8266 software for Arduino IDE.

You will be successful when you try to program again by selecting the NodeMCU card after these operations. I hope I could help.

8/18 - above done.  Worked. 


