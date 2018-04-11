package gov.dot.fhwa.saxton.carma.guidance.lanechange;

import cav_msgs.*;
import gov.dot.fhwa.saxton.carma.guidance.IGuidanceCommands;
import gov.dot.fhwa.saxton.carma.guidance.ManeuverPlanner;
import gov.dot.fhwa.saxton.carma.guidance.conflictdetector.IConflictDetector;
import gov.dot.fhwa.saxton.carma.guidance.lightbar.ILightBarManager;
import gov.dot.fhwa.saxton.carma.guidance.lightbar.IndicatorStatus;
import gov.dot.fhwa.saxton.carma.guidance.lightbar.LightBarIndicator;
import gov.dot.fhwa.saxton.carma.guidance.maneuvers.*;
import gov.dot.fhwa.saxton.carma.guidance.mobilityrouter.IMobilityRouter;
import gov.dot.fhwa.saxton.carma.guidance.mobilityrouter.MobilityRequestHandler;
import cav_msgs.MobilityResponse;
import cav_msgs.MobilityRequest;
import gov.dot.fhwa.saxton.carma.guidance.mobilityrouter.MobilityResponseHandler;
import gov.dot.fhwa.saxton.carma.guidance.plugins.AbstractPlugin;
import gov.dot.fhwa.saxton.carma.guidance.plugins.ITacticalPlugin;
import gov.dot.fhwa.saxton.carma.guidance.plugins.PluginServiceLocator;
import gov.dot.fhwa.saxton.carma.guidance.pubsub.IPublisher;
import gov.dot.fhwa.saxton.carma.guidance.pubsub.ISubscriber;
import gov.dot.fhwa.saxton.carma.guidance.pubsub.OnMessageCallback;
import gov.dot.fhwa.saxton.carma.guidance.trajectory.Trajectory;
import gov.dot.fhwa.saxton.carma.guidance.util.trajectoryconverter.ITrajectoryConverter;
import gov.dot.fhwa.saxton.carma.guidance.util.trajectoryconverter.RoutePointStamped;
import gov.dot.fhwa.saxton.carma.guidance.conflictdetector.ConflictSpace;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * This is a mandatory plugin for the Carma platform that manages all lane change activity within a given sub-trajectory,
 * with the intent of changing from the current lane into one of the adjacent lanes only (no multi-lane hopping).
 *
 * It will create a FutureLongitudinalManeuver place holder for both lateral and longitudinal dimensions, which allows the parent
 * strategic plugin to continue planning the remainder of the trajectory around this space. In parallel to that activity,
 * this plugin will plan the details of the lane change, inserting one or more maneuvers into the FutureLongitudinalManeuver space
 * as necessary to accomplish the mission. This process will involve determining if neighbor vehicles are in the
 * desired target lane space, and, if so, negotiating a coordinated movement with one or more of them to ensure that
 * enough space is opened up in the target lane. Because this negotiation could take significant time, it will be
 * allowed to run in parallel with the planning of the rest of the trajectory (by the parent plugin), and, in fact,
 * can proceed even during execution of the early part of that trajectory, right up to the point that the contents of
 * this resultant FutureLongitudinalManeuver needs to be executed. At that time, if its planning is incomplete an exception will be
 * thrown and that trajectory will be aborted by the parent.
 */

    //TODO - this plugin was written in a very limited amount of time, so has made a lot of assumptions and simplifications.
    //       Below are some of the major things known now that should be refactored (not necessarily a complete list):
    //
    //  - As soon as negotiations fail, we should let arbitrator know so that it has a chance to abort & replace the
    //  current trajectory in an orderly way; otherwise the only way arbitrator will know is when the trajectory fails
    //  ugly because it attempts to execute an empty FutureLongitudinalManeuver.
    //
    //  - We have a very simplistic model for what our neighbor situation will look like at the time of maneuver
    //  execution. This could be beefed up in many ways.
    //
    //  - There are several complementary simplifications built into the Negotiator node as well.
    //
    //  - Assumes there is only one lane change being planned at a time. So the planSubtrajectory() method will not get
    //  called while the loop() method is actively working on current negotiations.
    //
    //  - Lots of "TODO"s throughout this package

public class LaneChangePlugin extends AbstractPlugin implements ITacticalPlugin, MobilityResponseHandler {

