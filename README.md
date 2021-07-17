# smart water pressure sensor

This project pairs a pressure transducer with 2 microcontrollers which reads and publishes the water pressure periodically to an mqtt server.  An example configuration.yaml is provided for integrating this smart sensor into home assistant.  Other smart home architectures can be supported by configuring the mqtt parameters appropriately.  

![case open](https://raw.githubusercontent.com/kevincw01/smart-water-pressure-sensor/main/case%20open.jpg | width=100)
<img src="https://raw.githubusercontent.com/kevincw01/smart-water-pressure-sensor/main/case%20open.jpg" alt="case open" width="100" height="100">

## Installation

Review the provided wiring diagram (fritz and image provided), buy the parts, and wire it up!

You will need to buy the nano iot 33 and nano every microcontrollers, a 0.5-4.5v pressure transducer (I used [this](https://www.amazon.com/gp/product/B07Z3LRN6M/ref=ppx_yo_dt_b_asin_title_o08_s00?ie=UTF8&psc=1)) and a power supply (I just found an old 10v, 2A power supply).  

NOTE: while they should be sufficient, lower voltage and current power supplies wouldn't work for me e.g. 5v/1A.  I suspect this is due to inrush on start-up and/or my poor circuit design skills :)

Configure your wifi and MQTT credentials in the nano_33_water_sensor/credentials.h file.  Flash the arduino nano iot 33 microcontroller with the source in the nano_33_water_sensor folder.  

Flash the arduino nano every microcontroller with the source in the AnalogInput folder.

Review the configuration.yaml file and add to your home assistant installation.  You will need to add the MAC address of your nano IOT 33 to the file.  You can find this in you router's DHCP table along with the IP address.

## Usage

Apply power to the system and confirm it is connected to wifi and the mqtt server.  Use an mqtt browser (like mqtt.fx) connected to your mqtt server (like mosquitto) to find the topic and subscribe.  View the water (or really any pressure you've connected to your sensor too) pressure publications. 

My sensor publishes to the topic:

```'arduino/Arduino-7C:9E:BD:3B:33:B0/sensor/water-pressure'```

Your topic will vary by the MAC address of your nano IOT 33.

The system will publish the pressure upon start-up and then every 5s if the pressure difference is 2+psi from the previous measurement.  If the pressure difference hasn't changed, it will still publish it every 5 minutes.  You can change all of this in the iot 33 code to your liking.

## Troubleshooting
I suggest monitoring the iot 33 serial console when you first boot it to see that it's working correctly.  You might also want to bring the iot 33 LED out to the case so that you can see status.

Every once in a while, my iot 33 doesn't connect to wifi and requires a reboot.  I'm not sure why this is.  The built-in LED will flash rapidly when this happens and I log a message to the screen.  I haven't been able to figure out how to reboot automatically yet.  Power cycling the system usually fixes this.

If the mqtt server is configured incorrectly or goes down, the iot 33 built-in LED will flash slowly 5 times, every 10s.  

There is some calibration done in the AnalogInput source code based on my experience and the sensor I have.  Feel free to adjust as required.

## Contributing
Pull requests are welcome.

## License
[GNU GPL 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)
