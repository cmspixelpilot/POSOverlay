package rcms.fm.app.pixelfm;

import rcms.statemachine.definition.Input;

/**
 * Definition of Level 1 Function Manager Commands
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 */
public class PixelInputs {

	// Defined commands for the level 1 Function Manager

	public static final Input INITIALIZE = new Input("Initialize");
        
        public static final Input COLDRESET = new Input("ColdReset");

	public static final Input CONFIGURE = new Input("Configure");

	public static final Input SETCONFIGURE = new Input("SetConfigured");

	public static final Input START = new Input("Start");

	public static final Input SETSTART = new Input("SetRunning");
	
	public static final Input STOP = new Input("Stop");

	public static final Input HALT = new Input("Halt");

	public static final Input SETHALT = new Input("SetHalted");

	public static final Input PAUSE = new Input("Pause");

	public static final Input SETPAUSE = new Input("SetPaused");

	public static final Input RESUME = new Input("Resume");

	public static final Input SETRESUME = new Input("SetResume");

	public static final Input RECOVER = new Input("Recover");

	public static final Input SETINITIAL = new Input("SetInitial");

	public static final Input RESET = new Input("Reset");

	public static final Input SETRESET = new Input("SetReset");

	public static final Input TTSTEST_MODE = new Input("TTSTestMode");

	public static final Input SETTTSTEST_MODE = new Input("SetTTSTestMode");

	public static final Input TEST_TTS = new Input("TestTTS");

	public static final Input SETTESTING_TTS = new Input("SetTestingTTS");

	// Adding SEU inputs
	public static final Input SETRUNNING = new Input("Running" );

	public static final Input FIXSOFTERROR = new Input( "FixSoftError" );

	public static final Input SETSOFTERRORDETECTED = new Input( "SetSoftErrorDetected" );

	public static final Input SETFIXEDSOFTERROR = new Input( "SetFixedSoftError" );

	public static final Input SETRUNNINGDEGRADED = new Input( "SetRunningDegraded" );

	public static final Input RUNNINGDEGRADED = new Input( "RunningDegraded" );

	// Go to error
	public static final Input SETERROR = new Input("SetError");

}

