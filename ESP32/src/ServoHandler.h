
// OSR-Release v2.4,
// by TempestMAx 1-7-20
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands and uses them to control servos and vibration motors
// Can handle three linear channels (L0, L1, L2), three rotation channels (R0, R1, R2) 
// and two vibration channels (V0, V1)
// This code is designed to drive the OSR series of robot, but is also intended to be
// used as a template to be adapted to run other t-code controlled arduino projects
// Have fun, play safe!
// History:
// v2.0 - TCode v0.2 compatible, 28-1-2020
// v2.1 - OSR2 release, 1-2-2020
// v2.2 - OSR2+ release, 1-3-2020
// v2.3 - T-Valve support added, 1-5-2020
// v2.4 - T-wist support added; LR servos now +/- 350 for the sake of Raser1's sanity, 7-10-2020
// v2.6 - Experimental build. For use with Parallax Feedback 360° Servo (900-00360) in T-wist. 23-9-2020
// v2.7 - T-valve suction level control added to L3. 25-11-2020
// v2.7b - T-valve suction level algorithm modified. 30-11-2020

#include <ESP32Servo.h>
#include "ToyComs.h"

volatile int twistFeedBackPin = SettingsHandler::TwistFeedBack_PIN;
// Twist position monitor variables
volatile int twistPulseLength = 0;
volatile int twistPulseCycle = 1099;
volatile int twistPulseStart = 0;
// Twist position detection functions
void IRAM_ATTR twistChange() 
{
	if(digitalRead(twistFeedBackPin) == HIGH)
	{
		twistPulseCycle = micros()-twistPulseStart;
  		twistPulseStart = micros();
	}
	else
	{
  		twistPulseLength = micros()-twistPulseStart;
	}
}

class ServoHandler 
{

private:
    ToyComms toy; 
    // Declare servos
    Servo RightServo;
    Servo LeftServo;
    Servo RightUpperServo;
    Servo LeftUpperServo;
    Servo PitchLeftServo;
    Servo PitchRightServo;
    Servo ValveServo;
    Servo TwistServo;

    // Specify which pins are attached to what here
    int RightServo_PIN = SettingsHandler::RightServo_PIN;
    int LeftServo_PIN = SettingsHandler::LeftServo_PIN;
    int RightUpperServo_PIN = SettingsHandler::RightUpperServo_PIN;
    int LeftUpperServo_PIN = SettingsHandler::LeftUpperServo_PIN;
    int PitchLeftServo_PIN = SettingsHandler::PitchLeftServo_PIN;
    int PitchRightServo_PIN = SettingsHandler::PitchRightServo_PIN;
    int ValveServo_PIN = SettingsHandler::ValveServo_PIN;
    int TwistServo_PIN = SettingsHandler::TwistServo_PIN;
    int Vibe0_PIN = SettingsHandler::Vibe0_PIN;
    int Vibe1_PIN = SettingsHandler::Vibe1_PIN;
    int Lube_PIN = SettingsHandler::Lube_Pin;

	int rightServoConnected = 0;
	int rightUpperServoConnected = 0;
	int leftServoConnected = 0;
	int leftUpperServoConnected = 0;
	int pitchServoConnected = 0;
	int pitchRightServoConnected = 0;
	int valveServoConnected = 0;
	int twistServoConnected = 0;

    // Arm servo zeros
    // Change these to adjust arm positions
    // (1500 = centre)
    int RightServo_ZERO = SettingsHandler::RightServo_ZERO;
    int LeftServo_ZERO = SettingsHandler::LeftServo_ZERO;
    int RightUpperServo_ZERO = SettingsHandler::RightUpperServo_ZERO;
    int LeftUpperServo_ZERO = SettingsHandler::LeftUpperServo_ZERO;
    int PitchLeftServo_ZERO = SettingsHandler::PitchLeftServo_ZERO;
    int PitchRightServo_ZERO = SettingsHandler::PitchRightServo_ZERO;
    int TwistServo_ZERO = SettingsHandler::TwistServo_ZERO;
    int ValveServo_ZERO = SettingsHandler::ValveServo_ZERO;

    // Declare timing variables
    unsigned long nextPulse;
    int tick;

