/**************************************************************************
 * RCMS Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Author: Souvik Das 		  *
  *************************************************************************/

package rcms.fm.app.pixelfm;

import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Iterator;

import rcms.errorFormat.CMS.CMSError;
import rcms.fm.fw.StateEnteredEvent;
import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.FunctionManagerParameter;
import rcms.fm.fw.parameter.FunctionManagerParameter.Exported;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.parameter.type.IntegerT;
import rcms.fm.fw.parameter.type.StringT;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserStateNotificationHandler;
import rcms.fm.resource.QualifiedGroup;
import rcms.fm.resource.QualifiedResource;
import rcms.fm.resource.QualifiedResourceContainer;
import rcms.fm.resource.qualifiedresource.JobControl;
import rcms.fm.resource.qualifiedresource.XdaqExecutive;
import rcms.fm.resource.qualifiedresource.XdaqApplication;
import rcms.fm.resource.qualifiedresource.XdaqApplicationContainer;
import rcms.fm.resource.qualifiedresource.XdaqApplicationContainerException;
import rcms.stateFormat.StateNotification;
import rcms.util.logger.RCMSLogger;
import rcms.fm.resource.CommandException;

import rcms.fm.fw.parameter.Parameter;
import rcms.utilities.runinfo.RunInfo;
import rcms.utilities.runinfo.RunInfoException;
import rcms.utilities.runinfo.RunInfoConnectorIF;

import rcms.xdaqctl.XDAQMessage;
import net.hep.cms.xdaqctl.XDAQMessageException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;


public class PixelEventHandler extends UserStateNotificationHandler {
	
	/**
	 * <code>RCMSLogger</code>: RCMS log4j logger.
	 */
	static RCMSLogger logger = new RCMSLogger(PixelEventHandler.class);
        private static final String XDAQ_NS = "urn:xdaq-soap:3.0";
	
	PixelFunctionManager functionManager = null;
	
	private QualifiedGroup qualifiedGroup = null;
        private XdaqApplicationContainer pixelSupervisors = null;
	
	public PixelEventHandler() throws rcms.fm.fw.EventHandlerException {
		// this handler inherits UserStateNotificationHandler
		// so it is already registered for StateNotification events
		
		// Let's register also the StateEnteredEvent triggered when the FSM enters in a new state.
		subscribeForEvents(StateEnteredEvent.class);
                subscribeForEvents(StateNotification.class);
		
		addAction(PixelStates.INITIALIZING,			"initAction");
                addAction(PixelStates.COLDRESETTING,                    "coldResetAction");
		addAction(PixelStates.CONFIGURING, 			"configureAction");
                //addAction(PixelStates.CONFIGURED,                       "configuredAction");
		addAction(PixelStates.HALTING,     			"haltAction");
		addAction(PixelStates.PREPARING_TTSTEST_MODE,	"preparingTTSTestModeAction");
		addAction(PixelStates.TESTING_TTS,    			"testingTTSAction");
		addAction(PixelStates.PAUSING,     			"pauseAction");
		addAction(PixelStates.RECOVERING,  			"recoverAction");
		addAction(PixelStates.RESETTING,   			"resetAction");
		addAction(PixelStates.RESUMING,    			"resumeAction");
		addAction(PixelStates.STARTING,    			"startAction");
		addAction(PixelStates.STOPPING,    			"stopAction");
		addAction(PixelStates.RUNNING,                          "runAction" );
		addAction(PixelStates.FIXINGSOFTERROR,                    "fixedSEUAction" );
		addAction(PixelStates.RUNNINGSOFTERRORDETECTED,                   "fixingSEUAction"); // SEU transition states 
		addAction(PixelStates.RUNNINGDEGRADED,                  "runningDegradedAction" ); // Running Degraded
		
	}
	
	
	public void init() throws rcms.fm.fw.EventHandlerException {
		functionManager = (PixelFunctionManager) getUserFunctionManager();
		qualifiedGroup  = functionManager.getQualifiedGroup();

		// debug
		logger.debug("init() called: functionManager=" + functionManager );
	}
	
	

	public void initAction(Object obj) throws UserActionException, XdaqApplicationContainerException {



		if (obj instanceof StateNotification) {
			
			// triggered by State Notification from child resource
			
			return;
		}
		
		else if (obj instanceof StateEnteredEvent) {
		    
			// triggered by entered state action
			// let's command the child resources
			
			// debug
			logger.debug("initAction called.");
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("hello, world")));
			
			//			 get the parameters of the command
			ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();
			
			IntegerT sid = new IntegerT(-1);
			StringT gck = new StringT("dummyKey");
			
