function setFrequencyRamp(deviceID, startFreq, endFreq, duration)
%SETFREQUENCYRAMP Sets an intensity ramp with 1 being maximum vibration strength and 0 being off.
%
% Syntax:
%   tdk.setFrequencyRamp(deviceID, startFreq, endFreq, duration);

arguments
    deviceID (1,1) {mustBeInteger} %#ok<*INUSA> % Identifier for device
    startFreq (1,1) int16 {mustBeInRange(startFreq,300,3500)}
    endFreq (1,1) int16 {mustBeInRange(endFreq,300,3500)}
    duration (1,1) int16 {mustBeInteger, mustBeInRange(duration,0,2500)}; % Value before scaling by setTimeFactor(scalar)
end

% uint8(10) == 'rampFreq' code
tactor(uint8(10), deviceID, 1, startFreq, endFreq, duration, 0);

end