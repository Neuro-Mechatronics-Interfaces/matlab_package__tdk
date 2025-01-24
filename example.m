%EXAMPLE Example illustrating installation and usage of mex tactor interface. Run using `tdk.example;` from Command Window. 

% Run the install script to compile the MEX file
tdk.install();  

% Or, to force recompile:
% tdk.install(true);

% Run the setup script to make sure the dll folder is on workspace path
tdk.setup();

% Initialize mex and open first TDK WinUSB device discovered.
deviceID = tdk.open();

% Set tactor initial pulse to 75% gain and pulse immediately for 100 milliseconds.
tactor('changeGain', deviceID, 1, 191, 0);
tactor('pulse', deviceID, 1, 100, 0);
pause(1);

% Change gain to 25% (0 - off | 1 - max), after a delay of 1s milliseconds
tdk.setFrequency(deviceID, 300); % Set to low end of frequencies
tdk.pulse(deviceID, 250); % Pulse the tactor for 250ms
pause(1);

% Change gain to 25% (0 - off | 1 - max), after a delay of 1s milliseconds
tdk.setFrequency(deviceID, 3000); % Set to high end of frequencies
tdk.pulse(deviceID, 250); % Pulse the tactor for 250ms
pause(1);

% Shutdown the library
tdk.close();

% Print help at the end
fprintf(1, '\nInterface Help:\n');
tactor('help');