    private long                            EXPIRATION_TIME = 500;
    private final int                       SLEEP_TIME = 50; //ms
    private int                             targetLane_ = -1;
    private double                          startSpeed_ = 0.0;
    private double                          endSpeed_ = 0.0;
    private MobilityRequest                 plan_ = null;
    private FutureLongitudinalManeuver      futureLonMvr_ = null;
    private FutureLateralManeuver           futureLatMvr_ = null;
    private LaneChange                      laneChangeMvr_ = null;
    private final String                    STATIC_ID;
    private final String                    MOBILITY_STRATEGY = "Carma/LaneChange";
    private IPublisher                      requestPub_;
    private String                          mostRecentPlanId_ = "";
    private final long                      MS_PER_S = 1000L;
    // Light bar control variables
    private ILightBarManager lightBarManager_;
    private final LightBarIndicator LIGHT_BAR_INDICATOR = LightBarIndicator.YELLOW;
    private ISubscriber<UIInstructions> uiInstructionsSubscriber_;
    private boolean conductingLaneChange_ = false;
    private final long LANE_CHANGE_TIMEOUT = 500; // mili-seconds
    private long lastLaneChangeMsg_ = 0; // mili-seconds


    public LaneChangePlugin(PluginServiceLocator psl) {
        super(psl);
        version.setName("Lane Change Plugin");
        version.setMajorRevision(1);
        version.setIntermediateRevision(0);
        version.setMinorRevision(1);
        STATIC_ID = UUID.randomUUID().toString();
        pluginServiceLocator.getMobilityRouter().registerMobilityResponseHandler(MOBILITY_STRATEGY, this);
    }

    @Override
    public void onInitialize() {
        // Load Params
        this.EXPIRATION_TIME = pluginServiceLocator.getParameterSource().getInteger("~lane_change_nack_timeout", 500);
        log.info("Loaded param lane_change_nack_timeout: " + EXPIRATION_TIME);

        // Get light bar manager
        lightBarManager_ = pluginServiceLocator.getLightBarManager();

        // Get MobilityRequest publisher
        requestPub_ = pluginServiceLocator.getPubSubService().getPublisherForTopic("outgoing_mobility_request", cav_msgs.MobilityRequest._TYPE);

        // get subscriber for the ui instructions and use to set light bar
        uiInstructionsSubscriber_ = pubSubService.getSubscriberForTopic( "ui_instructions", UIInstructions._TYPE);
        uiInstructionsSubscriber_.registerOnMessageCallback(
            (UIInstructions msg) -> {
                if (msg.getMsg().equals("LEFT_LANE_CHANGE") && conductingLaneChange_ == false) {

                    conductingLaneChange_ = true;
                    lastLaneChangeMsg_ = System.currentTimeMillis();
                    setLightBarStatus(IndicatorStatus.LEFT_ARROW);

                } else if (msg.getMsg().equals("RIGHT_LANE_CHANGE") && conductingLaneChange_ == false) {

                    conductingLaneChange_ = true;
                    lastLaneChangeMsg_ = System.currentTimeMillis();
                    setLightBarStatus(IndicatorStatus.RIGHT_ARROW);
                }
        });

    }

    @Override
    public void onResume() {
        //indicate always available
        setAvailability(true);
    }


    @Override
    public void loop() throws InterruptedException {

        // If we are waiting on a plan to be nacked
        if (plan_ != null) {
            synchronized (plan_) {
                if (plan_ != null) {
                    // Check if enough time has passed without a NACK
                    if (System.currentTimeMillis() - plan_.getHeader().getTimestamp() > EXPIRATION_TIME) {
                        // Add the actual lane change to the trajectory
                        populateFutureManeuverWithLaneChange();
                        // Done negotiating
                        plan_ = null;
                    }
                }
            }
        }

        // Release control of light bar when lane change is done
        if (System.currentTimeMillis() - lastLaneChangeMsg_ > LANE_CHANGE_TIMEOUT && conductingLaneChange_ == true) {
            releaseControlAndTurnOff();
            conductingLaneChange_ = false;
        }
        //sleep a while
        Thread.sleep(SLEEP_TIME);
    }


