import csv
from pathlib import Path

FRAMES_CSV = '/home/thanh-huynhbao/git/umi-lerobot/insv-dataset/011/mav0/cam0/data.csv'       # Input frames file (#timestamp [ns],filename)
IMU_CSV = '/home/thanh-huynhbao/git/umi-lerobot/insv-dataset/011/mav0/imu0/data.csv'          # Input IMU file (#timestamp [ns],w_RS_S_x...)

FRAMES_OUT = 'data_synced.csv'
IMU_OUT = 'imu0_synced.csv'

# Set to True if you want to discard IMU measurements that occurred before t=0 (first frame)
DROP_IMU_BEFORE_ZERO = False


def load_first_timestamp(file_path):
    with open(file_path, mode='r') as f:
        reader = csv.reader(f)
        for row in reader:
            if not row or row[0].startswith('#'):
                continue
            return int(row[0].strip())
    raise ValueError(f"No valid timestamp found in {file_path}")


def sync_timestamps():
    # 1. Read base reference timestamps
    first_frame_ts = load_first_timestamp(FRAMES_CSV)
    first_imu_ts = load_first_timestamp(IMU_CSV)

    print(f"First Frame TS: {first_frame_ts} ns")
    print(f"First IMU TS:   {first_imu_ts} ns")

    # Shift needed so the first IMU measurement starts at t = 0:
    # offset = -first_imu_ts (e.g. -(-3315966000) = +3315966000)
    #
    # If your video frame 0 was already extracted starting from t=0 relative to recording start,
    # and the IMU negative timestamp represents the pre-roll duration before the first video frame:
    # then subtracting first_imu_ts shifts both datasets into the IMU coordinate time:
    time_shift = -first_imu_ts  

    print(f"Applying time shift: {time_shift:+d} ns")

    # 2. Process and write synced Frames CSV
    with open(FRAMES_CSV, mode='r') as f_in, open(FRAMES_OUT, mode='w', newline='') as f_out:
        reader = csv.reader(f_in)
        writer = csv.writer(f_out)

        for row in reader:
            if not row:
                continue
            if row[0].startswith('#'):
                writer.writerow(row)
                continue

            orig_ts = int(row[0].strip())
            # Shift frame timestamp
            new_ts = orig_ts + time_shift
            writer.writerow([new_ts, row[1].strip()])

    # 3. Process and write synced IMU CSV
    imu_count = 0
    with open(IMU_CSV, mode='r') as f_in, open(IMU_OUT, mode='w', newline='') as f_out:
        reader = csv.reader(f_in)
        writer = csv.writer(f_out)

        for row in reader:
            if not row:
                continue
            if row[0].startswith('#'):
                writer.writerow(row)
                continue

            orig_ts = int(row[0].strip())
            new_ts = orig_ts + time_shift

            if DROP_IMU_BEFORE_ZERO and new_ts < 0:
                continue

            writer.writerow([new_ts] + [col.strip() for col in row[1:]])
            imu_count += 1

    print(f"Done! Written to:\n  - {FRAMES_OUT}\n  - {IMU_OUT} ({imu_count} samples)")


if __name__ == '__main__':
    sync_timestamps()