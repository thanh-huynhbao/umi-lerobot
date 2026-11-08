import numpy as np
import rerun as rr

# Khởi tạo Rerun Viewer
rr.init("Camera_Trajectory_Viewer", spawn=True)

# Đường dẫn file trajectory
traj_file = "CameraTrajectory.txt"

# Đọc dữ liệu (8 cột: timestamp, tx, ty, tz, qx, qy, qz, qw)
data = np.loadtxt(traj_file)

timestamps = data[:, 0]
positions = data[:, 1:4]  # tx, ty, tz
quaternions = data[:, 4:8]  # qx, qy, qz, qw

# -------------------------------------------------------------
# 1. Vẽ toàn bộ đường quỹ đạo tĩnh (Path 3D)
# -------------------------------------------------------------
# Dùng static=True (thay vì timeless=True)
rr.log(
    "world/trajectory_path",
    rr.LineStrips3D([positions], colors=[[0, 255, 0]]),
    static=True,
)

# -------------------------------------------------------------
# 2. Tua chuyển động Camera Frustum theo Timeline
# -------------------------------------------------------------
for i in range(len(timestamps)):
    ts_ns = int(timestamps[i])
    pos = positions[i]
    quat = quaternions[i]  # [qx, qy, qz, qw]

    # API mới: set_time_nanos đổi thành set_time() với timestamp dạng nanosecond
    rr.set_time("timeline", timestamp=ts_ns)

    # Log vị trí và góc xoay của camera (Transform3D)
    rr.log(
        "world/camera",
        rr.Transform3D(
            translation=pos,
            rotation=rr.Quaternion(xyzw=quat),
        ),
    )

    # Hiển thị Camera Frustum
    rr.log(
        "world/camera",
        rr.Pinhole(
            resolution=[512, 512],
            focal_length=190.0,
            principal_point=[256.0, 256.0],
        ),
    )