    @Override
    public void onSuspend() {
        //indicate not available - any in-progress negotiations at this time are subject to timing out
        setAvailability(false);
    }


    @Override
    public void onTerminate() {
        //nothing to do
    }


    public void setLaneChangeParameters(int targetLane, double startSpeed, double endSpeed) {
        targetLane_ = targetLane;
        startSpeed_ = startSpeed;
        endSpeed_ = endSpeed;
    }


    @Override
    public boolean planSubtrajectory(Trajectory traj, double startDistance, double endDistance) {
        log.info(String.format("In planSubtrajectory with params = {laneId=%d,startLimit=%.02f,endLimit=%.02f,start=%.02f,end=%.02f}",
            targetLane_,
            startSpeed_,
            endSpeed_,
            startDistance,
            endDistance));

        //verify that the input parameters have been defined already
        if (targetLane_ > -1) {

            //attempt to plan the lane change
            try {
                plan(startDistance, endDistance, targetLane_, startSpeed_, endSpeed_);
            }catch (IllegalStateException e) {
                // if we can't fit the maneuver in the space available, no point in starting negotiations to do so;
                // abort the whole subtrajectory, with no future maneuver inserted
                log.warn("Plan returned false!");
                return false;
            }

            //create empty containers (future compound maneuvers) for the TBD maneuvers to be inserted into
            ManeuverPlanner planner = pluginServiceLocator.getManeuverPlanner();
            IManeuverInputs inputs = planner.getManeuverInputs();
            futureLatMvr_ = new FutureLateralManeuver(this,  targetLane_ - inputs.getCurrentLane(), inputs, startDistance, startSpeed_, endDistance, endSpeed_);
            futureLonMvr_ = new FutureLongitudinalManeuver(this, inputs, startDistance, startSpeed_, endDistance, endSpeed_);

            //insert these containers into the trajectory
            if (!traj.addManeuver(futureLatMvr_)  ||  !traj.addManeuver(futureLonMvr_)) {
                log.warn("Unable to add lane change future maneuvers to trajectory");
                return false;
            }

            //if we've reached this point, a future maneuver is at least possible, so parent can continue planning
            return true;

        }else {
            log.warn("V2V", "planSubtrajectory aborted because lane change parameters have not been defined.");
            return false;
        }
    }

    /**
     * Plans the tactical operation (one or more maneuvers), which may include negotiating with neighbor vehicles.
     * @param startDist - starting location of the tactic, meters from beginning of route
     * @param endDist - ending location of the tactic, meters from beginning of route
     * @param targetLane - ID of the lane we are heading toward
     * @param startSpeed - speed at the start of the tactical operation, m/s
     * @param endSpeed - speed at the end of the tactical operation, m/s
     * @throws IllegalStateException if the lane change is geometrically infeasible
     */
    private void plan(double startDist, double endDist, int targetLane, double startSpeed, double endSpeed)
                        throws IllegalStateException {
        boolean planAvailable = false;
        ManeuverPlanner planner = pluginServiceLocator.getManeuverPlanner();
        IManeuverInputs inputs = planner.getManeuverInputs();
        double curDist = inputs.getDistanceFromRouteStart();
        double curSpeed = inputs.getCurrentSpeed();

        // Estimate starting point of maneuver
        // TODO this is a very simplistic estimate and could be improved
        long futureTime = System.currentTimeMillis() + (long)(1000.0*2.0*(startDist - curDist)/(startSpeed + curSpeed));
        log.info("Expected arrival time at lane change area = " + futureTime);
        // TODO Estimate starting crosstrack using trajectory being planned
        double startCrosstrack = inputs.getCrosstrackDistance();                    
        RoutePointStamped startPoint = new RoutePointStamped(startDist, startCrosstrack, futureTime);

        // Convert the future maneuver to a set of route points
        Trajectory laneChangeTraj = new Trajectory(futureLonMvr_.getStartDistance(), futureLonMvr_.getEndDistance());
        laneChangeTraj.addManeuver(futureLonMvr_);
        laneChangeTraj.addManeuver(futureLatMvr_);

        ITrajectoryConverter trajectoryConverter = pluginServiceLocator.getTrajectoryConverter();
        List<RoutePointStamped> routePoints = trajectoryConverter.convertToPath(laneChangeTraj, startPoint);

        buildRequest(inputs, targetLane, routePoints);

        //construct our proposed simple lane change maneuver
        log.info("Creating lane change maneuver");
        laneChangeMvr_ = new LaneChange(this, futureLatMvr_.getEndingRelativeLane());
        laneChangeMvr_.setTargetLane(targetLane);
        if (planner.canPlan(laneChangeMvr_, startDist, endDist)) {
            log.info("Planning lane change maneuver...");
            planner.planManeuver(laneChangeMvr_, startDist, endDist);
            log.info("V2V", "plan: simple lane change maneuver is built.");
        }else {
            //TODO - would be nice to have some logic here to diagnose the problem and try again
            laneChangeMvr_ = null;
            log.warn("V2V", "plan: unable to construct the simple lane change maneuver.");
            throw new IllegalStateException("Proposed lane change maneuver won't fit the geometry.");
        }
    }


