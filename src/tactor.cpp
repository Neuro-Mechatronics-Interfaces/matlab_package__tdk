#include "mex.h"
#include "TactorInterface.h"
#include "EAI_Defines.h"
#include <string>
#include <map>

// Persistent device state
static std::map<int, int> deviceConnections; // Map of device IDs to their types
static bool atExitRegistered = false;       // Track if mexAtExit has been registered
static bool isConnected = false;
static bool isInitialized = false;

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

static std::map<std::string, uint8_t> stringCommands = {
    {"initialize", 1},
    {"shutdown", 2},
    {"discover", 3},
    {"getName", 4},
    {"connect", 5},
    {"setTimeFactor", 6},
    {"changeGain", 7},
    {"changeFreq", 8},
    {"rampGain", 9},
    {"rampFreq", 10},
    {"pulse", 11},
    {"stop", 12},
    {"setState", 13},
    {"beginStoreTAction", 14},
    {"finishStoreTAction", 15},
    {"playStoredTAction", 16},
    {"checkConnection", 17}
};

// Function to get error description
const char* getErrorDescription(int errorCode) {
    auto it = errorDescriptions.find(errorCode);
    if (it != errorDescriptions.end()) {
        return it->second.c_str();
    }
    return "Unknown error code.";
}

// Function to convert string command to uint8_t
uint8_t stringCommandToCode(const char* command) {
    auto it = stringCommands.find(command);
    if (it != stringCommands.end()) {
        return it->second;
    }
    return 0;
}

// Cleanup function for when MATLAB exits
void cleanup() {
    for (const auto& [deviceID, type] : deviceConnections) {
        Close(deviceID);
    }
    ShutdownTI();
    deviceConnections.clear();
    isConnected = false;
}

// Error handling function with descriptions
void handleError(int result, const char* functionName) {
    if (result < 0) {
        int errorCode = GetLastEAIError();
        const char* description = getErrorDescription(errorCode);
        mexErrMsgIdAndTxt("TDK:Error", "<strong>%s</strong> failed with error code: %d\n\t->\t(%s)", functionName, errorCode, description);
    }
}

void printHelpCodeList() {
    mexPrintf("<strong>uint8 function equivalents</strong>:\n");
    mexPrintf("  1 = 'initialize'\n");
    mexPrintf("  2 = 'shutdown'\n");
    mexPrintf("  3 = 'discover'\n");
    mexPrintf("  4 = 'getName'\n");
    mexPrintf("  5 = 'connect'\n"); 
    mexPrintf("  6 = 'setTimeFactor'\n"); 
    mexPrintf("  7 = 'changeGain'\n");
    mexPrintf("  8 = 'changeFreq'\n"); 
    mexPrintf("  9 = 'rampGain'\n"); 
    mexPrintf("  10 = 'rampFreq'\n");
    mexPrintf("  11 = 'pulse'\n");
    mexPrintf("  12 = 'stop'\n");
    mexPrintf("  13 = 'setState'\n");
    mexPrintf("  14 = 'beginStoreTAction'\n");
    mexPrintf("  15 = 'finishStoreTAction'\n");
    mexPrintf("  16 = 'playStoredTACtion'\n");
    mexPrintf("  17 = 'checkConnection'\n\n");
}

