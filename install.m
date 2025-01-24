function install(force)
%INSTALL Compile the tactor.cpp file to a MEX file.
%
%   INSTALL compiles the tactor.cpp file to a MEX file. 
%   The MEX file is saved in the same directory as the 
%   tactor.cpp file.
%
% Syntax:
%   tdk.install();
%   tdk.install(force); % Default: false - set true to overwrite existing mex
%
% See also: tdk.setup, tdk.example, `~/+tdk/src/tactor.cpp`

arguments
    force (1,1) logical = false;
end

% Get the path of this script
thisDir = fileparts(mfilename('fullpath'));
if ~force
    if exist(fullfile(thisDir, 'tactor.mexw64'), 'file') == 0
        fprintf(1, 'No mex file detected. Installing...\n');
    else
        tdk.setup(); % Just in case
        fprintf(1, 'Detected compiled mex. Skipping installation.\n\t->\t(Run tdk.install(true) to force re-compile of mex)\n');
        return;
    end
end

% Paths to headers and libraries
headerPath = fullfile(thisDir, 'TDK_API');
libPath = fullfile(thisDir, 'TDK_API');
libPathOutput = fullfile(pwd, 'TDK_API');

% Output file location
outputPath = thisDir; % MEX file will go in the +tdk directory
sourceFile = fullfile(thisDir, 'src', 'tactor.cpp');

% Explicitly specify the C++17 standard for the compiler
mexCmd = sprintf(['mex -outdir "%s" -output tactor ', ...
                  '-I"%s" -L"%s" -lTactorInterface -lTActionManager ', ...
                  '%s COMPFLAGS="$COMPFLAGS /std:c++17"'], ...
                  outputPath, headerPath, libPath, sourceFile);

% Run the command
disp('Compiling tactor.cpp...');
eval(mexCmd);
disp('Compilation complete.');
if exist(libPathOutput,'dir')==0
    disp('Copying link libraries...');
    copyfile(libPath,libPathOutput);
    disp('Libraries installed.');
end
if contains(path, libPathOutput)
    rmpath(libPathOutput);
end
if exist(fullfile(libPathOutput,'tactor.mexw64'),'file')~=0
    clear tactor; % Ensures that MATLAB is not using the mex file, if it already existed.
end
copyfile(fullfile(outputPath,'tactor.mexw64'), ...
             fullfile(libPathOutput, 'tactor.mexw64'), 'f');
tdk.setup();

end