    /**
     * Constructs a MobilityRequest message for a lane change
     * 
     * @param inputs Maneuver inputs for accessing vehicle state
     * @param targetLane The lane index we intend to change to
     * @param routePoints The list of points which describe the lane change motion to be performed
     * 
     * @return A constructed MobilityRequest
     */
    private MobilityRequest buildRequest(IManeuverInputs inputs, int targetLane, List<RoutePointStamped> routePoints) {
        ITrajectoryConverter trajectoryConverter = pluginServiceLocator.getTrajectoryConverter();
        // Convert path to trajectory message
        cav_msgs.Trajectory trajMsg = trajectoryConverter.pathToMessage(routePoints);
        // TODO we need some sort of timeout for an abort?
        MobilityRequest requestMsg = (MobilityRequest) requestPub_.newMessage();
        requestMsg.setStrategy(MOBILITY_STRATEGY);
        requestMsg.setUrgency((short)500);

        if (targetLane_ < inputs.getCurrentLane()) {
            requestMsg.getPlanType().setType(cav_msgs.PlanType.CHANGE_LANE_RIGHT);
        } else {
            requestMsg.getPlanType().setType(cav_msgs.PlanType.CHANGE_LANE_LEFT);
        }

        RoutePointStamped currentLocation = new RoutePointStamped(inputs.getDistanceFromRouteStart(), inputs.getCrosstrackDistance(), System.currentTimeMillis());
        cav_msgs.Trajectory currentLocationMsg = trajectoryConverter.pathToMessage(Arrays.asList(currentLocation));
        requestMsg.setLocation(currentLocationMsg.getLocation());
        requestMsg.setStrategyParams("LANE_CHANGE_PARAM");
        
        requestMsg.setTrajectory(trajMsg);
        // Build header
        requestMsg.getHeader().setSenderId(pluginServiceLocator.getMobilityRouter().getHostMobilityId());
        requestMsg.getHeader().setRecipientId(""); //Broadcast
        requestMsg.getHeader().setTimestamp((long)(currentLocation.getStamp() * MS_PER_S));
        requestMsg.getHeader().setPlanId(UUID.randomUUID().toString());
        requestMsg.getHeader().setSenderBsmId("FFFFFFFF"); // TODO use real BSM id
        requestMsg.setExpiration(requestMsg.getHeader().getTimestamp() + EXPIRATION_TIME);
        
        return requestMsg;
    }


    /**
     * Builds and publishes a MobilityRequest message to notify other CAVs of our intended lane change
     * 
     * @param inputs Maneuver inputs for accessing vehicle state
     * @param targetLane The lane index we intend to change to
     * @param routePoints The list of points which describe the lane change motion to be performed
     */
    private void publishRequestMessage(IManeuverInputs inputs, int targetLane,  List<RoutePointStamped> routePoints) {
        MobilityRequest requestMsg = buildRequest(inputs, targetLane, routePoints);
        
        mostRecentPlanId_ = requestMsg.getHeader().getPlanId();
        
        synchronized (plan_) {
            plan_ = requestMsg;
            requestPub_.publish(plan_);
        }
    }