    // Position variables
    float xLin,yLin,zLin;
    // Rotation variables
    float xRot,yRot,zRot;
    // Vibration variables
    float vibe0,vibe1;
	
	int suck;
    // Velocity tracker variables, for T-Valve
    float xLast;
    float xValve;

	// Twist position monitor variables
	float twistServoAngPos = 0.5;
	int twistTurns = 0;
	float twistPos;

	// Servo microseconds per radian
	// (Standard: 637 μs/rad)
	// (LW-20: 700 μs/rad)
	int ms_per_rad = 637;  // (μs/rad)

	int lubeAmount = SettingsHandler::lubeAmount;;

public:
    // Setup function
    // This is run once, when the arduino starts
    void setup(int servoFrequency) 
    {
		toy.setup();
        toy.identifyTCode();
        
    	RightServo_ZERO = SettingsHandler::RightServo_ZERO;
    	LeftServo_ZERO = SettingsHandler::LeftServo_ZERO;
    	RightUpperServo_ZERO = SettingsHandler::RightUpperServo_ZERO;
    	LeftUpperServo_ZERO = SettingsHandler::LeftUpperServo_ZERO;
    	PitchLeftServo_ZERO = SettingsHandler::PitchLeftServo_ZERO;
    	PitchRightServo_ZERO = SettingsHandler::PitchRightServo_ZERO;
    	TwistServo_ZERO = SettingsHandler::TwistServo_ZERO;
    	ValveServo_ZERO = SettingsHandler::ValveServo_ZERO;

        RightServo.setPeriodHertz(servoFrequency);
        RightUpperServo.setPeriodHertz(servoFrequency);
        LeftServo.setPeriodHertz(servoFrequency);
        LeftUpperServo.setPeriodHertz(servoFrequency);
        PitchLeftServo.setPeriodHertz(servoFrequency);
        PitchRightServo.setPeriodHertz(servoFrequency);
		//ValveServo.setTimerWidth(20);
        // ValveServo.setPeriodHertz(servoFrequency);
        // TwistServo.setPeriodHertz(servoFrequency);

		RightServo_PIN = SettingsHandler::RightServo_PIN;
		LeftServo_PIN = SettingsHandler::LeftServo_PIN;
		RightUpperServo_PIN = SettingsHandler::RightUpperServo_PIN;
		LeftUpperServo_PIN = SettingsHandler::LeftUpperServo_PIN;
		PitchLeftServo_PIN = SettingsHandler::PitchLeftServo_PIN;
		PitchRightServo_PIN = SettingsHandler::PitchRightServo_PIN;
		ValveServo_PIN = SettingsHandler::ValveServo_PIN;
		TwistServo_PIN = SettingsHandler::TwistServo_PIN;
		Vibe0_PIN = SettingsHandler::Vibe0_PIN;
		Vibe1_PIN = SettingsHandler::Vibe1_PIN;

        // Declare servos and set zero
        rightServoConnected = RightServo.attach(RightServo_PIN);
        if (rightServoConnected == 0) 
        {
            Serial.print("Failure to connect to right pin: ");
            Serial.println(RightServo_PIN);
        }
        rightUpperServoConnected = RightUpperServo.attach(RightUpperServo_PIN);
        if (rightUpperServoConnected == 0) 
        {
            Serial.print("Failure to connect to right upper pin: ");
            Serial.println(RightUpperServo_PIN);
        }
        leftServoConnected = LeftServo.attach(LeftServo_PIN);
        if (leftServoConnected == 0) 
        {
            Serial.print("Failure to connect to left pin: ");
            Serial.println(LeftServo_PIN);
        }
        leftUpperServoConnected = LeftUpperServo.attach(LeftUpperServo_PIN);
        if (leftUpperServoConnected == 0) 
        {
            Serial.print("Failure to connect to left upper pin: ");
            Serial.println(LeftUpperServo_PIN);
        }
        pitchServoConnected = PitchLeftServo.attach(PitchLeftServo_PIN);
        if (pitchServoConnected == 0) 
        {
            Serial.print("Failure to connect to pitch left pin: ");
            Serial.println(PitchLeftServo_PIN);
        }
        pitchRightServoConnected = PitchRightServo.attach(PitchRightServo_PIN);
        if (pitchRightServoConnected == 0) 
        {
            Serial.print("Failure to connect to pitch right pin: ");
            Serial.println(PitchRightServo_PIN);
        }
        valveServoConnected = ValveServo.attach(ValveServo_PIN);
        if (valveServoConnected == 0) 
        {
            Serial.print("Failure to connect to valve pin: ");
            Serial.println(ValveServo_PIN);
        }
        twistServoConnected = TwistServo.attach(TwistServo_PIN); 
        if (twistServoConnected == 0) 
        {
            Serial.print("Failure to connect to twist pin: ");
            Serial.println(TwistServo_PIN);
        }

        delay(500);
        
        if (rightServoConnected != 0) 
		{
        	RightServo.writeMicroseconds(RightServo_ZERO);
		}
        if (leftServoConnected != 0) 
		{
        	LeftServo.writeMicroseconds(LeftServo_ZERO);
		}
        if (rightUpperServoConnected != 0) 
		{
        	RightUpperServo.writeMicroseconds(RightUpperServo_ZERO);
		}
        if (leftUpperServoConnected != 0) 
		{
        	LeftUpperServo.writeMicroseconds(LeftUpperServo_ZERO);
		}
        if (pitchServoConnected != 0) 
		{
        	PitchLeftServo.writeMicroseconds(PitchLeftServo_ZERO);
		}
        if (pitchRightServoConnected != 0) 
		{
        	PitchRightServo.writeMicroseconds(PitchRightServo_ZERO);
		}
        if (valveServoConnected != 0) 
		{
        	ValveServo.writeMicroseconds(ValveServo_ZERO);
		}
        if (twistServoConnected != 0) 
		{
        	TwistServo.writeMicroseconds(TwistServo_ZERO);
		}

        // Set vibration PWM pins
        pinMode(Vibe0_PIN,OUTPUT);
        pinMode(Vibe1_PIN,OUTPUT);
        // Test vibration channels
        analogWrite(Vibe0_PIN,127);
        delay(300);
        analogWrite(Vibe0_PIN,0);
        analogWrite(Vibe1_PIN,127);
        delay(300);
        analogWrite(Vibe1_PIN,0);
  		pinMode(Lube_PIN,INPUT);

        // Set servo pulse interval
        tick = 1000/servoFrequency; //ms
        // Set time for first pulse
        nextPulse = millis() + tick;

        // Velocity tracker, for T-Valve
        xLast = 500;
        xValve = 0;

		// Initiate position tracking for twist
		pinMode(twistFeedBackPin, INPUT);
		attachInterrupt(twistFeedBackPin, twistChange, CHANGE);

        // Signal done
        Serial.println("Ready!");

    }

