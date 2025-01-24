#include "mex.h"
#include "TactorInterface.h"
#include "EAI_Defines.h"
#include <string>
#include <map>

// Persistent device state
static std::map<int, int> deviceConnections; // Map of device IDs to their types
static bool atExitRegistered = false;       // Track if mexAtExit has been registered
// Error code lookup table
static std::map<int, std::string> errorDescriptions = {
    {202000, "No initialization."},
    {202001, "Connection error."},
    {202002, "Bad parameter."},
    {202003, "Internal error."},
    {202004, "Partial read."},
    {202005, "Null handle."},
    {202006, "Windows error."},
    {202007, "Timeout error."},
    {202008, "No read."},
    {202009, "Failed to close."},
    {202010, "More to read."},
    {202011, "Failed to read."},
    {202012, "Failed to write."},
    {202013, "No supported driver."},
    {203000, "Parameter value out of bounds."},
    {204010, "Device Manager action limit reached."},
    {204011, "Failed to generate device ID."},
    {205000, "Unknown JNI error."},
    {205001, "Bad JNI call."},
    {205002, "Find class error."},
    {205003, "Find field error."},
    {205004, "Find method error."},
    {205005, "Call method error."},
    {205006, "Resource acquisition error."},
    {205007, "Resource release error."},
    {302000, "SI error."},
    {402000, "TM not initialized."},
    {402001, "No device."},
    {402002, "Can't map."},
    {402003, "Failed to open."}, 
    {402004, "Invalid parameter."}, 
    {402005, "Missing connected segment."}, 
    {402006, "Bad parameter."}, 
    {402007, "TAction ID doesn't exist."}, 
    {402008, "Database not initialized."}, 
    {402009, "Max controller limit reached."}, 
    {402010, "Max action limit reached."}, 
    {402011, "Controller not found."}, 
    {402012, "Max tactor location limit reached."}, 
    {402013, "TAction not found."}, 
    {402014, "Failed to unload."}, 
    {402015, "No TActions in database."}, 
    {402016, "Failed to open database."}, 
    {402017, "Failed packet parse."}, 
    {402018, "Failed to clone TAction."},
    {502000, "DBM error."},
    {502001, "DBM No error."},
    {602000, "Bad data."}
};

// Function to get error description
const char* getErrorDescription(int errorCode) {
    auto it = errorDescriptions.find(errorCode);
    if (it != errorDescriptions.end()) {
        return it->second.c_str();
    }
    return "Unknown error code.";
}

// Cleanup function for when MATLAB exits
void cleanup() {
    for (const auto& [deviceID, type] : deviceConnections) {
        Close(deviceID);
    }
    ShutdownTI();
    deviceConnections.clear();
}

// Error handling function with descriptions
void handleError(int result, const char* functionName) {
    if (result < 0) {
        int errorCode = GetLastEAIError();
        const char* description = getErrorDescription(errorCode);
        mexErrMsgIdAndTxt("TDK:Error", "%s failed with error code: %d (%s)", functionName, errorCode, description);
    }
}

