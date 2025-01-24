function setup()
%SETUP Adds the TDK dll folder to the current workspace path, if it is not already.
%
% Syntax:
%   tdk.setup();
%
% See also: tdk.install, tdk.example
apiDir = fullfile(pwd,'TDK_API');
if ~contains(path, apiDir)
    addpath(apiDir);
end
end