void printHelpCommandDetails(uint8_t command, bool detailed) {
    if (command == 0) {
        for (int i = 1; i <= 17; i++) {
            printHelpCommandDetails(i, false);
        }
        mexPrintf("\n<strong>General</strong>\n");
        mexPrintf("  'h'                    Print default help.\n");
        mexPrintf("  'l'                    Print list of all valid command names.\n");
        mexPrintf("  'h', <command>         Get help for a specific command.\n\n");
        return;
    }
    if (detailed) {
        mexPrintf("Usage: tactor(<command>, <args>...)\n");
    }
    switch (command) {
        case 1:
            mexPrintf("  'initialize'           Initialize the tactor interface.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Note:</strong> First call before any others.\n");
                mexPrintf("                                                      (handled by tdk.open())\n");
                mexPrintf("                                     See also: tdk.open()\n");
            }
            break;
        case 2:
            mexPrintf("  'shutdown'             Shutdown the tactor interface and clean up resources.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Note:</strong> Must be called before exiting MATLAB.\n");
                mexPrintf("                                     See also: tdk.close()\n");
            }
            break;
        case 3:
            mexPrintf("  'discover', <type>\n");
            mexPrintf("                         Discover devices of the specified type (e.g., USB = 1).\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Returns:</strong> number of discovered devices.\n");
                mexPrintf("                          -> Note: must be called before 'connect'.\n");
                mexPrintf("                                            (handled by tdk.open())\n");
                mexPrintf("                                     See also: tdk.open()\n");
            }
            break;
        case 4:
            mexPrintf("  'getName', <index>\n");
            mexPrintf("                         Get the name of the tactor from (0-indexed) discovered device list.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Returns:</strong> device name as a string (e.g. 'COM9').\n");
                mexPrintf("                                            (handled by tdk.open())\n");
                mexPrintf("                                               See also: tdk.open()\n");
            }
            
            break;
        case 5:
            mexPrintf("  'connect', <name>, <type>\n");
            mexPrintf("                         Connect to a device with the given name and type.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                <strong>Returns:</strong> `deviceID` of connected device (integer e.g. 0).\n\n");
                mexPrintf("                        IN: <strong>name</strong> - The name of the port to connect to.\n");
                mexPrintf("                                                    Example: 'COM9'.\n");
                mexPrintf("                        IN: <strong>type</strong> - The enumerated interface type.\n");
                mexPrintf("                                                    Defaults to 1 (WindowsUSB).\n");
                mexPrintf("                                -> please check out tdk.open() <-\n");
            }
            break;
        case 6:
            mexPrintf("  'setTimeFactor', <value>\n");
            mexPrintf("                         Set the time factor for the tactor interface (1 - 255).\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Does not appear to work.</strong>\n");
            }
            break;
        case 7:
            mexPrintf("  'changeGain', <deviceID>, <tactor>, <gain>, <delay>\n");
            mexPrintf("                         Change the gain of a tactor (1-indexed).\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                        IN: <strong>deviceID</strong> - The device ID to apply the command to.\n");
                mexPrintf("                        IN: <strong>tactor</strong> - The tactor number for the command. 1-indexed.\n");
                mexPrintf("                        IN: <strong>gain</strong> - The gain value (1 - 255).\n");
                mexPrintf("                        IN: <strong>delay</strong> - Delay before running command (ms).\n");
                mexPrintf("                                                     Does not seem to do anything.\n");
            }
            break;
        case 8:
            mexPrintf("  'changeFreq', <deviceID>, <tactor>, <freq>, <delay>\n");
            mexPrintf("                         Change the frequency (300 Hz - 3500 Hz) of a tactor.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                        IN: <strong>deviceID</strong> - The device ID to apply the command to.\n");
                mexPrintf("                        IN: <strong>tactor</strong> - The tactor number for the command. 1-indexed.\n");
                mexPrintf("                        IN: <strong>freq</strong> - The new frequency (Hz; 300 - 3500).\n");
                mexPrintf("                        IN: <strong>delay</strong> - Delay before running command (ms).\n");
                mexPrintf("                                                     Does not seem to do anything.\n");
            }
            break;
        case 9:   
            mexPrintf("  'rampGain', <deviceID>, <tactor>, <startGain>, <endGain>, <duration>, <delay>\n");
            mexPrintf("                         Set linear gain ramp over some period of time and delay.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                        IN: <strong>deviceID</strong> - The device ID to apply the command to.\n");
                mexPrintf("                        IN: <strong>tactor</strong> - The tactor number for the command. 1-indexed.\n");
                mexPrintf("                        IN: <strong>duration</strong> - Duration of the command (ms); range is 1-2500.\n");
                mexPrintf("                                                     Does not seem affected by `setTimeFactor` scalar.\n");
                mexPrintf("                        IN: <strong>delay</strong> - Delay before running command (ms).\n");
                mexPrintf("                                                     Does not seem to do anything.\n");
            }
            break;          
        case 10:
            mexPrintf("  'rampFreq', <deviceID>, <tactor>, <startFreq>, <endFreq>, <duration>, <delay>\n");
            mexPrintf("                         Set linear frequency ramp over some period of time and delay.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                        IN: <strong>deviceID</strong> - The device ID to apply the command to.\n");
                mexPrintf("                        IN: <strong>tactor</strong> - The tactor number for the command. 1-indexed.\n");
                mexPrintf("                        IN: <strong>startFreq</strong> - The ramp starting (Hz; 300 - 3500).\n");
                mexPrintf("                        IN: <strong>endFreq</strong> - The ramp ending frequency (Hz; 300 - 3500).\n");
                mexPrintf("                        IN: <strong>duration</strong> - Duration of the command (ms); range is 1-2500.\n");
                mexPrintf("                                                     Does not seem affected by `setTimeFactor` scalar.\n");
                mexPrintf("                        IN: <strong>delay</strong> - Delay before running command (ms).\n");
                mexPrintf("                                                     Does not seem to do anything.\n");
            }
            break;
        case 11:
            mexPrintf("  'pulse', <deviceID>, <tactor>, <duration>, <delay>\n");
            mexPrintf("                         Pulse a tactor (1-indexed) for the specified duration and delay.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                        IN: <strong>deviceID</strong> - The device ID to apply the command to.\n");
                mexPrintf("                        IN: <strong>tactor</strong> - The tactor number for the command. 1-indexed.\n");
                mexPrintf("                        IN: <strong>duration</strong> - Duration of the command (ms); range is 1-2500.\n");
                mexPrintf("                                                     Does not seem affected by `setTimeFactor` scalar.\n");
                mexPrintf("                        IN: <strong>delay</strong> - Delay before running command (ms).\n");
                mexPrintf("                                                     Does not seem to do anything.\n");
            }
            break;
        case 12:
            mexPrintf("  'stop', <deviceID>, <delay>\n");
            mexPrintf("                         Stops all tactors after the specified delay duration.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                        IN: <strong>deviceID</strong> - The device ID to apply the command to.\n");
                mexPrintf("                        IN: <strong>delay</strong> - Delay before running command (ms).\n");
                mexPrintf("                                                     Does not seem to do anything.\n");
            }
            break;
        case 13:
            mexPrintf("  'setState', <deviceID>, <delay>, <states>\n");
            mexPrintf("                         Set the state of all tactors after the specified delay duration.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Do not use! Seems to break MATLAB interface!</strong>\n");
            }
            break;
        case 14:
            mexPrintf("  'beginStoreTAction', <deviceID>, <tacID>\n");
            mexPrintf("                         Store a TAction with specified tacID (1 - 10).\n");
            mexPrintf("                         Should always be called with finishStoreTAction.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Does not appear to work.</strong>\n");
            }
            break;
        case 15:
            mexPrintf("  'finishStoreTAction', <deviceID>\n");
            mexPrintf("                         Stop storing the current TAction.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Does not appear to work.</strong>\n");
            }
            break;
        case 16:
            mexPrintf("  'playStoredTAction', <deviceID>, <delay>, <tacID>\n");
            mexPrintf("                         Play the specified TAction after some delay.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Does not appear to work.</strong>\n");
            }
            break;
        case 17:
            mexPrintf("  'checkConnection'\n");
            mexPrintf("                         Check if the tactor interface is connected to a device.\n");
            if (detailed) {
                mexPrintf("\n");
                mexPrintf("                         <strong>Returns:</strong> logical scalar indicating connection status.\n");
            }
            break;
        default:
            mexErrMsgIdAndTxt("TDK:UnknownCommand", "Unknown command code: %d", command);
    }
    if (detailed) {
        mexPrintf("\n");
    }
}

