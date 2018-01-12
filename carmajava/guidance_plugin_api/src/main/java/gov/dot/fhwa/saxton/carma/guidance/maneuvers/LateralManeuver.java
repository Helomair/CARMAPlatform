/*
 * Copyright (C) 2017 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package gov.dot.fhwa.saxton.carma.guidance.maneuvers;

import gov.dot.fhwa.saxton.carma.guidance.IGuidanceCommands;

/**
 * Base class for all lateral maneuvers.
 */
public abstract class LateralManeuver extends ManeuverBase {

    protected boolean completed = false;
    protected long startTime_ = 0;
    protected double maxAxleAngleRate = 0.0; 
    protected double maxAccel_ = 0.0;
    protected int targetLane_ = 0;
    protected double axleAngle_ = 0.0; // rad: Angle in radians to turn the wheels. Positive is left, Negative is right
    protected double lateralAccel_ = 0.0; // Max acceleration which can be caused by a turn
    protected double yawRate_ = 0.0;  // rad/s: Max axel angle velocity

    public LateralManeuver() {
    }

    @Override
    public void plan(IManeuverInputs inputs, IGuidanceCommands commands, double startDist)
            throws IllegalStateException {
        super.plan(inputs, commands, startDist);
    }

    @Override
    public double planToTargetDistance(IManeuverInputs inputs, IGuidanceCommands commands, double startDist,
            double endDist) throws IllegalStateException, ArithmeticException {
        return super.planToTargetDistance(inputs, commands, startDist, endDist);
    }

    @Override
    public boolean executeTimeStep() {
        verifyLocation();

        if (startTime_ == 0) {
            startTime_ = System.currentTimeMillis();
        }
        //TODO
        commands_.setSteeringCommand(axleAngle_, lateralAccel_, yawRate_);
        return true;
    }

    /**
     * Stores the target lane ID, to be used for lateral maneuvers only.
     * @param targetLane - target lane number at end of maneuver
     * @throws UnsupportedOperationException if called on a longitudinal maneuver object
     */
    public void setTargetLane(int targetLane) {
        targetLane_ = targetLane;
    }

    /**
     * Sets the acceleration constraints for this maneuver 
     *
     * @param yawRate Max axel angle velocity (How fast the wheel can turn)
     * @param lateralAccel Max acceleration which can be caused by a turn
     */
    public void setDynamicLimits(double yawRate, double lateralAccel) {
        yawRate_ = yawRate;
        lateralAccel_ = lateralAccel;
    }
}
