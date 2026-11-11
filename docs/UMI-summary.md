# UMI Paper Summary
**Author**: Thanh B. Huynh

## Method
UMI can be defined as a hand-held data collection and policy learning framework. It not only capture expert demonstrations but also deploy robot policies. The design of UMI method satisfied below requirements:
1. **Portable**: can be taken to any enviroment and collect data with close-to-zero setup time.
2. **Capable**: apart from pick-and-place, UMI is able to capture and transfer complex human manipulation skills.
3. **Sufficient**: collected data should be rich and informative for robot learning policies, but reduce the embodiment-specific information.
4. **Reproducible**: Every one can own a UMI gripper.
### A. Demonstration Interface Design
UMI have the form of a trigger-activated device. The trigger mechanism help controlling the width of soft finger gipper by human hand. In addition, a camera with fisheye lens and integrated IMU sensor is mounted on wrist and play as the only data receiving and recording device. 

The collected data contain both **observation** and **action**. Under the **observation** data, **informative visual context** and critical **depth information** should be captured. On the other hand, the action ought to have **precise robot action** under **fast human motion**, detailed **gripper's width** and be able to **validate each demonstration** in comparison with robot kinematics. 

**HD1. Wrist mounted cameras as input observation**
Rely on an only wrist-mounted camera. The setup of camera and gripper on the hand-held gripper and robot hardware should be the same. It brings following advantages:
1. Minimizing the observation embodiment gaps: the videos observed by on wrist camera are almost the same for human demonstration and robot deployment.
2. Mechanical robustness: UMI on robots does not require camera-robot-world calibration.
3. Portable hardware setup: with only on wrist camera, the setup becomes easier.
4. Camera motion for natural data diversification: training with a moving camera make the learning policy focus on the task-relevant objects or regions instead of background structures.

**Issues:** policy faces to non-stationary and partial observations. 

**Solutions:** wide-FoV Fisheye Lens and visual tracking

**HD2. Fisheye Lens for visual context**
Fisheye lens provide sufficient visual context for a wide range of tasks. By using the directly raw fisheye images, the resolution in the center is preserved while the information in the peripheral view is compressed. Also, fisheye lens improves policy performance. 

**HD4. IMU-aware tracking**
An inertial-monocular SLAM system based on ORBSLAM3 leverages visual context provided by fisheye lens and IMU data to maintain tracking even if visual tracking fails due to motion blur or a lack of visual features. This is important for action precision and intergripper pose proprioception

**HD6. Kinematic-based data filtering**
Before feeding demonstration data for training, data should be filted by robot kinematics. The selected demonstration must be qualified both kinematics and dynamics filltering according to the absolute end-effector pose produced by **SLAM pipeline**.

### B. Policy Interface Design
## Evaluation