    @Override
    public void handleMobilityResponseMessage(MobilityResponse msg) {
        // Check if this message is for the most recent plan we are tracking
        if (msg.getHeader().getPlanId().equals(mostRecentPlanId_)) {
            synchronized (plan_) {
                // Check if we are still tracking this plan
                if (plan_ != null) {
                    // If the request was rejected (NACK)
                    if (!msg.getIsAccepted()) {
                        // Abandon the lane change
                        populateFutureManeuverWithSteadySpeed();
                        plan_ = null;
                    }
                }
            }
        }
    }

    /**
     * Fills up the future maneuver structure from beginning to end with lane change and steady speed maneuvers
     */
    private void populateFutureManeuverWithLaneChange() {
        ManeuverPlanner planner = pluginServiceLocator.getManeuverPlanner();
        IManeuverInputs inputs = planner.getManeuverInputs();
        IGuidanceCommands commands = planner.getGuidanceCommands();

        try {
            //start the lane change immediately
            futureLatMvr_.addManeuver(laneChangeMvr_);

            //fill the remainder with a constant lane
            double startDist = futureLatMvr_.getLastDistance();
            double endDist = futureLatMvr_.getEndDistance();
            if (endDist - startDist > 0) {
                LaneKeeping lk = new LaneKeeping(this);
                lk.planToTargetDistance(inputs,commands, startDist, endDist);
                futureLatMvr_.addManeuver(lk);
            }

            //fill the whole longitudinal space with a constant speed
            SteadySpeed ss = new SteadySpeed(this);
            planner.planManeuver(ss, futureLonMvr_.getStartDistance(), endDist);
            futureLonMvr_.addManeuver(ss);

        }catch (IllegalStateException ise) {
            //log it to clarify the call sequence
            log.warn("V2V", "Exception trapped in populateFutureManeuver: " + ise.toString());
            throw ise;
        }
    }

    /**
     * Fills up the future maneuver structure from beginning to end with steady speed and lane keeping maneuvers
     */
    private void populateFutureManeuverWithSteadySpeed() {
        ManeuverPlanner planner = pluginServiceLocator.getManeuverPlanner();
        IManeuverInputs inputs = planner.getManeuverInputs();
        IGuidanceCommands commands = planner.getGuidanceCommands();

        try {
            // fill the whole longitudinal space with a constant speed
            SteadySpeed ss = new SteadySpeed(this);
            planner.planManeuver(ss, futureLonMvr_.getStartDistance(), futureLonMvr_.getEndDistance());
            futureLonMvr_.addManeuver(ss);

            // Fill the whole lateral space with 
            LaneKeeping laneKeeping = new LaneKeeping(this);
            laneKeeping.planToTargetDistance(inputs, commands, futureLatMvr_.getStartDistance(), futureLatMvr_.getEndDistance());
            futureLatMvr_.addManeuver(laneKeeping);

        }catch (IllegalStateException ise) {
            // log it to clarify the call sequence
            log.warn("V2V", "Exception trapped in populateFutureManeuver: " + ise.toString());
            throw ise;
        }
    }

    /**
     * Helper function to acquire control of the light bar
     */
    private void releaseControlAndTurnOff() {
        if (lightBarManager_ == null) {
            return;
        }
        lightBarManager_.setIndicator(LIGHT_BAR_INDICATOR, IndicatorStatus.OFF, this.getVersionInfo().componentName());
        lightBarManager_.releaseControl(Arrays.asList(LIGHT_BAR_INDICATOR), this.getVersionInfo().componentName());
    }

    /**
     * Attempts to set the lane change controlled light bar indicator
     * 
     * @param status the indicator status to set
     */
    protected void setLightBarStatus(IndicatorStatus status) {
        if (lightBarManager_ == null) {
            return;
        }
        // Request control every time as we will release control when a lane change is done
        List<LightBarIndicator> acquired = lightBarManager_.requestControl(Arrays.asList(LIGHT_BAR_INDICATOR), this.getVersionInfo().componentName(),
            // Lost control of light call back
            (LightBarIndicator lostIndicator) -> {
                log.info("Lost control of light bar indicator: " + LIGHT_BAR_INDICATOR);
         });
        // Check if the control request was successful. 
        if (acquired.contains(LIGHT_BAR_INDICATOR)) {
            lightBarManager_.setIndicator(LIGHT_BAR_INDICATOR, status, this.getVersionInfo().componentName());
            log.info("Got control of light bar indicator: " + LIGHT_BAR_INDICATOR);
        }
    }
    
}
