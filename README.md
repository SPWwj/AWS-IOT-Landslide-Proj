**1. Introduction**

SPMakers is a team of two year two students taking Diploma in Computer Engineering in Singapore Polytechnic. We are inspired to build an end-to-end IoT cloud computing system to make the world a better and safer place to live. The MakeSense1 is our solution, utilizing our self-developed IoT enabled microcontroller known as the &quot;SPEEEduino&quot; and Amazon Web Service&#39;s various cloud computing services to collect, store and perform computations on data, and finally provides visualization over the Internet using the D3 library.

**2. Problems and Research**

Landslides form silently, with minimal signs, and is considered one of the most difficult natural disasters to predict. Landslides are highly localized, and warnings often come too late for the people living there to take any life-saving action.

**3. Our Solution**

The MakeSense1 aims to acquire environmental parameters relevant to landslides while doubling as a conservation tool, as its hollow tubes with Voronoi patterns allows for one to deposit fertilizer in the tubes, feeding nearby vegetation which proactively prevents landslides. The MakeSense1 also aims to feeds data into a third-party machine learning based landslide prediction platform.

**3.1 System Diagram and Operation**


The MakeSense1 mainly consists of 3 subsystems: (i) Sensing Subsystem; (ii) Cloud Computing Subsystem; and (iii) Visualization Subsystem. The Sensing Subsystem uses the SPEEEduino, an IoT enabled microcontroller, and interfaces with 3 sensors: an IMU (Inertial Measurement Unit), a rainfall sensor, and a soil moisture sensor, contained in a 3D-printed enclosure with an IP65 waterproof box. The SPEEEduino has an ESP8266-01 Wi-Fi module onboard, allowing it to communicate to the Internet via gateways. It is modular and adaptable to technologies like SigFox and LoRa. The Cloud Computing Subsystem uses Amazon&#39;s AWS IoT, SNS and DynamoDB services to capture, store and process data sensed. The Visualization subsystem is a web interface that displays all the data from the database using the D3 library.


MakeSense1 Node

**3.3 Future plans**

Future development plans include (i) mesh networking between individual MakeSense1 nodes, allowing coverage of the entire area with wireless networking; (ii) push notifications to its users&#39; mobile devices which allows them to be immediately notified of an emergency; and (iii) feeding sensor data into a machine learning platform for early detection of environmental disasters.
