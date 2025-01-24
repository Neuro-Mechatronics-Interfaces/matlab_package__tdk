%EXAMPLE Example illustrating installation and usage of mex tactor interface. Run using `tdk.example;` from Command Window. 

% Run the install script to compile the MEX file
tdk.install();  

% Or, to force recompile:
% tdk.install(true);

% Run the setup script to make sure the dll folder is on workspace path
tdk.setup();

% Initialize the library
tactor('initialize');

% Discover USB devices
numDevices = tactor('discover', 1); % 1: Specifies Windows USB

if numDevices < 1
    error('No devices found! Please ensure the device is connected.');
else
    fprintf(1,'Discovered devices: %d\n', numDevices);
end

% Obtain the device name programmatically
deviceName = char(tactor('getName', 0));
fprintf(1, 'Using device: %s\n', deviceName);

% Connect to the first device
deviceID = tactor('connect', deviceName, 1);

% Pulse a tactor using character-based command
fprintf(1, 'Pulsing tactor using character-based command...\n');
tactor('pulse', deviceID, 1, 100, 0);

% Change gain using character-based command
fprintf(1, 'Changing gain using character-based command...\n');
tactor('changeGain', deviceID, 1, 200, 0);

% Pulse a tactor using uint8-based command (5 = 'pulse')
fprintf(1, 'Pulsing tactor using uint8-based command...\n');
tactor(uint8(5), deviceID, 1, 100, 0);

% Change gain using uint8-based command (6 = 'changeGain')
fprintf(1, 'Changing gain using uint8-based command...\n');
tactor(uint8(6), deviceID, 1, 200, 0);

% Timing comparison between character-based and uint8 commands
tdk.test(deviceID,1e6);

% Shutdown the library
tactor('shutdown');

% Print help at the end
fprintf(1, '\nInterface Help:\n');
tactor('help');
