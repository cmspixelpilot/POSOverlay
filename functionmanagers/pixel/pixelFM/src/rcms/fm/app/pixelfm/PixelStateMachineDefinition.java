package rcms.fm.app.pixelfm;


import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.ParameterException;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.parameter.type.IntegerT;
import rcms.fm.fw.parameter.type.StringT;
import rcms.fm.fw.user.UserStateMachineDefinition;
import rcms.statemachine.definition.State;
import rcms.statemachine.definition.StateMachineDefinitionException;

/**
 * This class defines the Finite State Machine for a level 1 Function Manager.
 * 
 * The actual definition of the State Machine must be put in the "init" method.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 */
public class PixelStateMachineDefinition extends UserStateMachineDefinition {

	public PixelStateMachineDefinition() throws StateMachineDefinitionException {
		//
		// Defines the States for this Finite State Machine.
		//

		// steady states
		addState(PixelStates.INITIAL);
		addState(PixelStates.HALTED);
		addState(PixelStates.CONFIGURED);
		addState(PixelStates.RUNNING);
		addState(PixelStates.PAUSED);
		addState(PixelStates.ERROR);
		addState(PixelStates.TTSTEST_MODE);
		addState(PixelStates.RUNNINGSOFTERRORDETECTED ); //SEU
		addState(PixelStates.RUNNINGDEGRADED );
		

		// transitional states
		addState(PixelStates.INITIALIZING);
                addState(PixelStates.COLDRESETTING);
		addState(PixelStates.CONFIGURING);
		addState(PixelStates.HALTING);
		addState(PixelStates.PAUSING);
		addState(PixelStates.RESUMING);
		addState(PixelStates.STARTING);
		addState(PixelStates.STOPPING);
		addState(PixelStates.RECOVERING);
		addState(PixelStates.RESETTING);
		addState(PixelStates.TESTING_TTS);
		addState(PixelStates.PREPARING_TTSTEST_MODE);
		addState(PixelStates.FIXINGSOFTERROR ); // SEU

		//
		// Defines the Initial state.
		//
		setInitialState(PixelStates.INITIAL);

		//
		// Defines the Inputs (Commands) for this Finite State Machine.
		//
		addInput(PixelInputs.INITIALIZE);
                addInput(PixelInputs.COLDRESET);
		addInput(PixelInputs.CONFIGURE);
		addInput(PixelInputs.START);
		addInput(PixelInputs.STOP);
		addInput(PixelInputs.PAUSE);
		addInput(PixelInputs.RESUME);
		addInput(PixelInputs.HALT);
		addInput(PixelInputs.RECOVER);
		addInput(PixelInputs.RESET);
		addInput(PixelInputs.SETERROR);
		addInput(PixelInputs.TTSTEST_MODE);
		addInput(PixelInputs.TEST_TTS);
		addInput(PixelInputs.FIXSOFTERROR ); // SEU
		addInput(PixelInputs.SETFIXEDSOFTERROR ); // SEU
		
		// The SETERROR Input moves the FSM in the ERROR State.
		// This command is not allowed from the GUI.
		// It is instead used inside the FSM callbacks.
		PixelInputs.SETERROR.setVisualizable(false);

		// invisible commands needed for fully asynchronous behaviour
		addInput(PixelInputs.SETCONFIGURE);
		addInput(PixelInputs.SETSTART);
		addInput(PixelInputs.SETPAUSE);
		addInput(PixelInputs.SETRESUME);
		addInput(PixelInputs.SETHALT);
		addInput(PixelInputs.SETINITIAL);
		addInput(PixelInputs.SETRESET);
		addInput(PixelInputs.SETTESTING_TTS);
		addInput(PixelInputs.SETTTSTEST_MODE);
		addInput(PixelInputs.SETSOFTERRORDETECTED ); // SEU
		addInput(PixelInputs.SETRUNNINGDEGRADED ); // for SEU loops
		
		// make these invisible
		PixelInputs.SETCONFIGURE.setVisualizable(false);
		PixelInputs.SETSTART.setVisualizable(false);
		PixelInputs.SETPAUSE.setVisualizable(false);
		PixelInputs.SETRESUME.setVisualizable(false);
		PixelInputs.SETHALT.setVisualizable(false);
		PixelInputs.SETINITIAL.setVisualizable(false);
		PixelInputs.SETRESET.setVisualizable(false);
		PixelInputs.SETTTSTEST_MODE.setVisualizable(false);
		PixelInputs.SETFIXEDSOFTERROR.setVisualizable(false);
//		PixelInputs.SETRUNNINGDEGRADED.setVisualizable(false);
			
		//
		// Define command parameters.
		// These are then visible in the default GUI.
		//
		
		// define parameters for tts testing command
		//
		CommandParameter<IntegerT> ttsTestFedid = new CommandParameter<IntegerT>(PixelParameters.TTS_TEST_FED_ID, new IntegerT(-1));
		CommandParameter<StringT> ttsTestMode = new CommandParameter<StringT>(PixelParameters.TTS_TEST_MODE, new StringT(""));
		CommandParameter<StringT> ttsTestPattern = new CommandParameter<StringT>(PixelParameters.TTS_TEST_PATTERN, new StringT(""));
		CommandParameter<IntegerT> ttsTestSequenceRepeat = new CommandParameter<IntegerT>(PixelParameters.TTS_TEST_SEQUENCE_REPEAT, new IntegerT(-1));
		
		// define parameter set
		ParameterSet<CommandParameter> ttsTestParameters = new ParameterSet<CommandParameter>();
		try {
			ttsTestParameters.add(ttsTestFedid);
			ttsTestParameters.add(ttsTestMode);
			ttsTestParameters.add(ttsTestPattern);
			ttsTestParameters.add(ttsTestSequenceRepeat);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to ttsTestParameters. Duplicate Parameter?", nothing );
		}
		
		// set the test parameters
		PixelInputs.TEST_TTS.setParameters(ttsTestParameters);
	
		//
		// define parameters for Initialize command
		//
		CommandParameter<IntegerT> initializeSid = new CommandParameter<IntegerT>(PixelParameters.SID, new IntegerT(0));
		CommandParameter<StringT> initializeGlobalConfigurationKey = new CommandParameter<StringT>(PixelParameters.GLOBAL_CONF_KEY, new StringT(""));

		// define parameter set
		ParameterSet<CommandParameter> initializeParameters = new ParameterSet<CommandParameter>();
		try {
			initializeParameters.add(initializeSid);
			initializeParameters.add(initializeGlobalConfigurationKey);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to initializeParameters. Duplicate Parameter?", nothing );
		}
		
		PixelInputs.INITIALIZE.setParameters(initializeParameters);
	
		//
		// define parameters for Configure command
		//
		CommandParameter<StringT> configureRunType = new CommandParameter<StringT>(PixelParameters.RUN_KEY, new StringT(""));

		// define parameter set
		ParameterSet<CommandParameter> configureParameters = new ParameterSet<CommandParameter>();
		try {
			configureParameters.add(configureRunType);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to configureParameters. Duplicate Parameter?", nothing );
		}
		
		PixelInputs.CONFIGURE.setParameters(configureParameters);
	
		//
		// define parameters for Start command
		//
		CommandParameter<IntegerT> startRunNumber = new CommandParameter<IntegerT>(PixelParameters.RUN_NUMBER, new IntegerT(-1));

		// define parameter set
		ParameterSet<CommandParameter> startParameters = new ParameterSet<CommandParameter>();
		try {
			startParameters.add(startRunNumber);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to startParameters. Duplicate Parameter?", nothing );
		}
		
		PixelInputs.START.setParameters(startParameters);
	
		
		//
		// Define the State Transitions
		//

		// INIT Command:
		// The INIT input is allowed only in the INITIAL state, and moves the
		// FSM in the INITIALIZING state.
		//
		addTransition(PixelInputs.INITIALIZE, PixelStates.INITIAL, PixelStates.INITIALIZING);
                
                // COLDRESET Command
                addTransition(PixelInputs.COLDRESET, PixelStates.HALTED, PixelStates.COLDRESETTING);

		// TEST_MODE Command:
		// The TEST_MODE input is allowed in the HALTED state and moves 
		// the FSM in the PREPARING_TEST_MODE state.
		//
		addTransition(PixelInputs.TTSTEST_MODE, PixelStates.HALTED, PixelStates.PREPARING_TTSTEST_MODE);

		// Reach the TEST_MODE State
		addTransition(PixelInputs.SETTTSTEST_MODE, PixelStates.PREPARING_TTSTEST_MODE, PixelStates.TTSTEST_MODE);
		addTransition(PixelInputs.SETTTSTEST_MODE, PixelStates.TESTING_TTS, PixelStates.TTSTEST_MODE);

		// TEST_TTS Command:
		// The TEST_TTS input is allowed in the TEST_MODE state and moves
		// the FSM in the TESTING_TTS state.
		addTransition(PixelInputs.TEST_TTS, PixelStates.TTSTEST_MODE, PixelStates.TESTING_TTS);

		// CONFIGURE Command:
		// The CONFIGURE input is allowed only in the HALTED state, and moves
		// the FSM in the CONFIGURING state.
		//
		addTransition(PixelInputs.CONFIGURE, PixelStates.HALTED, PixelStates.CONFIGURING);

		// START Command:
		// The START input is allowed only in the CONFIGURED state, and moves
		// the FSM in the STARTING state.
		//
		addTransition(PixelInputs.START, PixelStates.CONFIGURED, PixelStates.STARTING);

		// PAUSE Command:
		// The PAUSE input is allowed only in the RUNNING state, and moves
		// the FSM in the PAUSING state.
		//
		addTransition(PixelInputs.PAUSE, PixelStates.RUNNING, PixelStates.PAUSING);
		addTransition(PixelInputs.PAUSE, PixelStates.RUNNINGSOFTERRORDETECTED, PixelStates.PAUSING ); // SEU
		addTransition(PixelInputs.PAUSE, PixelStates.RUNNINGDEGRADED, PixelStates.PAUSING );

		// RESUME Command:
		// The RESUME input is allowed only in the PAUSED state, and moves
		// the FSM in the RESUMING state.
		//
		addTransition(PixelInputs.RESUME, PixelStates.PAUSED, PixelStates.RESUMING);
		
		// STOP Command:
		// The STOP input is allowed only in the RUNNING and PAUSED state, and moves
		// the FSM in the CONFIGURED state.
		//
		addTransition(PixelInputs.STOP, PixelStates.RUNNING, PixelStates.STOPPING);
		addTransition(PixelInputs.STOP, PixelStates.PAUSED, PixelStates.STOPPING);
		addTransition(PixelInputs.STOP, PixelStates.RUNNINGSOFTERRORDETECTED, PixelStates.STOPPING ); // SEU
		addTransition(PixelInputs.STOP, PixelStates.RUNNINGDEGRADED, PixelStates.STOPPING );

		// HALT Command:
		// The HALT input is allowed in the RUNNING, CONFIGURED and PAUSED
		// state, and moves the FSM in the HALTING state.
		//
		addTransition(PixelInputs.HALT, PixelStates.RUNNING, PixelStates.HALTING);
		addTransition(PixelInputs.HALT, PixelStates.CONFIGURED, PixelStates.HALTING);
		addTransition(PixelInputs.HALT, PixelStates.PAUSED, PixelStates.HALTING);
		addTransition(PixelInputs.HALT, PixelStates.TTSTEST_MODE, PixelStates.HALTING);
		addTransition(PixelInputs.HALT, PixelStates.RUNNINGSOFTERRORDETECTED, PixelStates.HALTING ); // SEU
		addTransition(PixelInputs.HALT, PixelStates.RUNNINGDEGRADED, PixelStates.HALTING );


		// RECOVER Command:
		// The RECOVER input is allowed from ERROR and moves the FSM in to
		// RECOVERING state.
		//
		addTransition(PixelInputs.RECOVER, PixelStates.ERROR, PixelStates.RECOVERING);

		// RESET Command:
		// The RESET input is allowed from any steady state and moves the FSM in the
		// RESETTING state.
		//
		addTransition(PixelInputs.RESET, PixelStates.HALTED , PixelStates.RESETTING);
		addTransition(PixelInputs.RESET, PixelStates.CONFIGURED , PixelStates.RESETTING);
		addTransition(PixelInputs.RESET, PixelStates.RUNNING , PixelStates.RESETTING);
		addTransition(PixelInputs.RESET, PixelStates.PAUSED , PixelStates.RESETTING);
		addTransition(PixelInputs.RESET, PixelStates.TTSTEST_MODE , PixelStates.RESETTING);
		addTransition(PixelInputs.RESET, PixelStates.ERROR , PixelStates.RESETTING);

		//
		// The following transitions are not triggered from the GUI.
		//

		// Transition for going to ERROR
		addTransition(PixelInputs.SETERROR, State.ANYSTATE, PixelStates.ERROR);

		//
		// add transitions for transitional States
		//

		// Reach the INITIAL State
		//		addTransition(PixelInputs.SETINITIAL, PixelStates.RECOVERING, PixelStates.INITIAL);

		// Reach the HALTED State
		addTransition(PixelInputs.SETHALT, PixelStates.RUNNINGSOFTERRORDETECTED, PixelStates.HALTED ); // SEU
		addTransition(PixelInputs.SETHALT, PixelStates.INITIALIZING, PixelStates.HALTED);
                addTransition(PixelInputs.SETHALT, PixelStates.COLDRESETTING, PixelStates.HALTED);
		addTransition(PixelInputs.SETHALT, PixelStates.HALTING, PixelStates.HALTED);
		addTransition(PixelInputs.SETHALT, PixelStates.RECOVERING, PixelStates.HALTED);
		addTransition(PixelInputs.SETHALT, PixelStates.RESETTING, PixelStates.HALTED);

		// Reach the CONFIGURED State
		addTransition(PixelInputs.SETCONFIGURE, PixelStates.INITIALIZING,
				PixelStates.CONFIGURED);		//seems wrong
		//		addTransition(PixelInputs.SETCONFIGURE, PixelStates.RECOVERING,
		//				PixelStates.CONFIGURED);
		addTransition(PixelInputs.SETCONFIGURE, PixelStates.CONFIGURING,
				PixelStates.CONFIGURED);
		addTransition(PixelInputs.SETCONFIGURE, PixelStates.STOPPING,
				PixelStates.CONFIGURED);
		
		// Reach the RUNNING State
		addTransition(PixelInputs.SETFIXEDSOFTERROR, PixelStates.FIXINGSOFTERROR, PixelStates.RUNNING ); // SEU
		//addTransition(PixelInputs.SETSTART, PixelStates.RUNNINGSOFTERRORDETECTED, PixelStates.RUNNING ); // SEU
		addTransition(PixelInputs.SETSTART, PixelStates.INITIALIZING,
				PixelStates.RUNNING);		//this seems wrong
		addTransition(PixelInputs.SETSTART, PixelStates.RECOVERING, PixelStates.RUNNING);		//this seems wrong
		addTransition(PixelInputs.SETSTART, PixelStates.STARTING, PixelStates.RUNNING);
		addTransition(PixelInputs.SETSTART, PixelStates.RUNNINGDEGRADED, PixelStates.RUNNING );	

		// Reach the PAUSED State
		addTransition(PixelInputs.SETPAUSE, PixelStates.PAUSING, PixelStates.PAUSED);
		//		addTransition(PixelInputs.SETPAUSE, PixelStates.RECOVERING, PixelStates.PAUSED); //seems wrong
		

		// Reach the RUNNING from RESUMING State
		addTransition(PixelInputs.SETRESUME, PixelStates.RESUMING, PixelStates.RUNNING);

		// Reach the SEU detected State
		addTransition(PixelInputs.SETSOFTERRORDETECTED, PixelStates.RUNNING, PixelStates.RUNNINGSOFTERRORDETECTED );
		addTransition(PixelInputs.SETSOFTERRORDETECTED, PixelStates.RUNNINGDEGRADED, PixelStates.RUNNINGSOFTERRORDETECTED );

		// reach Fixing SEU State
		addTransition(PixelInputs.FIXSOFTERROR, PixelStates.RUNNINGSOFTERRORDETECTED, PixelStates.FIXINGSOFTERROR );
		addTransition(PixelInputs.FIXSOFTERROR, PixelStates.RUNNING, PixelStates.FIXINGSOFTERROR );
		addTransition(PixelInputs.FIXSOFTERROR, PixelStates.RUNNINGDEGRADED, PixelStates.FIXINGSOFTERROR );

		// reach running degraded state
		addTransition(PixelInputs.SETRUNNINGDEGRADED, PixelStates.RUNNING, PixelStates.RUNNINGDEGRADED ); 
		addTransition(PixelInputs.SETRUNNINGDEGRADED, PixelStates.FIXINGSOFTERROR, PixelStates.RUNNINGDEGRADED );
		addTransition(PixelInputs.SETRUNNINGDEGRADED, PixelStates.RUNNINGSOFTERRORDETECTED, PixelStates.RUNNINGDEGRADED );
		addTransition(PixelInputs.SETRUNNINGDEGRADED, PixelStates.RESUMING, PixelStates.RUNNINGDEGRADED );

		// note that we also must program in the transitions from soft error to running, etc

	}

}
