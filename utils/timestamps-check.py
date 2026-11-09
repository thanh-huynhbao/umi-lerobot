import pandas as pd

df_cam = pd.read_csv('insv-dataset/011/mav0/cam0/data_synced.csv')
df_imu = pd.read_csv('insv-dataset/011/mav0/imu0/imu0_synced.csv')

t_cam_start = df_cam.iloc[0, 0]
t_cam_end = df_cam.iloc[-1, 0]

t_imu_start = df_imu.iloc[0, 0]
t_imu_end = df_imu.iloc[-1, 0]

print(f"Cam range: {t_cam_start} -> {t_cam_end}")
print(f"IMU range: {t_imu_start} -> {t_imu_end}")

if t_imu_start > t_cam_start:
    print(" CẢNH BÁO: IMU bắt đầu MUỘN hơn frame đầu tiên!")
if t_imu_end < t_cam_end:
    print(" CẢNH BÁO: IMU kết thúc TRƯỚC frame cuối cùng!")