function setState(deviceID, state)
%SETSTATE  Sets the state of up to 64 connected tactors based on bit value.
arguments
    deviceID (1,1) {mustBeInteger} %#ok<*INUSA>
    state (1,1) {mustBeInteger}
end

error("This really does not seem to work, and possibly makes it impossible to use the device until after you restart MATLAB.");

% uint8(13) == 'setState'
tactor(uint8(13), deviceID, state); %#ok<*UNRCH>

end

