function stop(deviceID)
%STOP Stops all pulsing tactors.
%
% Syntax:
%   tdk.stop(deviceID);

arguments
    deviceID (1,1) {mustBeInteger} % Identifier for device
end

% uint8(12) == 'stop' code
tactor(uint8(12), deviceID, 0);

end