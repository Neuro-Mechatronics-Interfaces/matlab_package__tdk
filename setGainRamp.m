function setGainRamp(deviceID, startGain, endGain, duration)
%SETGAINRAMP Sets an intensity ramp with 1 being maximum vibration strength and 0 being off.
%
% Syntax:
%   tdk.setGainRamp(deviceID, startGain, endGain, duration);

arguments
    deviceID (1,1) {mustBeInteger} %#ok<*INUSA> % Identifier for device
    startGain (1,1) double {mustBeInRange(startGain,0,1)}
    endGain (1,1) double {mustBeInRange(endGain,0,1)}
    duration (1,1) int16 {mustBeInteger, mustBeInRange(duration,0,255)}; % Value before scaling by setTimeFactor(scalar)
end

error("This does not seem to work.");

valStart = uint8(round(255.0 * startGain)); %#ok<*UNRCH>
valEnd = uint8(round(255.0 * endGain));

% uint8(9) == 'rampGain' code
tactor(uint8(9), deviceID, 1, valStart, valEnd, duration, 0);

end