			////// Export parameters required by Central RCMS //////
			// check parameter set
			if (parameterSet.size()==0 ) {
			    System.out.println("[initAction] input parameter set was empty");
			}
			else if ( parameterSet.get(PixelParameters.SID) == null )  {
			    System.out.println("[initAction] SID parameter is null");
			}
			else {
			    // get the SID
			    sid = ((IntegerT)parameterSet.get(PixelParameters.SID).getValue());
			    System.out.println("[initAction] received SID");
			}
			System.out.println("[initAction] SID = "+sid);
			functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.SID,sid,Exported.READONLY));
			functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.INITIALIZED_WITH_SID,sid,Exported.READONLY));
			
			if (parameterSet.size()==0 ) {
			    //don't need to see this output twice
			}
			else if ( parameterSet.get(PixelParameters.GLOBAL_CONF_KEY) == null )  {
			    System.out.println("[initAction] GLOBAL_CONF_KEY parameter is null");
			}
			else {
			    // get the global conf key
			    gck = ((StringT)parameterSet.get(PixelParameters.GLOBAL_CONF_KEY).getValue());//.getString();
			    System.out.println("[initAction] received GLOBAL_CONF_KEY");
			}
			System.out.println("[initAction] GLOBAL_CONF_KEY = "+gck);
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.GLOBAL_CONF_KEY,gck,Exported.READONLY));
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.INITIALIZED_WITH_GLOBAL_CONF_KEY,gck,Exported.READONLY));
			
			
			// 
			// initialize qualified group
			
			//
			QualifiedGroup qg = functionManager.getQualifiedGroup();

			try {
				qg.init();
			} catch (Exception e) {
				// failed to init
				String errMsg = this.getClass().toString() + " failed to initialize resources";
			
				// send error notification
				sendCMSError(errMsg);
		
				//log error
				logger.error(errMsg,e);
			
				// go to error state
				functionManager.fireEvent(PixelInputs.SETERROR);
			}

			// find xdaq executives
			functionManager.containerXdaqExecutive = new QualifiedResourceContainer(qg.seekQualifiedResourcesOfType(new XdaqExecutive()));

			// Find Job Controls
			functionManager.containerJobControl = new QualifiedResourceContainer(qg.seekQualifiedResourcesOfType(new JobControl()));

			// find xdaq applications
			List<QualifiedResource> xdaqList = qg.seekQualifiedResourcesOfType(new XdaqApplication());
			functionManager.containerXdaqApplication = new XdaqApplicationContainer(xdaqList);
			logger.debug("Application list : " + xdaqList.size() );

			//functionManager.pixelSupervisors = new XdaqApplicationContainer(functionManager.containerXdaqApplication.getApplicationsOfClass("PixelSupervisors"));

			functionManager.containerXdaqApplication.execute(PixelInputs.INITIALIZE, "PixelSupervisor");
			 
			// go to HALT
			functionManager.fireEvent( PixelInputs.SETHALT );
			
			
			// set action
			//functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));
			
			logger.info("initAction Executed");
		}
	}

	public void runAction( Object obj ) throws UserActionException, XdaqApplicationContainerException
	{
		logger.info( "run action called" );
		if( obj instanceof StateNotification )
		{
			StateNotification sn = (StateNotification) obj;
			String toState = sn.getToState();
			System.out.println( "[runAction] State Notification from Child Run " + toState );
			logger.info( "[runAction] State Notification from Child Run " + toState );
			if( toState.equals( "RunningSoftErrorDetected" ) )
			{
				System.out.println("[runAction] Got state RunningSoftErrorDetected from PixelSupervisor");
				functionManager.fireEvent( PixelInputs.SETSOFTERRORDETECTED );
			}
			if( toState.equals( "RunningDegradedDetected" ) )
			{
				System.out.println("[runAction] Got state RunningDegradedDetected from PixelSupervisor");
				functionManager.fireEvent( PixelInputs.SETRUNNINGDEGRADED );

			}
			if( toState.equals( "FixingSoftError" ) )
			{

				System.out.println("[runAction] Got state FixingSoftError from PixelSupervisor");
				functionManager.fireEvent( PixelInputs.FIXSOFTERROR );

			}
			
		}
		else if(obj instanceof StateEnteredEvent )
		{
			logger.info( "[runAction] called as StateEnteredEvent"  );
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT( "running" ) ) );
			functionManager.containerXdaqApplication.execute(PixelInputs.SETRUNNING, "PixelSupervisor");
			cleanUpFMParameters();
		}
		return;
	}
	
	public void runningDegradedAction(Object obj ) throws UserActionException, XdaqApplicationContainerException
	{
		logger.info( "running degraded action called! PixFM" );
		if( obj instanceof StateNotification )
		{
			StateNotification sn = (StateNotification) obj;
			String toState = sn.getToState();
			System.out.println( "[runningDegradedAction] State Notification from Child Run " + toState );
			logger.info( "[runningDegradedAction] State Notification from Child Run " + toState );
			if( toState.equals( "RunningSoftErrorDetected" ) )
			{
				System.out.println("[runningDegradedAction] Got state RunningSoftErrorDetected from PixelSupervisor");
				functionManager.fireEvent( PixelInputs.SETSOFTERRORDETECTED );
			}
			
		}
		else if(obj instanceof StateEnteredEvent )
		{
			logger.info( "[runningDegradedAction] called as StateEnteredEvent"  );
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT( "runningDegraded" ) ) );
			functionManager.containerXdaqApplication.execute(PixelInputs.SETRUNNINGDEGRADED, "PixelSupervisor");
			cleanUpFMParameters();
		}
		return;




	}
        
        public void coldResetAction(Object obj) throws UserActionException {
        
        logger.debug("coldResetAction called.");
        functionManager.fireEvent( PixelInputs.SETHALT );
        
        }

	public void fixingSEUAction( Object obj ) throws UserActionException, XdaqApplicationContainerException {

		logger.info("Executing fixingSEUAction");
		
		System.out.println("[fixingSEUAction] Got state RunningSoftErrorDetected from PixelSupervisor");
		if (obj instanceof StateNotification) 
		{
		
			// triggered by State Notification from child resource
		
			StateNotification sn = (StateNotification) obj;
			String toState = sn.getToState();
			System.out.println( "[fixingSEUAction] State Notification from Child Run " + toState ); 
			logger.info( "[fixingSEUAction] State Notification from Child Run " + toState );
			if (toState.equals("RunningSoftErrorDetected")) 
			{
		    		System.out.println("[fixingSEUAction] Got state RunningSoftErrorDetected from PixelSupervisor");
		    		functionManager.fireEvent( PixelInputs.SETSOFTERRORDETECTED );
			} 	
			else if( toState.equals( "FixSoftError" ) )
			{
				System.out.println("[SEUAction] Time to tell the Pixel Supervisor to fix that SoftError");
				functionManager.fireEvent( PixelInputs.FIXSOFTERROR );

			}
			else if( toState.equals( "Running" ) )
			{
				functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("FixSoftError")));
				functionManager.containerXdaqApplication.execute(PixelInputs.FIXSOFTERROR, "PixelSupervisor");

				System.out.println("[fixingSEUAction] Going to transition to running.");
				functionManager.fireEvent( PixelInputs.SETRUNNING );
				// functionManager.fireEvent( PixelInputs.SETRESUME
				// we're able to fix the soft error!
			}

			else 
			{
		    		System.out.println("[fixingSEUAction] Got something other than a valid state from PixelSupervisor!");
		    		functionManager.fireEvent(PixelInputs.SETERROR);
			}
		
			return;
	    	}
	    	else if (obj instanceof StateEnteredEvent) 
		{
			
			System.out.println("Executing fixingSEUAction- StateEnteredEvent");
			logger.info("Executing fixingSEUAction");
		
			// set action
			//functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("FixSoftError")));
			//functionManager.containerXdaqApplication.execute(PixelInputs.FIXSOFTERROR, "PixelSupervisor");
			
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("RunningSoftErrorDetected")));
			functionManager.containerXdaqApplication.execute(PixelInputs.SETSOFTERRORDETECTED, "PixelSupervisor");		

			// Clean-up of the Function Manager parameters
			cleanUpFMParameters();
		
			logger.info("fixingSEU Executed");
		
	    	}

	}
	public void fixedSEUAction( Object obj ) throws UserActionException, XdaqApplicationContainerException 
	{
		// Waiting to be notified of running
		if( obj instanceof StateNotification )
		{
			StateNotification sn = (StateNotification) obj;
			String toState = sn.getToState();
			if (toState.equals("Running")) 
			{

				// after fixing the SEU we transition immediately into Running
				System.out.println("[fixingSEUAction] Going to transition to running.");
				functionManager.fireEvent( PixelInputs.SETFIXEDSOFTERROR );
			}
			else
			{
				// We recieved an error!!!!!!
				System.out.println("[fixingSEUAction] Going to transition to error.");
				functionManager.fireEvent( PixelInputs.SETERROR );

			}

		}
		// Button click!
		else if (obj instanceof StateEnteredEvent) 
		{
			// set action- we fix the SEU 
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("FixSoftError")));
			functionManager.containerXdaqApplication.execute(PixelInputs.FIXSOFTERROR, "PixelSupervisor");



		}
		
	}
	

	public void resetAction(Object obj) throws UserActionException {
		
		if (obj instanceof StateNotification) {
			
			// triggered by State Notification from child resource
			
			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/
					
			return;
		}
		
		else if (obj instanceof StateEnteredEvent) {
			
						
				// triggered by entered state action
				// let's command the child resources
				
				// debug
				logger.debug("resetAction called.");
				
				// set action
				functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("Resetting")));
				
				/************************************************
				 * PUT HERE YOUR CODE							
				 ***********************************************/
				
				// go to Initital
				functionManager.fireEvent( PixelInputs.SETHALT );
				
				// Clean-up of the Function Manager parameters
				cleanUpFMParameters();
				
				logger.info("resetAction Executed");
		}	
	}
	
	public void recoverAction(Object obj) throws UserActionException, XdaqApplicationContainerException {
		
	    if (obj instanceof StateNotification) {
		
		// triggered by State Notification from child resource
		
		StateNotification sn = (StateNotification) obj;
		String toState = sn.getToState();
		if (toState.equals("Halted")) {
		    System.out.println("[recoverAction] Got state Halted from PixelSupervisor");
		    functionManager.fireEvent( PixelInputs.SETHALT );
		} else {
		    System.out.println("[recoverAction] Something other than Halted came back from PixelSupervisor!");
		    functionManager.fireEvent(PixelInputs.SETERROR);
		}
		
		return;
	    }
	    
	    else if (obj instanceof StateEnteredEvent) {
		
		System.out.println("Executing recoverAction");
		logger.info("Executing recoverAction");
		
		// set action
		functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("recovering")));
		functionManager.containerXdaqApplication.execute(PixelInputs.RECOVER, "PixelSupervisor");
		
		// Clean-up of the Function Manager parameters
		cleanUpFMParameters();
		
		logger.info("recoverAction Executed");
		
	    }
	}
	
	public void configureAction(Object obj) throws UserActionException {
		
		if (obj instanceof StateNotification) {
                
                 StateNotification sn = (StateNotification)obj;
                 String toState = sn.getToState();
		 String toStateReason = sn.getReason(); //this is the global key
		 //first problem. I can't figure out how to store this variable globally
		 //		 functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.PIXEL_GLOBAL_KEY,new StringT(toStateReason)));
                 if (toState.equals("Configured")) {
		     String msg = "Configured with Configuration Key = ";
		     String msgForLV0 = msg + toStateReason;
		     functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT(msgForLV0)));
		     System.out.println(msgForLV0);
		     functionManager.fireEvent( PixelInputs.SETCONFIGURE );
                 } else {
		     System.out.println("[configureAction] Something other than Configured came back from PixelSupervisor!");
		     functionManager.fireEvent(PixelInputs.SETERROR);
                 }

		 //this was for testing
		 //		 ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();
		 //System.out.println(parameterSet.get(PixelParameters.PIXEL_GLOBAL_KEY));

                
                 return;
		}
		
		else if (obj instanceof StateEnteredEvent) {
		    System.out.println("Executing configureAction"); //this is the one that is 'human readable' in /opt/rcms/pixelpro/..../catalina.out
			logger.info("Executing configureAction");
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("configuring")));
			
