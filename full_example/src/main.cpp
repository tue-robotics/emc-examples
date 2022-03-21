/**
 * Example code for the MRC (formerly EMC) course.
 * Pico (the robot) starts driving forward using its drive forward skill.
 * In the meantime, it continuously monitors its distance to the wall using its detection skill. 
 * When a wall is detected, Pico can not drive forward anymore and needs to turn. 
 * The state switches and another set of skills is called: drive backward and rotate.
 */

#include <emc/io.h>
#include <emc/rate.h>
#include <cmath>
#include <iostream>

#include "worldModel.h"
#include "detection.h"
#include "driveControl.h"
#include "config.h"

typedef enum {
    drive_forward = 1,
    drive_backward,
    rotate,
} state_t;

int main(int argc, char *argv[])
{
    // Initialize the robot
    emc::Rate r(EXECUTION_RATE);
    emc::IO io;
    
    // Initialize the Classes
    WorldModel worldModel;
    Detection detection(&io);
    DriveControl picoDrive(&io);

    // Initialize the state of the State Machine
    state_t state = drive_forward;
    double rotatedAngle = 0.0;
    double distanceBackwards = 0.0;

    /* Main Execution Loop, 
     * this loop keeps running until io.ok returns false, i.e. when a robot error occurs. */
    while(io.ok())
    {
        // Get the sensor data from the LRF
        if(detection.getSensorData())
        {
            // Feed the WorldModel
            worldModel.setMinimumDistance(&(detection.laser));

            // State Machine
            switch(state) 
            {
                // case drive_forward: the robot drives forward until a wall is detected
                case drive_forward:
                    if(detection.wallDetected(*(worldModel.getMinimumDistance()))) {
                        // If a wall is detected, stop before we hit the wall 
                        picoDrive.stop();
                        // Reset rotatedAngle to 0
                        rotatedAngle = 0.0;
                        // Reset distanceBackwards to 0
                        distanceBackwards = 0.0;
                        // Switch state to move backwards
                        state = drive_backward;
                    } else {
                        picoDrive.driveForward(FORWARD_SPEED);
                    }
                    break;

                // case drive_backward: the robot drives backward
                case drive_backward:
                    // Start driving backwards, add distance driven to counter distanceBackwards
                    distanceBackwards += picoDrive.driveBackward(FORWARD_SPEED);
                    // If we have driven backwards far enough,
                    if(fabs(distanceBackwards) >= DIST_BACKWARDS) {
                        // we start rotating.
                        state = rotate;
                    }
                    break;

                // case rotate: the robot rotates
                case rotate:
                    // Start rotating, add angular displacement to counter rotatedAngle
                    rotatedAngle += picoDrive.rotate(ROTATE_SPEED);	
                    // If we have rotated enough,
                    if(fabs(rotatedAngle) >= ROTATED_ANGLE) {
                        // start driving again.
                        state = drive_forward;
                    }
                    break;

                default:
                    picoDrive.stop();
                    break;
            }
        } 
        else 
        {
            picoDrive.stop();
        }

        // Use this to ensure an execution rate of 20 Hertz
        r.sleep();
    }

    return 0;
}
