<B>Project goals:</B>
<P>
For this project, I had a couple of simple(ish) goals that I wanted to achieve:
  
- I wanted to build something cheap that could sample both the total current (amps) flowing into my home, and the current flowing into my hot water system (which is on a circuit control by the local distribution/lines company). 

- Graph this data in ~10 second periods so that I could deduce my (households) load profile, and understand the times when we’re using the most energy, and what out base load is (the parasitic load of our electronics).

- I’m thinking of signing up with an electricity retailer that offers 1-hour period of free power (outside of the morning and evening peaks). With this retailer, I can nominate this 1-hour period on their web portal, and provided I make this nomination before midnight, I can select which hour I want free for that same day. So, my long-term goal is to have the RPi monitor energy consumption over each of the 24 * one-hour periods in a day, work out the most ‘energy intensive’ one-hour period (outside of the peak periods), and then automatically login into this retailer’s web portal at ~23:45 in the evening, and set the free one-hour period on my behalf. 

</P>
This repository contains the code I have written / pillaged to start achieving some of these goals, and as it’s a work in progress, I’ll keep updating things as I go along.
<P>
For anyone else thinking of following in my footsteps, it would pay for me to mention that I’ve have pretty easy access to the distribution board (in order to install the CTs around the appropriate cable cores), I’m licensed to be legally able to do that, and that the DB was in ear shot my Wi-Fi router. Another way of putting it is…… I had a readily available power supply for my RPi, I could use Wi-Fi to link my RPi to my home LAN, and the DB board was fairly open to accommodate all this extra hardware.
</P>
<B>The parts list:</B>
<P>
  
-	1* Raspberry Pi 3B+ (from any retailer, they’re all alike)

-	1* 20A/1V CT (To measure hot water load).
https://www.amazon.com/YHDC-Manufacturer-SCT013-020-Current-Transformer/dp/B0192KWKLK

-	1* 60A/1V CT To measure household total load).
https://www.amazon.com/YHDC-Manufacturer-SCT013-060-Current-Transformer/dp/B0192OEZ0K

-	1* ADS1115, 16Bit, 4 Channel, I2C Interface.
https://www.amazon.com/ADS1115-16-Bit-ADC-Programmable-Amplifier/dp/B00SOY52CU

-	1* PDL642MUSB Module (To hardwire a power supply to the Pi)
http://www.pdl.co.nz/Trade/Products/ProductDetail?catNo=PDL642MUSB&cat=1&sc=1&r=63599&nt=63599-510575&nt2=510575-327608

-	Odd bits of wire to connect everything together.
https://www.amazon.com/Smraza-Breadboard-Resistors-Mega2560-Raspberry/dp/B01HRR7EBG

I used CT’s which output 1 volt on the secondary terminals (at full scale primary amps) as this would save me from having to install a resistor in series with the CT secondary, in order for the ADS1115 to measure volt drop over the resistor (The ADS1115 cannot measure current flow natively). 
</P>
<B>Wiring:</B>
<P>
The CT’s come with a 3.5mm male jack on each end. The first thing I did was cut this off and solder on some female pin sockets that allowed me to plug straight onto the pins of the ADS1115 (for this I unclamped the pins off of the breadboard wires, and soldered them onto the CT leads).

The current waveform I intend to measure is AC, and will be alternating 50 times per second (50Hz). This means each CT will output a voltage signal in the range of +1V to -1V. Because of this, I’ll configure the ADS1115 to operate with two differential inputs (as opposed to four single ended inputs). This will allow me to connect both leads of each CT directly to the ADS input pins, and for the ADS to measure both positive and negative voltages (removing the need to bias the AC signal with a small amount of DC voltage). 

For me, the two legs of the 60A CT connected to pins A0 and A1 of the ADS1115. The two legs of the 20A CT connected to pins A2 and A3. The ADS itself was then connected to the Raspberry PI via I2C bus, which required the connections as shown in the wiring diagram (included as a separate file in this repository). By connecting the ADS ‘address’ pin to ground, the ADS assumes an address of 0x48.
</P>
<B>Software:</B>
<P>
I chose to install Raspbian as my OS, as this would be sufficient for my needs. The installation process is well documented, so I won’t cover it again here.

Once the OS was installed, I chose to also install Node-Red. I’ve used Node-Red elsewhere and I love it. It’s a really intuitive way to program for those not wanting to get into the nitty gritty detail, and a big bonus is it can be accessed via the web browser of a remote laptop. 
Once again, the installation process is well documented already, so I won’t cover it again. I will note that once installed, I also installed the “Node-Red-Dashboard” extension, which can be done from within the Node-Red UI and is a really impressive extension.

One drawback of Node-Red (or most high-level programming languages for that matter), is there is some loss of execution speed. I did some testing with JavaScript and Python, to see how fast they could read/store data from the ADS1115, and they generally topped out at about 400Hz. As the AC waveform is 50Hz, this means they could capture ~8 samples per AC cycle (sine wave), which is a little low for calculating RMS current accurately (in my opinion).

On the recommendation of a friend, I took a crash course and learned a bit about C++, just enough to make something work (and probably not well). I downloaded and installed ‘Netbeans IDE 8.2’ onto my laptop, and the cool thing about this IDE is it allowed me to cross compile to my RPi (I.E. I write the code on my laptop, the code is compiled on the RPi where a file is made, this file is executed on the RPi, and the result is feed back to the output panel on my laptop. Brilliant!).
Long story short, I managed to hack a bit of C++ code together that was able to sample each AC cycle ~80 times (compared to ~8 using JS/Python), perfect!

From here, I made it so the C++ code samples the AC current waveform from (each CT) ‘n’ number of times, then outputs two comma separated float values. The first being total current, the second being hot water current. The compiled C++ binary file was then saved somewhere logical on my RPi (I.E. in my user home directory).

In Node-Red, a timer pulse then activates an ‘exec’ node on a reoccurring basis, and the ‘exec’ node executes the compiled C++ binary I created earlier, which spits out two numbers. I then used a couple of other nodes in the same flow to split the two CSV values apart, and add a couple of hard code attributes to them before they are displayed on the graphical UI.

I also take the direct output of the ‘exec’ node, and fire this off to AdaFruit IO’s MQTT cloud service, so that I can access the same data (currents) from their platform, and make use of their graphing tools and dashboard.
</P>
