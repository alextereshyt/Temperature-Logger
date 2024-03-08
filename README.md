<h1>Temperature Logger v1
</h1>
<img width="500" src="https://github.com/alextereshyt/Temperature-Logger/assets/48646892/faf23647-2506-4f6e-808b-aa41008e9889"/><br>
A simple temperature logger with lcd display and real time info.

<h2>Components that were used:</h2>

- ESP32 (NodeMCU Dev Board)
- DS3231 RTC 
- LCD 16x2 (I2C or without)
- MicroSD Reader 
- Encoder with button

<h2>Get started</h2>

- Format SD Card to FAT32 filesystem

- Configure RTC Module:
 Uncomment this line to set up your computers date and time.
 Flash it , than comment and flash again.<br>
 ![image](https://github.com/alextereshyt/Temperature-Logger/assets/48646892/55121406-21ad-433e-b6ff-9285349cd071)

 
- Calibrate a temperature sensor and set up config in .ino code
![image](https://github.com/alextereshyt/Temperature-Logger/assets/48646892/fd7ad960-8538-4c87-a90c-3e5991984e0b)

<h2>Usage:</h2>

- Just power on the device and it will automatically start logging temperature and date to sd card 
(Device will create a txt file every day)<br>
- Select dashboard to view real time statistic 

<h2>Libraries</h2>

- Basically just install all libraries from Library Manager<br>
<img src="https://github.com/alextereshyt/Temperature-Logger/assets/48646892/6f23cc6d-cea1-4fa6-a559-593dddca6778"/>


<h2>Corps</h2>
<img src="https://github.com/alextereshyt/Temperature-Logger/assets/48646892/0a8289f9-b41a-43b3-9990-d79033a1c497"/>
<img src="https://github.com/alextereshyt/Temperature-Logger/assets/48646892/83558816-4e40-4529-a2cb-9ec75a7fd622"/>

- 3d printed(.step included)








