/* *****************************************************************************
 * Raspberry Pi Fan Manager - Version 0.1.1 - 2020-09-20
 * Copyright 2020 Matt Rude <matt@mattrude.com>
 *
 * *****************************************************************************
 *
 * This is a small program written in c that monitors the CPU temperature,
 * turns on an installed fan if the temperature reaches a set limit, then turns
 * the fan back off when the temperature drops below a second set limit.
 *
 * There is no configuration file or command line flags, all options must be
 * set before compiling. See Below.
 *
 * *****************************************************************************
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * *****************************************************************************
 */

// The avaliable options

// See http://wiringpi.com/pins/ for the WiringPi pinout.
int fanPin = 0;         // (pin) The GPIO pin you're using to control the fan.
int sleepTime = 30;     // (seconds) How often to check the core temperature.

int onTemp = 62;        // (degrees Celsius) Fan turns on at this temperature.

// Note: The Raspberry Pi 4 seems to idle at around 56°c, keep offTemp above this.
int offTemp = 58;       // (degress Celsius) Fan shuts off at this temperature.

/*******************************************************************************/

#include <stdio.h>      // Needed for 'fopen' function
#include <unistd.h>     // Needed for 'sleep' function
#include <syslog.h>     // Needed to write to syslog
#include <wiringPi.h>   // Needed to set GPIO pins

int main() {
    
    
    ("It's fan manager program"); // my contribute

    // Create the fan running check variable
    int fanRunning = 0;

    // Start the wiringPi module
    wiringPiSetup();

    // Set the fan pin as an output
    pinMode (fanPin, OUTPUT);

    // Start with the fan off, for now.
    digitalWrite (fanPin, LOW);

    // Setup the temp & time files
    float sysTemp, millideg;

    // Create a 'thermal' file to read the actual temp file with
    FILE *thermal;
    
    printf("program starting"); // my contribute
    
    openlog ("raspi-fan-manager", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (LOG_INFO, "Starting: raspi-fan-manager - Version: 0.1.1");
    syslog (LOG_INFO, "https://github.com/mattrude/raspberrypi-fan-manager/");
    closelog ();

    // Check to make sure the on temp is highter then the off temp
    if ( offTemp >= onTemp ) {
        printf("raspi-fan-manager Error: the 'onTemp' is lower or the same as 'offTemp', this just wont work\n");
        printf("onTemp: %d, offTemp: %d\n",onTemp,offTemp);

        openlog ("raspi-fan-manager", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        syslog (LOG_ERR, "Error: the 'onTemp' is lower or the same as 'offTemp', this just wont work");
        closelog ();
        return 2;
    }

    // This is were the magic happens
    while(1) {

        // Read the CPU temp and store the value to 'sysTemp'
        thermal = fopen("/sys/class/thermal/thermal_zone0/temp","r");
        fscanf(thermal,"%f",&millideg);
        fclose(thermal);
        sysTemp = millideg / 1000;

        // Check if sysTemp is greater then onTemp & that the fan is not running already
        if ( sysTemp >= onTemp && fanRunning == 0 ) {

            // Turn the fan on
            digitalWrite (fanPin, HIGH);
            
            printf("\nCurrent cpu temp is %f\n", sysTemp); // my contributon
	        printf("starting fan"); // my contribution

            // Send message to syslog that the fan is now running
            openlog ("raspi-fan-manager", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
            syslog (LOG_INFO, "CPU is running hot at %f, starting fan.",sysTemp);
            closelog ();

            // Set the 'fanRunning' varible to True
            fanRunning = 1;

        // If sysTemp is lower then offTemp & fan is already running, stop it
        } else if ( sysTemp <= offTemp && fanRunning == 1 ) {

            // Turn the fan off
            digitalWrite (fanPin, LOW);
            
            printf("\nCurrent cpu temp is %f\n", sysTemp); //my contribution
	        printf("stopping fan"); //my contribution


            // Send message to syslog that the fan is no longer running
            openlog ("raspi-fan-manager", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
            syslog (LOG_INFO, "CPU is running cool again at %f, stopping fan.",sysTemp);
            closelog ();

            // Set the 'fanRunning' varible to False
            fanRunning = 0;
        }

        // Sleep for the 'sleepTime' value
        sleep(sleepTime);
    }
}
