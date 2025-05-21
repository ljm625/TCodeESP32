/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#pragma once
#include <list>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"

class CatToyHandler{
    public: 
        static float avg_distance;
        static bool isConnected;
        static int distance_count;
        static float distance;
        void setup() {
            LogHandler::info(_TAG, "Setting up CatToy");
            if(SettingsHandler::enableLaser){
                pinMode(SettingsHandler::Laser_PIN,OUTPUT);
            }
            if(SettingsHandler::enableDistance){
                pinMode(SettingsHandler::TRIG_PIN, OUTPUT);  
                pinMode(SettingsHandler::ECHO_PIN, INPUT); 
            }
            last_motion_time = 0;
            distance_count = 0;
        }

        void run(bool motion_enabled){
            if(SettingsHandler::enableDistance){
                digitalWrite(SettingsHandler::TRIG_PIN, LOW);  
                delayMicroseconds(2);  
                digitalWrite(SettingsHandler::TRIG_PIN, HIGH);  
                delayMicroseconds(10);  
                digitalWrite(SettingsHandler::TRIG_PIN, LOW);  
                float duration = pulseIn(SettingsHandler::ECHO_PIN, HIGH); 
                distance = (duration*.0343)/2;
                if(distance_count==0){
                    avg_distance = distance;
                } else{
                    avg_distance = (distance+(avg_distance*distance_count))/(distance_count+1);
                }
                if(distance_count<=1000){
                    distance_count+=1;
                }
            }
            if(motion_enabled){
                last_motion_time = millis();
                bool send_laser = true;
                if(SettingsHandler::enableDistance){
                    if(distance<=float(SettingsHandler::minDistance)){
                        send_laser = false;
                    }
                    if((abs(distance-avg_distance))>=10){
                        send_laser = false;
                    }
                }
                if(SettingsHandler::enableLaser && send_laser){
                    digitalWrite(SettingsHandler::Laser_PIN,HIGH);
                } else{
                    digitalWrite(SettingsHandler::Laser_PIN,LOW);

                }
            } else{
                digitalWrite(SettingsHandler::Laser_PIN,LOW);
            }
        }
        
    private:
        // std::mutex serial_mtx;
        // std::mutex command_mtx;
// unsigned long lastCall;
        const char* _TAG = TagHandler::CatToyHandler;
        static unsigned long last_motion_time;

};
