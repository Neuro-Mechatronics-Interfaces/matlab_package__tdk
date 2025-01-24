function pulse(deviceID, duration, tacNum)
%PULSE Pulse the tactor for the given duration (milliseconds) and optional delay.
%
% Syntax:
%   tdk.pulse(deviceID, duration);
%   tdk.pulse(deviceID, duration, tacNum);

arguments
    deviceID (1,1) {mustBeInteger} % Identifier for device
    duration (1,1) int16 {mustBeInteger, mustBeInRange(duration,1,2500)} % Before scaling by setTimeFactor(scalar)
    tacNum (1,1) int16 {mustBeInteger, mustBeInRange(tacNum,0,255)} = 1;
end

% uint8(11) == 'pulse' code; 
tactor(uint8(11), deviceID, tacNum, duration, 0); % Delay does not seem to work.

end