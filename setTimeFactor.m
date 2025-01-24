function setTimeFactor(scalar)
%SETTIMEFACTOR  Sets interface to multiply any time (duration/delay) value by this scalar (default = 1).
arguments
    scalar (1,1) uint8 {mustBeInRange(scalar,1,255)} = 1;
end
% 'setTimeFactor' == uint8(6)
tactor(uint8(6), scalar);
end