void printHelpExamples() {
    mexPrintf("<strong>Examples</strong>\n");
    mexPrintf("  tactor('initialize');\n");
    mexPrintf("  tactor('discover', 1);\n");
    mexPrintf("  tactor('connect', 'DeviceName', 1);\n");
    mexPrintf("  tactor('pulse', deviceID, 1, 100, 0);\n");
    mexPrintf("  tactor('shutdown');\n\n");
}

void printHelpNotes() {
    mexPrintf("<strong>Note 1</strong>: It is probably easiest to use the tdk package functions rather than tactor directly.\n");
    mexPrintf("<strong>Note 2</strong>: As of 2025-01-24, Max has not figured out how to make TAction or the ramp functions work.\n\n");
}

// Help function
void printHelp() {
    mexPrintf("<strong>NML-TDK Vibrotactor MEX Interface</strong>\n");
    mexPrintf("-----------------------------------\n");
    mexPrintf("Usage: tactor(<command>, <args>...)\n\n");
    mexPrintf("<strong>Commands</strong>\n");
    printHelpCommandDetails(0, false);
    printHelpCodeList();
    printHelpExamples();
    printHelpNotes();
}

// Individual command functions
void initializeTI() {
    if (isInitialized) return;
    int result = InitializeTI();
    handleError(result, "InitializeTI");
    if (result == 0) {
        isInitialized = true;
    }
}