//			 get the parameters of the command
			ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();

			String runType="dummy";
			IntegerT runNum = new IntegerT(-1);
			StringT tpgKey = new StringT("dummyTpgKey");
			StringT fedEnableMask = new StringT("dummyFedEnableMask");

			// check parameter set
			if (parameterSet.size()==0 ) {
			    ///it seems that for now RUN_KEY will not necessarily have a value
			    //|| parameterSet.get(PixelParameters.RUN_KEY) == null ||
			    //	((StringT)parameterSet.get(PixelParameters.RUN_KEY).getValue()).equals("") )  {

				// go to error, we require parameters
				String errMsg = "configureAction: no parameters given with configure command.";
				
				// log error
				logger.error(errMsg);
				
				// notify error
		      		sendCMSError(errMsg);
				
				System.out.println(errMsg); //output to screen as well

				//go to error state
				//JMT - for now we don't really need any information, so don't go to error
				//functionManager.fireEvent( PixelInputs.SETERROR );

			}
			else { //parameterSet is not empty
			    // get the run type from the start command
			    if ( parameterSet.get(PixelParameters.RUN_KEY) == null )  {
				System.out.println("[configureAction] RUN_KEY is null");
			    }
			    else {
				runType = ((StringT)parameterSet.get(PixelParameters.RUN_KEY).getValue()).getString();
			    }
			    if (parameterSet.get(PixelParameters.RUN_NUMBER) == null ) {
				System.out.println("[configureAction] RUN_NUMBER is null");
			    }
			    else {
				runNum = ((IntegerT)parameterSet.get(PixelParameters.RUN_NUMBER).getValue());
			    }
			    if ( parameterSet.get(PixelParameters.TPG_KEY) == null )  {
				System.out.println("[configureAction] TPG_KEY is null");
			    }
			    else {
				tpgKey = ((StringT)parameterSet.get(PixelParameters.TPG_KEY).getValue());
			    }
			    if ( parameterSet.get(PixelParameters.FED_ENABLE_MASK) == null )  {
				System.out.println("[configureAction] FED_ENABLE_MASK is null");
			    }
			    else {
				fedEnableMask = ((StringT)parameterSet.get(PixelParameters.FED_ENABLE_MASK).getValue());
			    }
			}
			System.out.println("[configureAction] RUN_KEY = "+runType);
			System.out.println("[configureAction] RUN_NUMBER = "+runNum);
			System.out.println("[configureAction] TPG_KEY = "+tpgKey);
			System.out.println("[configureAction] FED_ENABLE_MASK = "+fedEnableMask);
			
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.CONFIGURED_WITH_RUN_KEY,new StringT(runType),Exported.READONLY));
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.CONFIGURED_WITH_FED_ENABLE_MASK,new StringT(fedEnableMask),Exported.READONLY));
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.CONFIGURED_WITH_TPG_KEY,new StringT(tpgKey),Exported.READONLY));
			functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.CONFIGURED_WITH_RUN_NUMBER,runNum,Exported.READONLY));
			
			runType="Physics";
			System.out.println(runType); //add additional output
			
			// Set the runType in the Function Manager parameters
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.RUN_KEY,new StringT(runType)));
			
                        try {
                           Map attributeMap = new HashMap();
                           attributeMap.put(PixelParameters.RUN_KEY, runType);
                           XdaqApplication pixelSupervisor=null;
                           List<XdaqApplication> pixelSupervisor_list=functionManager.containerXdaqApplication.getApplicationsOfClass("PixelSupervisor");
                           Iterator i_pixelSupervisor=pixelSupervisor_list.iterator();
                           while (i_pixelSupervisor.hasNext()) {
                              pixelSupervisor=(XdaqApplication)i_pixelSupervisor.next();
                              pixelSupervisor.command(xdaqMsgWithAttributes("Configure", attributeMap ));
			   }
                        } 
			  /*catch (CommandException e) {
				String errMessage="Command exception occured in Configuring.";
				logger.error(errMessage,e);
				e.printStackTrace();
			} */

			catch (XDAQMessageException e) {
				String errMessage="XDAQMessage exception occured in Configuring.";
				logger.error(errMessage,e);
				e.printStackTrace();
			}
			
			// leave intermediate state
			//functionManager.fireEvent( PixelInputs.SETCONFIGURE );
			
			// set action
			//functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));
			
			logger.info("configureAction Executed");
		}
	}
        
        public void configuredAction(Object obj) throws UserActionException {
        
          logger.error("Received Configured from PixelSupervisor!!");
          //if (obj instanceof StateNotification) {

            // triggered by State Notification from child resource
            
            // functionManager.fireEvent( PixelInputs.SETCONFIGURE );

            return;
          //}
        }
	
	public void startAction(Object obj) throws UserActionException, XdaqApplicationContainerException {
		
	    if (obj instanceof StateNotification) {
		//ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();
		
		// triggered by State Notification from child resource
		
		StateNotification sn = (StateNotification) obj;
		String toState = sn.getToState();
		if (toState.equals("Running")) {
		    System.out.println("[startAction] Got state Running from PixelSupervisor");
		    //		    System.out.println("[startAction] Got state Running from PixelSupervisor; will write to RunInfo");
		    //		    System.out.println(parameterSet.get(PixelParameters.PIXEL_GLOBAL_KEY)); //could not make this work

		    //here we try to write to run info (haven't made this work yet)
// 		    RunInfoConnectorIF ric = functionManager.getRunInfoConnector();
// 		    if ( ric == null ) {
// 			System.out.println("[startAction] failed to get RunInfoConnector");
// 		    }
// 		    else {
// 			IntegerT runNum = new IntegerT(-1);
// 			runNum = ((IntegerT) parameterSet.get(PixelParameters.RUN_NUMBER).getValue());
// 			System.out.println("run number = ");
// 			System.out.println(runNum);

// 			RunInfo ri = new RunInfo(ric,  runNum.intValue());
// 			ri.setNameSpace("PIXEL");

// 			IntegerT keyVal =  new IntegerT(-1);
// 			keyVal = ((IntegerT) parameterSet.get(PixelParameters.PIXEL_GLOBAL_KEY).getValue());
// 			System.out.println("GK = ");
// 			System.out.println(keyVal);
// 			Parameter<IntegerT> p = new Parameter<IntegerT>("CONFIGURATION_KEY",keyVal);
// 			try {
// 			    ri.publish(p);
// 			}
// 			catch (RunInfoException e) {
// 			    System.out.println("[startAction] failed to publish to RunInfo");
// 			}
// 		    }
		    //end of writing to run info

		    functionManager.fireEvent( PixelInputs.SETSTART );
		} else {
		    System.out.println("[startAction] Something other than Running came back from PixelSupervisor!");
		    functionManager.fireEvent(PixelInputs.SETERROR);
		}
                
		return;
	    }
	    
		else if (obj instanceof StateEnteredEvent) {
			System.out.println("Executing startAction");
			logger.info("Executing startAction");
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("starting")));

			// get the parameters of the command
			ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();

			// check parameter set
			if (parameterSet.size()==0 || parameterSet.get(PixelParameters.RUN_NUMBER) == null )  {

				// go to error, we require parameters
				String errMsg = "startAction: no parameters given with start command.";
				
				// log error
				logger.error(errMsg);
				
				// notify error
				sendCMSError(errMsg);
				
				// go to error state
				functionManager.fireEvent( PixelInputs.SETERROR );

			}
			
			// get the run number from the start command
			String runNumber = ((IntegerT)parameterSet.get(PixelParameters.RUN_NUMBER).getValue()).getInteger().toString();

			functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.STARTED_WITH_RUN_NUMBER,new IntegerT(runNumber),Exported.READONLY));

			// Set the run number in the Function Manager parameters
			functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.RUN_NUMBER,new IntegerT(runNumber)));
			
			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/

			try {
			   Map attributeMap = new HashMap();
                           attributeMap.put(PixelParameters.RUN_NUMBER, runNumber);
                           XdaqApplication pixelSupervisor=null;
                           List<XdaqApplication> pixelSupervisor_list=functionManager.containerXdaqApplication.getApplicationsOfClass("PixelSupervisor");
                           Iterator i_pixelSupervisor=pixelSupervisor_list.iterator();
                           while (i_pixelSupervisor.hasNext()) {
                              pixelSupervisor=(XdaqApplication)i_pixelSupervisor.next();
                              pixelSupervisor.command(xdaqMsgWithAttributes("Start", attributeMap ));
			   }
                        } 
			  /*catch (CommandException e) {
				String errMessage="Command exception occured in Starting.";
				logger.error(errMessage,e);
				e.printStackTrace();
			} */

			catch (XDAQMessageException e) {
				String errMessage="XDAQMessage exception occured in Starting.";
				logger.error(errMessage,e);
				e.printStackTrace();
			}
			
			// leave intermediate state
			//functionManager.fireEvent( PixelInputs.SETSTART );
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));
			
			logger.debug("startAction Executed");
			
		}
	}
	
	public void pauseAction(Object obj) throws UserActionException, XdaqApplicationContainerException {
		
	    if (obj instanceof StateNotification) {
		
		// triggered by State Notification from child resource
		
		StateNotification sn = (StateNotification) obj;
		String toState = sn.getToState();
		if (toState.equals("Paused")) {
		    System.out.println("[pauseAction] Got state Paused from PixelSupervisor");
		    functionManager.fireEvent( PixelInputs.SETPAUSE );
		} else {
		    System.out.println("[pauseAction] Something other than Paused came back from PixelSupervisor!");
		    functionManager.fireEvent(PixelInputs.SETERROR);
		}
		return;
	    }
		
		else if (obj instanceof StateEnteredEvent) {	
			System.out.println("Executing pauseAction");
			logger.info("Executing pauseAction");
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("pausing")));
			
			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/
                        functionManager.containerXdaqApplication.execute(PixelInputs.PAUSE, "PixelSupervisor");
			
			// leave intermediate state
			//functionManager.fireEvent( PixelInputs.SETPAUSE );
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));
			
			logger.debug("pausingAction Executed");
			
		}
	}
	
	public void stopAction(Object obj) throws UserActionException, XdaqApplicationContainerException {
		
	    if (obj instanceof StateNotification) {
		
		// triggered by State Notification from child resource
		
		StateNotification sn = (StateNotification) obj;
		String toState = sn.getToState();
		if (toState.equals("Configured")) {
		    System.out.println("[stopAction] Got state Configured from PixelSupervisor");
		    functionManager.fireEvent( PixelInputs.SETCONFIGURE );
		} else {
		    System.out.println("[stopAction] Something other than Configured came back from PixelSupervisor!");
		    functionManager.fireEvent(PixelInputs.SETERROR);
		}
		
		return;
	    }
		
		else if (obj instanceof StateEnteredEvent) {	
			System.out.println("Executing stopAction");
			logger.info("Executing stopAction");
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("stopping")));
			
			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/
                        functionManager.containerXdaqApplication.execute(PixelInputs.STOP, "PixelSupervisor");
			
			// leave intermediate state
			//functionManager.fireEvent( PixelInputs.SETCONFIGURE );
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));
			
			logger.debug("stopAction Executed");
			
		}
	}
	public void resumeAction(Object obj) throws UserActionException, XdaqApplicationContainerException {
		
	    if (obj instanceof StateNotification) {
		
		// triggered by State Notification from child resource
		
		StateNotification sn = (StateNotification) obj;
		String toState = sn.getToState();
		if (toState.equals("Running")) {
		    System.out.println("[resumeAction] Got state Running from PixelSupervisor");
		    functionManager.fireEvent( PixelInputs.SETRESUME );
		} else {
		    System.out.println("[resumeAction] Something other than Running came back from PixelSupervisor!");
		    functionManager.fireEvent(PixelInputs.SETERROR);
		}
		
		return;
	    }
		
		else if (obj instanceof StateEnteredEvent) {	
			System.out.println("Executing resumeAction");
			logger.info("Executing resumeAction");
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("resuming")));
			
			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/
                        functionManager.containerXdaqApplication.execute(PixelInputs.RESUME, "PixelSupervisor");
			
			// leave intermediate state
			//functionManager.fireEvent( PixelInputs.SETRESUME );
			
			// Clean-up of the Function Manager parameters
			cleanUpFMParameters();
		
			logger.debug("resumeAction Executed");
			
		}
	}
	
	public void haltAction(Object obj) throws UserActionException, XdaqApplicationContainerException {
		
	    if (obj instanceof StateNotification) {
		
		// triggered by State Notification from child resource
		
		StateNotification sn = (StateNotification) obj;
		String toState = sn.getToState();
		if (toState.equals("Halted")) {
		    System.out.println("[haltAction] Got state Halted from PixelSupervisor");
		    functionManager.fireEvent( PixelInputs.SETHALT );
		} else {
		    System.out.println("[haltAction] Something other than Halted came back from PixelSupervisor!");
		    functionManager.fireEvent(PixelInputs.SETERROR);
		}
                
		return;
	    }
		
		else if (obj instanceof StateEnteredEvent) {
			System.out.println("Executing haltAction");
			logger.info("Executing haltAction");
			
			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("halting")));
			
			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/
                        functionManager.containerXdaqApplication.execute(PixelInputs.HALT, "PixelSupervisor");

			// check from which state we came.
			if (functionManager.getPreviousState().equals(PixelStates.TTSTEST_MODE)) {
				// when we came from TTSTestMode we need to
				// 1. give back control of sTTS to HW
			}
			
			
			// leave intermediate state
			//functionManager.fireEvent( PixelInputs.SETHALT );
			
			// Clean-up of the Function Manager parameters
			cleanUpFMParameters();
			
			logger.debug("haltAction Executed");
		}
	}	
	
	public void preparingTTSTestModeAction(Object obj) throws UserActionException, XdaqApplicationContainerException {

		if (obj instanceof StateNotification) {

			// triggered by State Notification from child resource

			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/

			return;
		}

		else if (obj instanceof StateEnteredEvent) {
			System.out.println("Executing preparingTestModeAction");
			logger.info("Executing preparingTestModeAction");

			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("preparingTestMode")));

			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/
			// to prepare test we need to 
			// 1. configure & enable fed application
			// 2. take control of fed
			
			functionManager.containerXdaqApplication.execute(PixelInputs.TTSTEST_MODE, "PixelSupervisor");

			// leave intermediate state
			functionManager.fireEvent( PixelInputs.SETTTSTEST_MODE );

			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));

			logger.debug("preparingTestModeAction Executed");
		}
	}	
	
	public void testingTTSAction(Object obj) throws UserActionException {


		XdaqApplication fmm = null;
		Map attributeMap = new HashMap();

		if (obj instanceof StateNotification) {

			// triggered by State Notification from child resource

			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/

			return;
		}

		else if (obj instanceof StateEnteredEvent) {
			System.out.println("Executing testingTTSAction");
			logger.info("Executing testingTTSAction");

			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("testing TTS")));

			// get the parameters of the command
			ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();

			// check parameter set
			if (parameterSet.size()==0 || parameterSet.get(PixelParameters.TTS_TEST_FED_ID) == null ||
					parameterSet.get(PixelParameters.TTS_TEST_MODE) == null ||
					((StringT)parameterSet.get(PixelParameters.TTS_TEST_MODE).getValue()).equals("") ||
					parameterSet.get(PixelParameters.TTS_TEST_PATTERN) == null ||
					((StringT)parameterSet.get(PixelParameters.TTS_TEST_PATTERN).getValue()).equals("") ||
					parameterSet.get(PixelParameters.TTS_TEST_SEQUENCE_REPEAT) == null)
					{

				// go to error, we require parameters
				String errMsg = "testingTTSAction: no parameters given with TestTTS command.";
				
				// log error
				logger.error(errMsg);
				
				// notify error
				sendCMSError(errMsg);
				
				//go to error state
				functionManager.fireEvent( PixelInputs.SETERROR );
				
			}
			
			String fedId = ((IntegerT)parameterSet.get(PixelParameters.TTS_TEST_FED_ID).getValue()).getInteger().toString();
			String mode = ((StringT)parameterSet.get(PixelParameters.TTS_TEST_MODE).getValue()).getString();
			String pattern = ((StringT)parameterSet.get(PixelParameters.TTS_TEST_PATTERN).getValue()).getString();
			String cycles = ((IntegerT)parameterSet.get(PixelParameters.TTS_TEST_SEQUENCE_REPEAT).getValue()).getInteger().toString();
			

			// Set last parameters in the Function Manager parameters
			functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.TTS_TEST_FED_ID,new IntegerT(fedId)));
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.TTS_TEST_MODE,new StringT(mode)));
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.TTS_TEST_PATTERN,new StringT(pattern)));
			functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.TTS_TEST_SEQUENCE_REPEAT,new IntegerT(cycles)));

			// debug
			logger.debug("Using parameters: fedId=" + fedId + "mode=" + mode + " pattern=" + pattern + " cycles=" + cycles );

			// find out which application controls the fedId.


			// found the correct application
			// to test we need to 
			// 1. issue the test command


			/************************************************
			 * PUT HERE YOUR CODE							
			 ***********************************************/

			try {
			    Map attributeMapToSOAP = new HashMap();
			    attributeMapToSOAP.put(PixelParameters.TTS_TEST_FED_ID, fedId);
			    attributeMapToSOAP.put(PixelParameters.TTS_TEST_TYPE, mode);
			    attributeMapToSOAP.put(PixelParameters.TTS_TEST_PATTERN, pattern);
			    attributeMapToSOAP.put(PixelParameters.TTS_TEST_SEQUENCE_REPEAT, cycles);
			    XdaqApplication pixelSupervisor=null;
			    List<XdaqApplication> pixelSupervisor_list=functionManager.containerXdaqApplication.getApplicationsOfClass("PixelSupervisor");
			    Iterator i_pixelSupervisor=pixelSupervisor_list.iterator();
			    while (i_pixelSupervisor.hasNext()) {
				pixelSupervisor=(XdaqApplication)i_pixelSupervisor.next();
				pixelSupervisor.command(xdaqMsgWithAttributes("TestTTS", attributeMapToSOAP ));
			    }
                        } 
			/*catch (CommandException e) {
			  String errMessage="Command exception occured in Starting.";
			  logger.error(errMessage,e);
			  e.printStackTrace();
			  } */
			
			catch (XDAQMessageException e) {
			    String errMessage="XDAQMessage exception occured in Starting.";
			    logger.error(errMessage,e);
			    e.printStackTrace();
			}			
			
			// leave intermediate state
			functionManager.fireEvent( PixelInputs.SETTTSTEST_MODE );


			// set action
			functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));

			logger.debug("preparingTestModeAction Executed");
		}
	}	

	@SuppressWarnings("unchecked")
	private void sendCMSError(String errMessage){
		
		// create a new error notification msg
		CMSError error = functionManager.getErrorFactory().getCMSError();
		error.setDateTime(new Date().toString());
		error.setMessage(errMessage);

		// update error msg parameter for GUI
		functionManager.getParameterSet().get(PixelParameters.ERROR_MSG).setValue(new StringT(errMessage));
		
		// send error
		try {
			functionManager.getParentErrorNotifier().sendError(error);
		} catch (Exception e) {
			logger.warn(functionManager.getClass().toString() + ": Failed to send error mesage " + errMessage);
		}
	}
	
	private void cleanUpFMParameters() {
	    ///FIXME: should eventually add all the new parameters (CONFIGURED_WITH_ etc) here
		// Clean-up of the Function Manager parameters
	    //	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.PIXEL_GLOBAL_KEY,new StringT(""))); //jmt
		functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ACTION_MSG,new StringT("")));
		functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.ERROR_MSG,new StringT("")));
		functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.RUN_NUMBER,new IntegerT(-1)));
		functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.RUN_KEY,new StringT("")));
		functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.TTS_TEST_FED_ID,new IntegerT(-1)));
		functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.TTS_TEST_MODE,new StringT("")));
		functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(PixelParameters.TTS_TEST_PATTERN,new StringT("")));
		functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(PixelParameters.TTS_TEST_SEQUENCE_REPEAT,new IntegerT(-1)));
	}

        private XDAQMessage xdaqMsgWithAttributes(String command, Map attributeMap) throws XDAQMessageException {

                XDAQMessage xdaqMsg = new XDAQMessage( command );

                Document document = (Document)xdaqMsg.getDOM();

                Element cmd = (Element)document.getElementsByTagNameNS(XDAQ_NS, command ).item(0);

                Iterator it = attributeMap.keySet().iterator();

                while (it.hasNext()) {
                   String key = (String)it.next();
                   String value = (String)attributeMap.get(key);
                   cmd.setAttribute(key, value);
                }

                xdaqMsg.setDOM(document);

                return xdaqMsg;
        }


}
