import subprocess
import sys

# Định nghĩa câu lệnh và các tham số dưới dạng danh sách
cmd = [
    "./slam/run_tum",
    "slam/Vocabulary/ORBvoc.txt",
    "slam/config/TUM-V.yaml",
    "tum/dataset-corridor4_512_16/",
    "tum/dataset-corridor4_512_16/mav0/cam0/data.csv",
]

try:
    print(f"Đang thực thi lệnh: {' '.join(cmd)}")
    # check=True sẽ ném ra exception nếu tiến trình kết thúc với mã lỗi (exit code != 0)
    result = subprocess.run(cmd, check=True)
    print("\nThực thi hoàn tất thành công!")
except subprocess.CalledProcessError as e:
    print(f"\nLỗi khi thực thi lệnh! Mã thoát: {e.returncode}", file=sys.stderr)
except FileNotFoundError:
    print(f"\nKhông tìm thấy file thực thi: {cmd[0]}", file=sys.stderr)