# ECE3140 Final Project for Felicia(ff267) & Yihan (yz2788)!

# 💃🕺🪩 Shake It Up! 🪩🕺💃

<p align="center">
    <img src="assets/game.png" width="800"><br>
    <!-- <em>Project main image: Shake It Up!</em> -->
</p>


## Introduction 

Our project implements a rhythm-based interactive game called “Shake It Up!” using the FRDM-KL46Z board. The system uses the onboard MMA8451Q accelerometer to track a player’s motion and determine whether their movements match the timing of a music track, providing feedback such as “PERFECT,” “GOOD,” or “MISS” through a Python interface on a computer. Inspired by games like Just Dance, we explored how a low-cost embedded system can perform motion sensing and basic gesture recognition without cameras or external sensors. The board handles real-time processing by sampling acceleration data via I2C, computing motion magnitude to detect deliberate movements, and sending processed results to the host computer over UART for visualization and scoring. We successfully built a working prototype that detects motion and synchronizes it with external feedback. Through this project, we learned how to integrate sensing, real-time processing, and communication in an embedded system, and gained experience debugging and organizing code across both embedded C and Python.

[Check Our Video Here](https://youtube.com/shorts/vkBay1WjeI4)
<div style="width:100%;height:0;position:relative;padding-bottom:64.923%;">
  <iframe
    src="https://youtube.com/embed/vkBay1WjeI4"
    frameborder="0"
    allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
    allowfullscreen
    style="width:100%;height:100%;position:absolute;left:0;top:0;overflow:hidden;">
  </iframe>
</div>

<br><br>

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

### Accelerometer and Gesture Recognition
* Printed the raw X, Y, and Z acceleration values from the MMA8451Q sensor to make sure the board was reading data correctly.
* Waved the board manually in four directions: Up, Down, Left, and Right, to check if each motion produced distinguishable sensor values.
* Adjusted the threshold values in the C code so intentional waves were detected, while small hand tremors and idle noise were ignored.
* Fine-tuned the gesture detection logic by adding a cooldown, so one wave would be counted as one input instead of multiple rapid triggers.

### Bluetooth Communication and Signal Stability
* Checked that the HC-06 Bluetooth module could pair with the PC and create a stable wireless connection.
* Used demo.ipynb to test whether Up, Down, Left, and Right gestures could be received correctly over Bluetooth.
* Tested the Bluetooth range to make sure normal player movement would not cause disconnection.
* Monitored the Python serial output to see if Bluetooth introduced noticeable latency compared to wired UART.

### Python and GUI: Interaction and Synchronization
* Let Python generate random directional prompts and manually performed the matching board movements to test responsiveness.
* Verified that the arrow images, including up.png, down.png, left.png, and right.png, loaded and appeared in the correct screen positions.
* Checked that the feedback images, including perfect.png, good.png, and miss.png, displayed correctly based on player timing.
* Tested perfect, good, and miss cases to make sure the score and combo updated in real time.
* Confirmed that the final score and max combo were displayed correctly when the game ended.
* Checked that the board movement and screen animation stayed synchronized without noticeable delay.



## Resources
[Music Link Here](https://open.spotify.com/track/0vor7b1zPli7ROBMPPZzTp?si=14eb88aada3d448c)

## Work Distribution
<!-- - Describe how you worked together, and who did what. If you encountered difficulties, then how did you deal with them? -->

The initial planning and system architecture were completed collaboratively by both of us. While Yihan focused on developing the DSP filtering, MMA8451 driver, and the Python GUI, Felicia focused on the hardware connections, game logic firmware, and the Bluetooth HC-06 integration, we helped modify each other’s code and setup whenever one encountered a problem. We met frequently to debug and test the human interaction elements together. Each of us has a solid understanding of every aspect of the project and contributed an equal amount of work to the final system.

## AI Usag
We utilized Generative AI to produce our visual assets. This included generating the game background, the directional arrows (up.png, down.png, left.png, right.png), and the real-time performance signals (perfect.png, good.png, miss.png) used for user feedback.