// Help function
void printHelp() {
    mexPrintf("TDK Vibrotactor MEX Interface Help:\n");
    mexPrintf("----------------------------------\n");
    mexPrintf("Usage: tdk.tactor(<command>, <args>...)\n\n");
    mexPrintf("Commands:\n");
    mexPrintf("  'initialize'            Initialize the tactor interface.\n");
    mexPrintf("  'shutdown'              Shutdown the tactor interface and clean up resources.\n");
    mexPrintf("  'discover', <type>      Discover devices of the specified type (e.g., USB = 1).\n");
    mexPrintf("  'connect', <name>, <type>\n");
    mexPrintf("                          Connect to a device with the given name and type.\n");
    mexPrintf("  'setTimeFactor', <value>\n");
    mexPrintf("                          Set the time factor for the tactor interface (1 - 255).\n");
    mexPrintf("  'changeGain', <deviceID>, <tactor>, <gain>, <delay>\n");
    mexPrintf("                          Change the gain of a tactor (1-indexed).\n");
    mexPrintf("  'changeFreq', <deviceID>, <tactor>, <freq>, <delay>\n");
    mexPrintf("                          Change the frequency (300 - 3550) of a tactor.\n");
    mexPrintf("  'getName', <index>\n");
    mexPrintf("                          Get the name of the tactor from (0-indexed) discovered device list.\n");
    mexPrintf("  'rampGain', <deviceID>, <tactor>, <startGain>, <endGain>, <duration>, <delay>\n");
    mexPrintf("                          Set linear gain ramp over some period of time and delay.\n");
    mexPrintf("  'rampFreq', <deviceID>, <tactor>, <startFreq>, <endFreq>, <duration>, <delay>\n");
    mexPrintf("                          Set linear frequency ramp over some period of time and delay.\n");
    mexPrintf("  'pulse', <deviceID>, <tactor>, <duration>, <delay>\n");
    mexPrintf("                          Pulse a tactor (1-indexed) for the specified duration and delay.\n");
    mexPrintf("  'stop', <deviceID>, <delay>\n");
    mexPrintf("                          Stops all tactors after the specified delay duration.\n");
    mexPrintf("  'beginStoreTAction', <deviceID>, <tacID>\n");
    mexPrintf("                          Store a TAction with specified tacID (1 - 10). Should always be called with finishStoreTAction.\n");
    mexPrintf("  'finishStoreTAction', <deviceID>\n");
    mexPrintf("                          Stop storing the current TAction.\n");
    mexPrintf("  'playStoredTAction', <deviceID>, <delay>, <tacID>\n");
    mexPrintf("                          Play the specified TAction after some delay.\n");
    mexPrintf("Alternative: Use uint8 codes for commands to reduce processing overhead:\n");
    mexPrintf("  1 = 'initialize', 2 = 'shutdown', 3 = 'discover'\n");
    mexPrintf("  4 = 'getName', 5 = 'connect'\n"); 
    mexPrintf("  6 = 'setTimeFactor', 7 = 'changeGain', 8 = 'changeFreq'\n"); 
    mexPrintf("  9 = 'rampGain', 10 = 'rampFreq'\n");
    mexPrintf("  11 = 'pulse', 12 = 'stop'\n");
    mexPrintf("  13 = 'beginStoreTAction', 14 = 'finishStoreTAction', 15 = 'playStoredTACtion'\n\n");
    mexPrintf("Examples:\n");
    mexPrintf("  tactor('initialize');\n");
    mexPrintf("  tactor('discover', 1);\n");
    mexPrintf("  tactor('connect', 'DeviceName', 1);\n");
    mexPrintf("  tactor('pulse', deviceID, 1, 100, 0);\n");
    mexPrintf("  tactor('shutdown');\n\n");
    mexPrintf("Note 1: It is probably easiest to use the tdk package functions rather than tactor directly.\n");
    mexPrintf("Note 2: As of 2025-01-24, Max has not figured out how to make TAction or the ramp functions work.\n\n");
}

// Individual command functions
void initializeTI() {
    int result = InitializeTI();
    handleError(result, "InitializeTI");
}

void shutdownTI() {
    cleanup();
}

void discoverDevices(int nrhs, const mxArray* prhs[], mxArray*& plhs) {
    if (nrhs < 2 || !mxIsNumeric(prhs[1])) {
        mexErrMsgIdAndTxt("TDK:InputError", "Discover requires a device type as an argument.");
    }
    int type = static_cast<int>(mxGetScalar(prhs[1]));
    int result = Discover(type);
    handleError(result, "Discover");
    plhs = mxCreateDoubleScalar(result);
}

void connectDevice(int nrhs, const mxArray* prhs[], mxArray*& plhs) {
    if (nrhs < 3 || !mxIsChar(prhs[1]) || !mxIsNumeric(prhs[2])) {
        mexErrMsgIdAndTxt("TDK:InputError", "Connect requires a device name (string) and type (integer).");
    }
    char deviceName[64];
    mxGetString(prhs[1], deviceName, sizeof(deviceName));
    int type = static_cast<int>(mxGetScalar(prhs[2]));
    int deviceID = Connect(deviceName, type, nullptr);
    handleError(deviceID, "Connect");

    deviceConnections[deviceID] = type;
    plhs = mxCreateDoubleScalar(deviceID);
}

