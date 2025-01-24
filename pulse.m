function pulse(deviceID, duration, delay)
%PULSE Pulse the tactor for the given duration (milliseconds) and optional delay.
%
% Syntax:
%   tdk.pulse(deviceID, duration);
%   tdk.pulse(deviceID, duration, delay);

arguments
    deviceID (1,1) {mustBeInteger} % Identifier for device
    duration (1,1) int16 {mustBeInteger, mustBeInRange(duration,0,255)} % Before scaling by setTimeFactor(scalar)
    delay (1,1) int16 {mustBeInteger, mustBeInRange(delay,0,255)} = 0; % Before scaling
end

% uint8(11) == 'pulse' code
tactor(uint8(11), deviceID, 1, duration, delay);

end