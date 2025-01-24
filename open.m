function deviceID = open(verbose)
%OPEN Opens first detected tactor device connected via WinUSB. 

arguments
    verbose (1,1) logical = true;
end

tdk.setup();

% Make sure we have initialized
tactor('initialize');

% Discover USB devices
numDevices = tactor('discover', 1); % 1: Specifies Windows USB

if numDevices < 1
    error('No devices found! Please ensure the device is connected.');
else
    if verbose
        fprintf(1,'Discovered devices: %d\n', numDevices);
    end
end

% Obtain the device name programmatically
deviceName = char(tactor('getName', 0));
if verbose
    fprintf(1, 'Using device: %s\n', deviceName);
end

% Connect to the first device
deviceID = tactor('connect', deviceName, 1);

% Defaults the timescale to unity-gain.
tactor('setTimeFactor', 1);

end

