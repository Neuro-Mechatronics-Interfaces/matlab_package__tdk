function setGain(deviceID, gain, delay)
%SETGAIN Sets the intensity with 1 being maximum vibration strength and 0 being off.
%
% Syntax:
%   tdk.setGain(deviceID, gain);
%   tdk.setGain(deviceID, gain, delay);

arguments
    deviceID (1,1) {mustBeInteger} % Identifier for device
    gain (1,1) double {mustBeInRange(gain,0,1)}
    delay (1,1) int16 {mustBeInteger, mustBeInRange(delay,0,255)} = 0; % Value before scaling by setTimeFactor(scalar)
end

val = uint8(round(255.0 * gain));
% uint8(7) == 'changeGain' code
tactor(uint8(7), deviceID, 1, val, delay);

end