    // ----------------------------
    //   MAIN
    // ----------------------------
    // This loop runs continuously

    void read(byte inByte) 
    {
        // Send the serial bytes to the t-code object
        // This is the only required input for the object
        toy.serialRead(inByte);
    }

    void execute() 
    {

        // Pulse Servos based on time interval
        // This function will run every 20ms, sending a pulse to the servos
        if (millis() > nextPulse) { 
            unsigned long t = nextPulse;
            nextPulse = nextPulse + tick;
            // Collect inputs
            // These functions query the t-code object for the position/level at a specified time
            // Number recieved will be an integer, 1-1000
                // Serial.print("SH xLinear ");
                // Serial.printf("n: 0, t: %lu\n", t);
            xLin = toy.xLinear(0,t);
            //Serial.printf("xLin: %u, t: %lu\n", xLin, t);

			// SR6 /////////////////
			yLin = toy.xLinear(1,t);
			zLin = toy.xLinear(2,t);
			////////////////////////

    		suck = toy.xLinear(3,t);
            xRot = toy.xRotate(0,t);
            yRot = toy.xRotate(1,t);
            zRot = toy.xRotate(2,t);
            // Serial.print("zRot ");
            // Serial.println(zRot);
            // Serial.printf("zRot: %u, t: %lu\n", zRot, t);
            vibe0 = toy.xVibe(0,t);
            vibe1 = toy.xVibe(1,t);

            // If you want to mix your servos differently, enter your code below:
            

            //Calculate valve position
			//Track receiver velocity
			if (SettingsHandler::autoValve)
			{
				float Vel,ValveCmd,localSuck;
				Vel = xLin - xLast;
				Vel = 50*Vel/tick;
				xLast = xLin;
				localSuck = 20;
				if (Vel > localSuck) {
					ValveCmd = Vel-localSuck;
				} else if (Vel < 0){
					ValveCmd = -Vel;
				} else {
					ValveCmd = 0;
				}
				xValve = (4*xValve + ValveCmd)/5;
			} 
			else 
			{
				float Vel,ValveCmd;
				Vel = xLin - xLast;
				xLast = xLin;
				if (Vel < 0) {
					ValveCmd = 1000;
				} else {
					ValveCmd = 1000-suck;
				}
				xValve = (2*xValve + ValveCmd)/3;     
			}

            //Serial.print("xValve: ");
            //Serial.println(xValve);
            if (!SettingsHandler::continousTwist) 
			{
				// Calculate twist position
				float dutyCycle = twistPulseLength;
				dutyCycle = dutyCycle/twistPulseCycle;
				float angPos = (dutyCycle - 0.029)/0.942;
				angPos = constrain(angPos,0,1) - 0.5;
				if (angPos - twistServoAngPos < - 0.8) { twistTurns += 1; }
				if (angPos - twistServoAngPos > 0.8) { twistTurns -= 1; }
				twistServoAngPos = angPos;
				twistPos = 1000*(angPos + twistTurns);
			}

			if (SettingsHandler::sr6Mode) 
			{
				//Serial.print("SR6 mode");
				int roll,pitch,fwd,thrust,side;
				roll = map(yRot,0,1000,-3000,3000);
				pitch = map(zRot,0,1000,-2500,2500);
				fwd = map(yLin,0,1000,-3000,3000);
				thrust = map(xLin,0,1000,-6000,6000);
    			side = map(zLin,0,1000,-3000,3000);

				// Main
				int leftLowerValue = SetMainServo(16248 - fwd, 1500 + thrust + roll);
				int leftUpperValue = SetMainServo(16248 - fwd, 1500 - thrust - roll);
				int rightUpperValue = SetMainServo(16248 - fwd, 1500 - thrust + roll);//reversed both rights
				int rightLowerValue = SetMainServo(16248 - fwd, 1500 + thrust - roll);
				// Pitcher
				int pitchLeftValue = SetPitchServo(16248 - fwd, 4500 - thrust,  side - 1.5*roll, -pitch);
				int pitchRightValue = SetPitchServo(16248 - fwd, 4500 - thrust, -side + 1.5*roll, -pitch);

				if (leftServoConnected != 0) 
					LeftServo.writeMicroseconds(LeftServo_ZERO - leftLowerValue);
				if (leftUpperServoConnected != 0) 
					LeftUpperServo.writeMicroseconds(LeftUpperServo_ZERO + leftUpperValue);
				if (rightServoConnected != 0) 
					RightServo.writeMicroseconds(RightServo_ZERO + rightLowerValue);//reversed both rights
				if (rightUpperServoConnected != 0) 
					RightUpperServo.writeMicroseconds(RightUpperServo_ZERO - rightUpperValue);
				if (pitchServoConnected != 0) 
					PitchLeftServo.writeMicroseconds(constrain(PitchLeftServo_ZERO - pitchLeftValue, PitchLeftServo_ZERO-600, PitchLeftServo_ZERO+1000));
				if (pitchRightServoConnected != 0) 
					PitchRightServo.writeMicroseconds(constrain(PitchRightServo_ZERO + pitchRightValue, PitchRightServo_ZERO-1000, PitchRightServo_ZERO+600));
			}
			else 
			{
				//Serial.print("OSR mode");
				// Mix and send servo channels
				// Linear scale inputs to servo appropriate numbers
				int stroke,roll,pitch;
				stroke = map(xLin,1,1000,-350,350);
				roll   = map(yRot,1,1000,-180,180);
				pitch  = map(zRot,1,1000,-350,350);  
				//valve  = map(valve,1000,1, 1,1000);  
				//valve  = constrain(xValve, 0, 1000);
				// Serial.print("SettingsHandler::continousTwist: ");
				// Serial.println(SettingsHandler::continousTwist);
				// Serial.print("twistPos: ");
				// Serial.println(twistPos);
				// Serial.print("twist: ");
				// Serial.println(twist);
				// Serial.print("xRot: ");
				// Serial.println(xRot);


				//Serial.printf("a %d, b %d, c %d, d %d, twist %d\n", a,b,c,d,twist);
				//Serial.printf("zRot %d, yLin %d, yRot %d, zRot %d, xRot %d\n", xLin,yLin,yRot,zRot,xRot);
				
				// Send signals to the servos
				// Note: 1000 = -45deg, 2000 = +45deg

				if (rightServoConnected != 0) 
					RightServo.writeMicroseconds(RightServo_ZERO + stroke + roll);
				if (leftServoConnected != 0) 
					LeftServo.writeMicroseconds(LeftServo_ZERO - stroke + roll);
				if (pitchServoConnected != 0) 
					PitchLeftServo.writeMicroseconds(PitchLeftServo_ZERO - pitch);
			}

			int valve,twist;
			valve  = xValve - 500;
			valve  = constrain(valve, -500, 500);
			if (!SettingsHandler::continousTwist) 
			{
				twist  = 2*(xRot - map(twistPos,-1500,1500,1000,1));
				twist = constrain(twist, -750, 750);
			} else {
				twist = map(xRot,1,1000,-180,180);
			}

			if (valveServoConnected != 0) 
				ValveServo.writeMicroseconds((ValveServo_ZERO - 500) + valve);
			if (twistServoConnected != 0) 
				TwistServo.writeMicroseconds(TwistServo_ZERO + twist);

            // Done with servo channels

            // Output vibration channels
            // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
            if ((vibe0 > 1) && (vibe0 <= 1000)) 
                analogWrite(Vibe0_PIN,map(vibe0,2,1000,31,255));
			else 
                analogWrite(Vibe0_PIN,0);

            if ((vibe1 > 1) && (vibe1 <= 1000)) 
                analogWrite(Vibe1_PIN,map(vibe1,2,1000,31,255));
			else if (digitalRead(Lube_PIN) == HIGH) 
				// For iLube - if no software action, check the manual button too
				analogWrite(Vibe1_PIN, lubeAmount);
			else 
      			analogWrite(Vibe1_PIN,0);

            // Done with vibration channels
            
        }
    }

