/**************************************************************************
 * RCMS Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Author: Souvik Das 		  *
  *************************************************************************/

package rcms.fm.app.pixelfm;

import java.util.List;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserFunctionManager;
import rcms.fm.resource.QualifiedResource;
import rcms.fm.resource.QualifiedResourceContainer;
import rcms.fm.resource.qualifiedresource.JobControl;
import rcms.fm.resource.qualifiedresource.XdaqApplicationContainer;
import rcms.fm.resource.qualifiedresource.XdaqExecutive;
import rcms.statemachine.definition.State;
import rcms.statemachine.definition.StateMachineDefinitionException;
import rcms.util.logger.RCMSLogger;


public class PixelFunctionManager extends UserFunctionManager {

	static RCMSLogger logger = new RCMSLogger(PixelFunctionManager.class);
	public XdaqApplicationContainer containerXdaqApplication = null;
	public XdaqApplicationContainer cEVM = null;
	public QualifiedResourceContainer containerXdaqExecutive = null;
	public QualifiedResourceContainer containerFunctionManager = null;
	public QualifiedResourceContainer containerJobControl = null;
	public State calcState = null;

  public PixelFunctionManager() {
  
		addParameters();
		
	}

	/*
	 * This method is called when the Function Manager is created
	 */
	public void createAction(ParameterSet<CommandParameter> pars) throws UserActionException {

		System.out.println("createAction called.");
		logger.debug("createAction called.");
 
		Calendar cal = Calendar.getInstance();
		Date now = cal.getTime();
		String now_string = now.toGMTString();    
		String args = now_string;
		System.out.println(args);
 
		System.out.println("createAction executed.");
		logger.debug("createAction executed.");

	}

	/*
	 * This method is called when the Function Manager is destroyed
	 */
	public void destroyAction() throws UserActionException {

		System.out.println("destroyAction called");
		logger.debug("destroyAction called");
				
    // Use one Job Control to trigger a script to move the log file to a better place
    String user = "pixelpro";
    String pathToScript= "/nfshome0/pixelpro/TriDAS/pixel/JobControl/moveLogFiles.sh";
    //String args = "";
    Calendar cal = Calendar.getInstance();
    Date now = cal.getTime();
    String now_string = now.toGMTString();    
    String args = now_string;
    System.out.println(args);
    String JID = pathToScript;
    String environment = "";
    List<QualifiedResource> jcList = containerJobControl.getQualifiedResourceList();
    if (jcList.isEmpty()) {
      System.out.println("Did not find any Job Control process.");		
      logger.info("Did not find any Job Control process.");			
    }	else {
      for (Object obj : jcList) {
        JobControl jc = (JobControl) obj;
        jc.killJid(JID); // remove previous jobs
    	jc.start(pathToScript,args,environment,user,JID);
    	System.out.println("Executed moveLogFiles.sh");
      }
    }
    
    for (Object obj : containerXdaqExecutive.getQualifiedResourceList()) {
      XdaqExecutive xdaqExe = (XdaqExecutive) obj;
      xdaqExe.killMe();
    }

		System.out.println("destroyAction executed");
		logger.debug("destroyAction executed");
		
	}

	/**
	 * add parameters to parameterSet. After this they are accessible.
	 */
	private void addParameters() {

		// add parameters to parameter Set so they are visible.
		parameterSet = PixelParameters.LVL_ONE_PARAMETER_SET;

	}

	public void init() throws StateMachineDefinitionException, rcms.fm.fw.EventHandlerException {

		// Set first of all the State Machine Definition
		setStateMachineDefinition(new PixelStateMachineDefinition());

		// Add event handler
		addEventHandler(new PixelEventHandler());

		// Add error handler
		addEventHandler(new PixelErrorHandler());

	}
}
