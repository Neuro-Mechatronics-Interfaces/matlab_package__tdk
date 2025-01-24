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
    {202013, "No supported driver."}
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
    mexPrintf("  'pulse', <deviceID>, <tactor>, <duration>, <delay>\n");
    mexPrintf("                          Pulse a tactor for the specified duration and delay.\n");
    mexPrintf("  'changeGain', <deviceID>, <tactor>, <gain>, <delay>\n");
    mexPrintf("                          Change the gain of a tactor.\n");
    mexPrintf("  'getName', <index>\n");
    mexPrintf("                          Get the name of the tactor from (0-indexed) discovered device list.\n\n");
    mexPrintf("Alternative: Use uint8 codes for commands to reduce processing overhead:\n");
    mexPrintf("  1 = 'initialize', 2 = 'shutdown', 3 = 'discover', 4 = 'connect'\n");
    mexPrintf("  5 = 'pulse', 6 = 'changeGain, 7 = 'getName'\n\n");
    mexPrintf("Examples:\n");
    mexPrintf("  tdk.tactor('initialize');\n");
    mexPrintf("  tdk.tactor('discover', 1);\n");
    mexPrintf("  tdk.tactor('connect', 'DeviceName', 1);\n");
    mexPrintf("  tdk.tactor('pulse', deviceID, 1, 100, 0);\n");
    mexPrintf("  tdk.tactor('shutdown');\n");
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

    int result = Pulse(deviceID, tacNum, duration, delay);
    handleError(result, "Pulse");
}

void changeGain(int nrhs, const mxArray* prhs[]) {
    if (nrhs < 5) {
        mexErrMsgIdAndTxt("TDK:InputError", "ChangeGain requires deviceID, tactor number, gain value, and delay.");
    }
    int deviceID = static_cast<int>(mxGetScalar(prhs[1]));
    int tacNum = static_cast<int>(mxGetScalar(prhs[2]));
    int gainValue = static_cast<int>(mxGetScalar(prhs[3]));
    int delay = static_cast<int>(mxGetScalar(prhs[4]));

    int result = ChangeGain(deviceID, tacNum, gainValue, delay);
    handleError(result, "ChangeGain");
}

void getName(int nrhs, const mxArray* prhs[], mxArray*& plhs) {
    if (nrhs < 2) mexErrMsgIdAndTxt("TDK:InputError", "getName requires an index.");
    int index = static_cast<int>(mxGetScalar(prhs[1]));
    const char* deviceName = GetDiscoveredDeviceName(index);
    if (!deviceName) mexErrMsgIdAndTxt("TDK:Error", "getName failed with error code: %d", GetLastEAIError());
    plhs = mxCreateString(deviceName); // Return the device name
}

// Dispatch Table for String-based Commands
void dispatchCommand(const char* command, int nrhs, const mxArray* prhs[], mxArray*& plhs) {
    if (strcmp(command, "initialize") == 0) {
        initializeTI();
    }  else if (strcmp(command, "getName") == 0) {
        getName(nrhs, prhs, plhs);
    } else if (strcmp(command, "shutdown") == 0) {
        shutdownTI();
    } else if (strcmp(command, "discover") == 0) {
        discoverDevices(nrhs, prhs, plhs);
    } else if (strcmp(command, "connect") == 0) {
        connectDevice(nrhs, prhs, plhs);
    } else if (strcmp(command, "pulse") == 0) {
        pulseTactor(nrhs, prhs);
    } else if (strcmp(command, "changeGain") == 0) {
        changeGain(nrhs, prhs);
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
            connectDevice(nrhs, prhs, plhs);
            break;
        case 5:
            pulseTactor(nrhs, prhs);
            break;
        case 6:
            changeGain(nrhs, prhs);
            break;
        case 7:
            getName(nrhs, prhs, plhs);
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