	// Function to calculate the angle for the main arm servos
	// Inputs are target x,y coords of receiver pivot in 1/100 of a mm
	int SetMainServo(float x, float y) 
	{
		x /= 100; y /= 100;          // Convert to mm
		float gamma = atan2(x,y);    // Angle of line from servo pivot to receiver pivot
		float csq = sq(x) + sq(y);   // Square of distance between servo pivot and receiver pivot
		float c = sqrt(csq);         // Distance between servo pivot and receiver pivot
		float beta = acos((csq - 28125)/(100*c));  // Angle between c-line and servo arm
		int out = ms_per_rad*(gamma + beta - 3.14159); // Servo signal output, from neutral
		return out;
	}


	// Function to calculate the angle for the pitcher arm servos
	// Inputs are target x,y,z coords of receiver upper pivot in 1/100 of a mm
	// Also pitch in 1/100 of a degree
	int SetPitchServo(float x, float y, float z, float pitch) 
	{
		pitch *= 0.0001745; // Convert to radians
		x += 5500*sin(0.2618 + pitch);
		y -= 5500*cos(0.2618 + pitch);
		x /= 100; y /= 100; z /= 100;   // Convert to mm
		float bsq = 36250 - sq(75 + z); // Equivalent arm length
		float gamma = atan2(x,y);       // Angle of line from servo pivot to receiver pivot
		float csq = sq(x) + sq(y);      // Square of distance between servo pivot and receiver pivot
		float c = sqrt(csq);            // Distance between servo pivot and receiver pivot
		float beta = acos((csq + 5625 - bsq)/(150*c)); // Angle between c-line and servo arm
		int out = ms_per_rad*(gamma + beta - 3.14159); // Servo signal output, from neutral
		return out;
	}
};
