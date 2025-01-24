# TDK Vibrotactor MATLAB Interface

This project provides a MATLAB MEX-based interface to control TDK vibrotactors using the `tactor` MEX function. The interface supports both character-based and integer-based commands, enabling efficient communication with TDK devices.

## Installation

### Requirements
- MATLAB R2024b or later
- Microsoft Visual C++ Compiler (e.g., MSVC 2022)
- TDK API files, including required `.dll` and `.lib` files

### Setup Steps
1. Place the `+tdk` folder in your MATLAB path. The best way is by cloning from Git and adding it to your project as a submodule:
   ```bash
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

Now, you can use the `tactor` MEX function or the helper package functions in `+tdk` as explained below.

## MATLAB Package Functions

The package provides the following high-level functions for ease of use:

### [`tdk.open`](open.m)
_Status: **Working**_  
Opens the first available device and initializes the TDK library.
- **Usage**:
  ```matlab
  deviceID = tdk.open();
  ```
- **Output**:
  - `deviceID`: Integer identifier for the connected device.

---

### [`tdk.close`](close.m)
_Status: **Working**_  
Closes the connection to the device and shuts down the library.
- **Usage**:
  ```matlab
  tdk.close();
  ```

---

### [`tdk.pulse`](pulse.m)
_Status: **Working**_  
Pulses the connected tactor for a specified duration.
- **Usage**:
  ```matlab
  tdk.pulse(deviceID, duration);
  ```
- **Parameters**:
  - `deviceID`: Integer identifier for the connected device.
  - `duration`: Duration of the pulse in milliseconds.

---

### [`tdk.setGain`](setGain.m)
_Status: **Working**_  
Sets the gain (intensity) of the connected tactor.
- **Usage**:
  ```matlab
  tdk.setGain(deviceID, gain);
  ```
- **Parameters**:
  - `deviceID`: Integer identifier for the connected device.
  - `gain`: Integer value between `1` (minimum) and `255` (maximum).

---

### [`tdk.setFrequency`](setFrequency.m)
_Status: **Working**_  
Sets the frequency of the connected tactor.
- **Usage**:
  ```matlab
  tdk.setFrequency(deviceID, frequency);
  ```
- **Parameters**:
  - `deviceID`: Integer identifier for the connected device.
  - `frequency`: Integer value between `300` and `3550` Hz.

---

### [`tdk.setFrequencyRamp`](setFrequencyRamp.m)
_Status: **Does Nothing**_  
Applies a frequency ramp to the connected tactor.
- **Usage**:
  ```matlab
  tdk.setFrequencyRamp(deviceID, startFreq, endFreq, duration, delay);
  ```
- **Parameters**:
  - `deviceID`: Integer identifier for the connected device.
  - `startFreq`: Start frequency in Hz.
  - `endFreq`: End frequency in Hz.
  - `duration`: Duration of the ramp in milliseconds.
  - `delay`: Delay before starting the ramp in milliseconds.

---

### [`tdk.setGainRamp`](setGainRamp.m)
_Status: **Does Nothing**_  
Applies a gain ramp to the connected tactor.
- **Usage**:
  ```matlab
  tdk.setGainRamp(deviceID, startGain, endGain, duration, delay);
  ```
- **Parameters**:
  - `deviceID`: Integer identifier for the connected device.
  - `startGain`: Start gain (1-255).
  - `endGain`: End gain (1-255).
  - `duration`: Duration of the ramp in milliseconds.
  - `delay`: Delay before starting the ramp in milliseconds.

---

### [`tdk.stop`](stop.m)
_Status: **Untested**_  
Stops all active tactors.
- **Usage**:
  ```matlab
  tdk.stop(deviceID, delay);
  ```
- **Parameters**:
  - `deviceID`: Integer identifier for the connected device.
  - `delay`: Delay before stopping the tactors in milliseconds.

---

## Example Usage

Use [`tdk.example`](example.m) to install and see how to use functions in the package.
```
cd your_project_folder;
tdk.example;
```
