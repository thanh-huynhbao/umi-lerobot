import os
import subprocess

def main():
    ORB_SLAM3_DIR = os.path.expanduser("/home/thanh-huynhbao/git/umi-lerobot/submodules/ORB_SLAM3")
    VOCAB_PATH = os.path.join(ORB_SLAM3_DIR, "Vocabulary/ORBvoc.txt")
    CONFIG_PATH = os.path.join(
        ORB_SLAM3_DIR, "Examples/Monocular/EuRoC.yaml"
    )  # hoặc file yaml tương ứng với camera của sequence này
    DATASET_PATH = (
        "/home/thanh-huynhbao/git/umi-lerobot/tum/dataset-corridor4_512_16"
    )
    CAM_CSV_PATH = os.path.join(DATASET_PATH, "mav0/cam0/data.csv")

    EXECUTABLE = os.path.join(ORB_SLAM3_DIR, "Examples/Monocular/mono_euroc")

    cmd = [EXECUTABLE, VOCAB_PATH, CONFIG_PATH, DATASET_PATH, CAM_CSV_PATH]

    subprocess.run(cmd, check=True)