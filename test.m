function test(deviceID, numLoops)
%TEST Runs performance test using pulse, timing over desired number of loop iterations to check how use of uint8 codes improves performance vs character parsing.
%
% Syntax:
%   tdk.test(deviceID, numLoops);
%
% Inputs:
%   deviceID - Integer index of tactor to pulse
%   numLoops - Number of times to iterate each comparison loop.
%
% See also: tdk.example, tdk.install

fprintf(1, '\nRunning timing comparison...\n');

% Character-based timing
tic;
for i = 1:numLoops
    tactor('pulse', deviceID, 1, 10, 0);
end
charTime = toc;

% uint8-based timing
tic;
for i = 1:numLoops
    tactor(uint8(5), deviceID, 1, 10, 0);
end
uint8Time = toc;

fprintf(1, 'Timing Results:\n');
fprintf(1, '  Character-based commands: %.6f seconds (%d iterations)\n', charTime, numLoops);
fprintf(1, '  uint8-based commands:     %.6f seconds (%d iterations)\n', uint8Time, numLoops);
fprintf(1, '  Efficiency gain: %.2f%%\n', ((charTime - uint8Time) / charTime) * 100);

end