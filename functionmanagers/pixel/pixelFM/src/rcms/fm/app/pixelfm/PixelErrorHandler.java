package rcms.fm.app.pixelfm;

import rcms.fm.fw.EventHandlerException;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserErrorHandler;
import rcms.statemachine.definition.State;
import rcms.util.logger.RCMSLogger;

/**
 * 
 * Error Event Handler class for Level 1 Function Manager.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 *
 */
public class PixelErrorHandler extends UserErrorHandler {
	
	/**
	 * <code>RCMSLogger</code>: RCMS log4j logger.
	 */
	static RCMSLogger logger = new RCMSLogger(PixelEventHandler.class);

	public PixelErrorHandler() throws EventHandlerException {
		// this handler inherits UserErrorHandler
		// so it is already registered for Error events
			
		// error handler
		addAction(State.ANYSTATE,"errorHandler");
		
	}
	
	public void init() throws rcms.fm.fw.EventHandlerException {

	}
	
	
	public void errorHandler(Object obj) throws UserActionException {
	
		System.out.println("errorHandler() Got an event: " + obj.getClass() );
		
	}
	
	
}
