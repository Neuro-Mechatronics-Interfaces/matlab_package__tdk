function deviceID = open(options)
%OPEN Opens first detected tactor device connected via WinUSB. 

arguments
    options.Reset (1,1) logical = false;
    options.Verbose (1,1) logical = true;
end

tdk.setup();

% Make sure we have initialized
tactor('initialize');

isConnected = tactor('checkConnection');
if isConnected
    deviceName = char(tactor('getName', 0));
    if options.Verbose
        fprintf(1,'Already had open tactor connection with device %s.\n', deviceName);
    end
    if options.Reset
        tactor('shutdown');
        pause(0.1);
        tactor('initialize');
        pause(0.1);
    else
        deviceID = 0;
        return;
    end
end

% Discover USB devices
numDevices = tactor('discover', 1); % 1: Specifies Windows USB

if numDevices < 1
    error('No devices found! Please ensure the device is connected.');
else
    if options.Verbose
        fprintf(1,'Discovered devices: %d\n', numDevices);
    end
end

% Obtain the device name programmatically
deviceName = char(tactor('getName', 0)); % Select the first device (0)
if options.Verbose
    fprintf(1, 'Using device: %s\n', deviceName);
end

% Connect to the device via WindowsUSB (1)
deviceID = tactor('connect', deviceName, 1);
end

