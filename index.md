# ECE3140 page for ff267 yz2788!

# 💃🕺🪩 Shake It Up! 🪩🕺💃

## Introduction 

Our project implements a rhythm-based interactive game called “Shake It Up!” using the FRDM-KL46Z board. The system uses the onboard MMA8451Q accelerometer to track a player’s motion and determine whether their movements match the timing of a music track, providing feedback such as “PERFECT,” “GOOD,” or “MISS” through a Python interface on a computer. Inspired by games like Just Dance, we explored how a low-cost embedded system can perform motion sensing and basic gesture recognition without cameras or external sensors. The board handles real-time processing by sampling acceleration data via I2C, computing motion magnitude to detect deliberate movements, and sending processed results to the host computer over UART for visualization and scoring. We successfully built a working prototype that detects motion and synchronizes it with external feedback. Through this project, we learned how to integrate sensing, real-time processing, and communication in an embedded system, and gained experience debugging and organizing code across both embedded C and Python.

## System Overview 

<p align="center">
    <img src="assets/system_diagram.png" ><br>
    <em>Figure 1: System architecture overview</em>
</p>

<!-- - Include a flow chart/block diagram of the system, showing how components interact. This diagram could potentially be replaced by \textit{very} clear writing.
- Please fill this in by the project check-in date (Apr 21st)   -->
<!-- - Video: After the overview is a good place to embed the video. The video is not required by the check-in date.  -->

## System Description 

 Our system consists of two main components: the FRDM-KL46Z board for sensing and real-time processing, and a Python interface on a computer for visualization and feedback. On the embedded side, we use the onboard MMA8451Q accelerometer, which communicates with the microcontroller over I2C. The board samples 3-axis acceleration data at a fixed rate using a PIT timer interrupt.
The accelerometer data is processed directly on the board. We compute the magnitude of the acceleration vector to detect deliberate movements. Once a gesture is detected, the board formats the result and transmits it to the computer using UART, either through USB or an HC-06 Bluetooth module for wireless communication.

<p align="center">
    <img src="assets/hc06.png" width="800"><br>
    <em> Figure 2: Hardware Schematic </em>
</p>

On the computer side, a Python program reads the incoming serial data and synchronizes it with a music track. Based on the timing of the detected movements relative to the music, the program assigns feedback such as “PERFECT,” “GOOD,” or “MISS” and displays it through a GUI. The embedded system focuses on real-time sensing and computation, while the computer handles timing evaluation, visualization, and user interaction.

<!-- - Explain how your system/software works. This should be at an appropriate level of detail to allow us to evaluate design decisions. Feel free to include code snippets where appropriate. Projects with additional hardware must include a schematic.
- Please upload a draft of this text/schematic by the project check-in date (Apr 21st)  -->

## Testing 
- Describe your testing procedure and how you determined that the system works correctly. Some projects may not have traditional test cases (e.g. tested the system by human interaction with it), but all projects must be tested. 

## Resources
- Cite the resources you used for this project. If this is based on another codebase, link it here. If you did not use any other code, please still include this section and state that you did everything from scratch. 

## Work Distribution
- Describe how you worked together, and who did what. If you encountered difficulties, then how did you deal with them?

## AI Usage
- Indicate if you used GenAI for help with this project (as a TA in compliance with the course restrictions of course), and how. 