void shutdownTI() {
    cleanup();
    isInitialized = false;
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
    if (isConnected) {
        mexErrMsgIdAndTxt("TDK:ConnectionError", "Already connected to a device. Close the current connection first.");
    }
    char deviceName[64];
    mxGetString(prhs[1], deviceName, sizeof(deviceName));
    int type = static_cast<int>(mxGetScalar(prhs[2]));
    int deviceID = Connect(deviceName, type, nullptr);
    handleError(deviceID, "Connect");
    deviceConnections[deviceID] = type;
    plhs = mxCreateDoubleScalar(deviceID);
    isConnected = true;
}

void checkConnection(mxArray*& plhs) {
    plhs = mxCreateLogicalScalar(isConnected);
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

void setState(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 3) {
        mexErrMsgIdAndTxt("TDK:InputError", "SetState requires deviceID and states (64-bit mask of ON/OFF with tactor1 == LSB).");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    unsigned char* states = (unsigned char*)mxGetData(prhs[2]);
    int result = SetTactors(deviceID, 0, states);
    handleError(result, "SetTactors");
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

    // int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    // handleError(internalUpdateResult, "UpdateTI");

    int result = RampFreq(deviceID, tacNum, startFreq, endFreq, duration, 0x01, delay);
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

    // int internalUpdateResult = UpdateTI(); // Update the Tactor Interface
    // handleError(internalUpdateResult, "UpdateTI");

    int result = RampGain(deviceID, tacNum, gainStart, gainEnd, duration, 0x01, delay);
    handleError(result, "RampGain");
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
    if (nrhs < 2) {
        mexErrMsgIdAndTxt("TDK:InputError", "Stop requires deviceID.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));

    int result = Stop(deviceID, 0);
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
    } else if (strcmp(command, "setState") == 0) {
        setState(nrhs, prhs);
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
    } else if (strcmp(command, "checkConnection") == 0) {
        checkConnection(plhs);
    } else if ((strcmp(command, "help") == 0) || (strcmp(command,"-help") == 0)) {
        printHelp();
    } else if ((strcmp(command, "h") == 0) || (strcmp(command,"-h") == 0)) {
        if (nrhs > 1) {
            char detail[64];
            mxGetString(prhs[1], detail, sizeof(detail));
            uint8_t cmd = stringCommandToCode(detail);
            printHelpCommandDetails(cmd, true);
            if (command == 0) mexErrMsgIdAndTxt("TDK:UnknownCommand", "Unknown command: %s", detail);
        } else {
            printHelp();
        }
    } else if ((strcmp(command, "list") == 0) || (strcmp(command,"-list") == 0) || (strcmp(command,"-l") == 0) || (strcmp(command,"l") == 0)) {
        printHelpCodeList();
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
            setState(nrhs, prhs);
            break;
        case 14:
            beginStoreTAction(nrhs, prhs);
            break;
        case 15:
            finishStoreTAction(nrhs, prhs); 
            break;
        case 16:
            playStoredTAction(nrhs, prhs); 
            break;
        case 17:
            checkConnection(plhs);
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
    if (mxIsNumeric(prhs[0]) && mxGetClassID(prhs[0]) == mxUINT8_CLASS) {
        // Integer-based dispatch
        uint8_t command = static_cast<uint8_t>(mxGetScalar(prhs[0]));
        dispatchCommand(command, nrhs, prhs, plhs[0]);
    } else if (mxIsChar(prhs[0])) {
        // String-based dispatch
        char command[64];
        mxGetString(prhs[0], command, sizeof(command));
        dispatchCommand(command, nrhs, prhs, plhs[0]);
    } else {
        mexErrMsgIdAndTxt("TDK:InputError", "First argument must be a command string or uint8.");
    }
}
