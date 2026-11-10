# UMI-LeRobot

**UMI-LeRobot**: *from UMI data acquiition to LeRobot dataset*

**Author**: Thanh B. Huynh
## Folder Structure
## Dataset
TUM Dataset: [Download dataset-corridor4_512_16](https://cdn2.vision.in.tum.de/tumvi/exported/euroc/512_16/dataset-corridor4_512_16.tar). Then store and extract in `umi-lerobot/tum/` folder

## SLAM
ORBSLAM3: [Install ORBSLAM3](/docs/install-ORBSLAM3.md)

## LeRobot Dataset 

## Quick Start

If you already having tum dataset. At the main folder, you can run the command below to have a fresh start.
```bash
./slam/run_tum \                          # main execute file
    slam/Vocabulary/ORBvoc.txt \          # Vocabulary file
    slam/config/TUM-V.yaml \              # camera config in yaml
    tum/dataset-corridor4_512_16/ \       # dataset folder 
    tum/dataset-corridor4_512_16/mav0/cam0/data.csv # list of frames
```