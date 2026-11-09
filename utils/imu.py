import csv
import math
import telemetry_parser

INPUT_VIDEO = '/home/thanh-huynhbao/git/umi-lerobot/insv/VID_20261105_154741_00_011.insv'
OUTPUT_CSV = '/home/thanh-huynhbao/git/umi-lerobot/insv-dataset/011/mav0/imu0/data.csv'

# Initialize parser and get normalized IMU records
tp = telemetry_parser.Parser(INPUT_VIDEO)
imu_data = tp.normalized_imu()

DEG_TO_RAD = math.pi / 180.0

with open(OUTPUT_CSV, mode='w', newline='') as f:
    writer = csv.writer(f)

    # EuRoC MAV header format
    header = [
        '#timestamp [ns]',
        'w_RS_S_x [rad s^-1]',
        'w_RS_S_y [rad s^-1]',
        'w_RS_S_z [rad s^-1]',
        'a_RS_S_x [m s^-2]',
        'a_RS_S_y [m s^-2]',
        'a_RS_S_z [m s^-2]'
    ]
    writer.writerow(header)

    for entry in imu_data:
        # 1. Convert timestamp (ms -> ns integer)
        timestamp_ms = entry['timestamp_ms']
        timestamp_ns = int(round(timestamp_ms * 1_000_000))

        # 2. Gyroscope: convert deg/s -> rad/s
        gx, gy, gz = entry['gyro']
        w_x = gx * DEG_TO_RAD
        w_y = gy * DEG_TO_RAD
        w_z = gz * DEG_TO_RAD

        # 3. Accelerometer: already in m/s^2
        ax, ay, az = entry['accl']

        writer.writerow([
            timestamp_ns,
            f"{w_x:.10f}",
            f"{w_y:.10f}",
            f"{w_z:.10f}",
            f"{ax:.10f}",
            f"{ay:.10f}",
            f"{az:.10f}"
        ])

print(f"Successfully exported {len(imu_data)} IMU records to {OUTPUT_CSV}")