void pulseTactor(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 5) {
        mexErrMsgIdAndTxt("TDK:InputError", "Pulse requires deviceID, tactor number, duration, and delay.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int tacNum = static_cast<int>(mxGetScalar(prhs[2]));
    int duration = static_cast<int>(mxGetScalar(prhs[3]));
    int delay = static_cast<int>(mxGetScalar(prhs[4]));

    int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    handleError(internalUpdateResult, "UpdateTI");

    int actualPulseFunctionResult = Pulse(deviceID, tacNum, duration, delay);
    handleError(actualPulseFunctionResult, "Pulse");
}

void changeGain(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 5) {
        mexErrMsgIdAndTxt("TDK:InputError", "ChangeGain requires deviceID, tactor number, gain value, and delay.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int tacNum = static_cast<int>(mxGetScalar(prhs[2]));
    int gainValue = static_cast<int>(mxGetScalar(prhs[3]));
    int delay = static_cast<int>(mxGetScalar(prhs[4]));

    int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    handleError(internalUpdateResult, "UpdateTI");

    int result = ChangeGain(deviceID, tacNum, gainValue, delay);
    handleError(result, "ChangeGain");
}

void changeFreq(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 5) {
        mexErrMsgIdAndTxt("TDK:InputError", "ChangeFreq requires deviceID, tactor number, freq value (300 - 3550), and delay.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int tacNum = static_cast<int>(mxGetScalar(prhs[2]));
    int freqValue = static_cast<int>(mxGetScalar(prhs[3]));
    int delay = static_cast<int>(mxGetScalar(prhs[4]));

    int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    handleError(internalUpdateResult, "UpdateTI");

    int result = ChangeFreq(deviceID, tacNum, freqValue, delay);
    handleError(result, "ChangeFreq");
}

void getName(int nrhs, const mxArray* prhs[], mxArray*& plhs) {
    if (nrhs < 2) mexErrMsgIdAndTxt("TDK:InputError", "getName requires an index.");
    int index = static_cast<int>(mxGetScalar(prhs[1]));
    const char* deviceName = GetDiscoveredDeviceName(index);
    if (!deviceName) handleError(GetLastEAIError(), "getName");
    plhs = mxCreateString(deviceName); // Return the device name
}

void rampFreq(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 7) {
        mexErrMsgIdAndTxt("TDK:InputError", "ChangeGain requires deviceID, tactor number, start frequency (300 - 3550), end frequency (300 - 3550), ramp duration, and delay.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int tacNum = static_cast<int>(mxGetScalar(prhs[2]));
    int startFreq = static_cast<int>(mxGetScalar(prhs[3]));
    int endFreq = static_cast<int>(mxGetScalar(prhs[4]));
    int duration = static_cast<int>(mxGetScalar(prhs[5]));
    int delay = static_cast<int>(mxGetScalar(prhs[6]));

    int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    handleError(internalUpdateResult, "UpdateTI");

    int result = RampFreq(deviceID, tacNum, startFreq, endFreq, duration, TDK_LINEAR_RAMP, delay);
    handleError(result, "RampFreq");
}

void rampGain(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 7) {
        mexErrMsgIdAndTxt("TDK:InputError", "ChangeGain requires deviceID, tactor number, start gain (0 - 255), end gain (0 - 255), ramp duration, and delay.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int tacNum = static_cast<int>(mxGetScalar(prhs[2]));
    int gainStart = static_cast<int>(mxGetScalar(prhs[3]));
    int gainEnd = static_cast<int>(mxGetScalar(prhs[4]));
    int duration = static_cast<int>(mxGetScalar(prhs[5]));
    int delay = static_cast<int>(mxGetScalar(prhs[6]));

    int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    handleError(internalUpdateResult, "UpdateTI");

    int result = RampFreq(deviceID, tacNum, gainStart, gainEnd, duration, TDK_LINEAR_RAMP, delay);
    handleError(result, "RampFreq");
}

void setTimeFactor(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 2) {
        mexErrMsgIdAndTxt("TDK:InputError", "SetTimeFactor requires a byte value (1 - 255).");
    }
    int value = static_cast<int>(mxGetScalar(prhs[1]));

    int result = SetTimeFactor(value);
    handleError(result, "SetTimeFactor");
}

void stopTactor(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 3) {
        mexErrMsgIdAndTxt("TDK:InputError", "Pulse requires deviceID, and delay.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int delay = static_cast<int>(mxGetScalar(prhs[2]));

    int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    handleError(internalUpdateResult, "UpdateTI");

    int result = Stop(deviceID, delay);
    handleError(result, "Stop");
}

void beginStoreTAction(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 3) {
        mexErrMsgIdAndTxt("TDK:InputError", "BeginStoreTACtion requires deviceID and TActionID.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int tacID = static_cast<int>(mxGetScalar(prhs[2]));
    int result = BeginStoreTAction(deviceID, tacID); 
    handleError(result, "BeginStoreTAction");
}

void finishStoreTAction(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 2) {
        mexErrMsgIdAndTxt("TDK:InputError", "FinishStoreTACtion requires deviceID.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int result = FinishStoreTAction(deviceID); 
    handleError(result, "FinishStoreTAction");
}

void playStoredTAction(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 4) {
        mexErrMsgIdAndTxt("TDK:InputError", "BeginStoreTACtion requires deviceID and TActionID.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int delay = static_cast<int>(mxGetScalar(prhs[2])); 
    int tacID = static_cast<int>(mxGetScalar(prhs[3]));
    int result = PlayStoredTAction(deviceID, delay, tacID); 
    handleError(result, "PlayStoredTAction");
}

// Dispatch Table for String-based Commands
void dispatchCommand(const char* command, int nrhs, const mxArray* prhs[], mxArray*& plhs) {
    if (strcmp(command, "initialize") == 0) {
        initializeTI();
    }  else if (strcmp(command, "pulse") == 0) {
        pulseTactor(nrhs, prhs);
    } else if (strcmp(command, "getName") == 0) {
        getName(nrhs, prhs, plhs);
    } else if (strcmp(command, "shutdown") == 0) {
        shutdownTI();
    } else if (strcmp(command, "discover") == 0) {
        discoverDevices(nrhs, prhs, plhs);
    } else if (strcmp(command, "connect") == 0) {
        connectDevice(nrhs, prhs, plhs);
    } else if (strcmp(command, "setTimeFactor") == 0) {
        setTimeFactor(nrhs, prhs);
    } else if (strcmp(command, "changeGain") == 0) {
        changeGain(nrhs, prhs);
    } else if (strcmp(command, "changeFreq") == 0) {
        changeFreq(nrhs, prhs);
    } else if (strcmp(command, "stop") == 0) {
        stopTactor(nrhs, prhs);
    } else if (strcmp(command, "rampFreq") == 0) {
        rampFreq(nrhs, prhs);
    } else if (strcmp(command, "rampGain") == 0) {
        rampGain(nrhs, prhs);
    } else if (strcmp(command, "beginStoreTAction") == 0) {
        beginStoreTAction(nrhs, prhs);
    } else if (strcmp(command, "finishStoreTAction") == 0) {
        finishStoreTAction(nrhs, prhs);
    } else if (strcmp(command, "playStoredTAction") == 0) {
        playStoredTAction(nrhs, prhs);  
    } else if ((strcmp(command, "help") == 0) || (strcmp(command, "h") == 0)) {
        printHelp();
    } else {
        printHelp();
        mexErrMsgIdAndTxt("TDK:UnknownCommand", "Unknown command: %s", command);
    }
}

// Dispatch Table for Integer-based Commands
void dispatchCommand(uint8_t command, int nrhs, const mxArray* prhs[], mxArray*& plhs) {
    switch (command) {
        case 1:
            initializeTI();
            break;
        case 2:
            shutdownTI();
            break;
        case 3:
            discoverDevices(nrhs, prhs, plhs);
            break;
        case 4:
            getName(nrhs, prhs, plhs);
            break;
        case 5:
            connectDevice(nrhs, prhs, plhs);
            break;
        case 6:
            setTimeFactor(nrhs, prhs);
            break;
        case 7:
            changeGain(nrhs, prhs);
            break;
        case 8:
            changeFreq(nrhs, prhs);
            break;
        case 9:
            rampGain(nrhs, prhs);
            break;
        case 10:
            rampFreq(nrhs, prhs);
            break;
        case 11:
            pulseTactor(nrhs, prhs);
            break;
        case 12:
            stopTactor(nrhs, prhs);
            break;
        case 13:
            beginStoreTAction(nrhs, prhs);
            break;
        case 14:
            finishStoreTAction(nrhs, prhs); 
            break;
        case 15:
            playStoredTAction(nrhs, prhs); 
            break;
        default:
            printHelp();
            mexErrMsgIdAndTxt("TDK:UnknownCommand", "Unknown command code: %d", command);
    }
}

// MEX entry point
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
    // If no arguments are provided, print help
    if (nrhs == 0) {
        printHelp();
        return;
    }

    // Register cleanup function once
    if (!atExitRegistered) {
        mexAtExit(cleanup);
        atExitRegistered = true;
    }

    // Dispatch based on first input type
    if (mxIsChar(prhs[0])) {
        // String-based dispatch
        char command[64];
        mxGetString(prhs[0], command, sizeof(command));
        dispatchCommand(command, nrhs, prhs, plhs[0]);
    } else if (mxIsNumeric(prhs[0]) && mxGetClassID(prhs[0]) == mxUINT8_CLASS) {
        // Integer-based dispatch
        uint8_t command = static_cast<uint8_t>(mxGetScalar(prhs[0]));
        dispatchCommand(command, nrhs, prhs, plhs[0]);
    } else {
        mexErrMsgIdAndTxt("TDK:InputError", "First argument must be a command string or uint8.");
    }
}
