function setFrequency(deviceID, freq)
%SETFREQUENCY Sets the frequency (Hz).
%
% Syntax:
%   tdk.setFrequency(deviceID, freq);

arguments
    deviceID (1,1) {mustBeInteger} % Identifier for device
    freq (1,1) int16 {mustBeInRange(freq,300,3500)}
end

% uint8(8) == 'changeFreq' code
tactor(uint8(8), deviceID, 1, freq, 0); 

end