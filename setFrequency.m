function setFrequency(deviceID, freq, delay)
%SETFREQUENCY Sets the frequency (Hz).
%
% Syntax:
%   tdk.setFrequency(deviceID, freq);
%   tdk.setFrequency(deviceID, freq, delay);

arguments
    deviceID (1,1) {mustBeInteger} % Identifier for device
    freq (1,1) int16 {mustBeInRange(freq,300,3550)}
    delay (1,1) int16 {mustBeInteger, mustBeInRange(delay,0,255)} = 0; % Value before scaling by setTimeFactor(scalar)
end

% uint8(8) == 'changeFreq' code
tactor(uint8(8), deviceID, 1, freq, delay);

end