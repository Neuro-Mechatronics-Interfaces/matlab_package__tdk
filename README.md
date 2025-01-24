# TDK Vibrotactor MATLAB Interface

This project provides a MATLAB MEX-based interface to control TDK vibrotactors using the `tactor` MEX function. The interface supports both character-based and integer-based commands, enabling efficient communication with TDK devices.

## Installation

### Requirements
- MATLAB R2024b or later
- Microsoft Visual C++ Compiler (e.g., MSVC 2022)
- TDK API files, including required `.dll` and `.lib` files

### Setup Steps
1. Place the `+tdk` folder in your MATLAB path. The best way is by cloning from git and adding to your own git project as a gitmodule:  
   ```batch
   cd your_project_folder
   git submodule add git@github.com:Neuro-Mechatronics-Interfaces/matlab_package__tdk.git +tdk
   ```
2. From MATLAB, run `tdk.install` to compile the MEX file:
   ```matlab
   cd your_project_folder;
   tdk.install();
   ```
   Use `tdk.install(true)` to force recompilation if needed.
3. Ensure the `TDK_API` folder containing required `.dll` files is accessible by running:
   ```matlab
   tdk.setup(); % Should be used at the start of any function that references `tactor`
   ```
   Now, you can use `tactor(...);` from MATLAB as a `mex` wrapper to the TDK API as explained below.  

## Interface Functions

### 1. `tactor`
The main MEX function providing commands to interact with the vibrotactor.

#### Syntax
```matlab
tactor(<command>, <args>...)
```

#### Commands
| Command                    | Arguments                                                                 | Description                                                                 |
|----------------------------|---------------------------------------------------------------------------|-----------------------------------------------------------------------------|
| `'initialize'`             | None                                                                      | Initializes the TDK tactor interface.                                       |
| `'shutdown'`               | None                                                                      | Shuts down the TDK tactor interface and releases resources.                 |
| `'discover'`               | `<type>` (integer)                                                        | Discovers devices of the specified type (e.g., `1` for USB on Windows).     |
| `'getName'`                | `<index>` (integer)                                                       | Retrieves the name of the discovered device at the given index.             |
| `'connect'`                | `<name>` (string), `<type>` (integer)                                     | Connects to the device with the specified name and type.                    |
| `'pulse'`                  | `<deviceID>` (integer), `<tactor>` (integer), `<duration>` (ms), `<delay>` (ms) | Sends a pulse command to the specified tactor.                              |
| `'changeGain'`             | `<deviceID>` (integer), `<tactor>` (integer), `<gain>` (1-255), `<delay>` (ms) | Adjusts the gain (intensity) of the specified tactor.                       |

#### Integer Command Codes
For performance-critical applications, use the following `uint8` command codes instead of strings:
| Command Code | Equivalent String Command |
|--------------|---------------------------|
| `1`          | `'initialize'`            |
| `2`          | `'shutdown'`              |
| `3`          | `'discover'`              |
| `4`          | `'connect'`               |
| `5`          | `'pulse'`                 |
| `6`          | `'changeGain'`            |
| `7`          | `'getName'`               |

### 2. MATLAB Scripts

#### `tdk.install`
Compiles the `tactor.cpp` file into a MEX file.
- **Usage**:
  ```matlab
  tdk.install();      % Compile MEX file
  tdk.install(true);  % Force recompilation
  ```

#### `tdk.setup`
Adds the `TDK_API` folder to the MATLAB path to ensure required DLLs are accessible.
- **Usage**:
  ```matlab
  tdk.setup();
  ```

#### `tdk.example`
Demonstrates the usage of the `tactor` interface.
- **Key Operations**:
  - Initializes the library.
  - Discovers and connects to a device.
  - Pulses a tactor and adjusts its gain.
  - Runs a timing comparison between character-based and uint8 commands.

#### `tdk.test`
Performs a timing comparison between character-based and uint8-based commands.
- **Usage**:
  ```matlab
  tdk.test(deviceID, numLoops);
  ```
- **Output**:
  Prints timing results and efficiency gains.

## Function Details

### `pulse`
Sends a pulse command to the specified tactor.

#### Syntax
```matlab
tactor('pulse', deviceID, tactor, duration, delay);
```

#### Parameters
- `deviceID`: Integer identifier of the connected device.
- `tactor`: Integer specifying the tactor to pulse (e.g., `1` for the first tactor).
- `duration`: Duration of the pulse in milliseconds.
- `delay`: Delay before the pulse starts in milliseconds.

#### Example
```matlab
% Pulse tactor 1 on device 0 for 100ms with no delay
tactor('pulse', deviceID, 1, 100, 0);
```

### `changeGain`
Adjusts the intensity (gain) of a tactor.

#### Syntax
```matlab
tactor('changeGain', deviceID, tactor, gain, delay);
```

#### Parameters
- `deviceID`: Integer identifier of the connected device.
- `tactor`: Integer specifying the tactor to adjust (e.g., `1` for the first tactor).
- `gain`: Integer between `1` (lowest) and `255` (highest) specifying the desired intensity.
- `delay`: Delay before the gain adjustment is applied, in milliseconds.

#### Notes
- Setting `tactor = 0` adjusts the gain for all tactors.
- Gains closer to `255` produce higher vibration intensity.

#### Example
```matlab
% Set the gain of tactor 1 to maximum with no delay
tactor('changeGain', deviceID, 1, 255, 0);
```

## Timing Performance
Using integer-based command codes (e.g., `uint8(5)` for `'pulse'`) improves performance by avoiding string parsing overhead.

#### Example Timing Comparison
```matlab
% Run timing comparison with 10,000 iterations
tdk.test(deviceID, 10000);
```
Output:
```plaintext
Timing Results:
  Character-based commands: 1.234567 seconds (10000 iterations)
  uint8-based commands:     0.567890 seconds (10000 iterations)
  Efficiency gain: 54.02%
```

## Troubleshooting
- **Error: `Connect failed with error code: 202001`**
  - Ensure the correct device name is used (e.g., obtained via `tactor('getName', 0)`).
  - Check device connection in the Windows Device Manager.

- **MEX File Not Found**:
  - Run `tdk.install()` and ensure `tactor.mexw64` is in the `TDK_API` folder.

## License
This project is distributed under the MIT License.

