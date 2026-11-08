import os
import cv2

def extract_insv_to_tum_vi_512(
    insv_path: str,
    output_dir: str,
    target_size: tuple = (512, 512),
    start_time_ns: int = 0
):
    """
    Trích xuất frame từ video .insv:
    - Crop nửa phải (Right half fisheye)
    - Resize về 512x512
    - Chuyển thành grayscale 8-bit
    - Lưu cấu trúc TUM-VI / EuRoC
    """
    if not os.path.exists(insv_path):
        raise FileNotFoundError(f"Không tìm thấy file: {insv_path}")

    # Tạo thư mục theo chuẩn TUM-VI: mav0/cam0/data
    cam0_dir = os.path.join(output_dir, "mav0", "cam0")
    data_dir = os.path.join(cam0_dir, "data")
    os.makedirs(data_dir, exist_ok=True)
    
    csv_path = os.path.join(cam0_dir, "data.csv")

    cap = cv2.VideoCapture(insv_path)
    if not cap.isOpened():
        raise RuntimeError(f"Không thể mở file video: {insv_path}")

    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    print(f"[*] Thông tin video gốc:")
    print(f"    - Kích thước gốc: {width} x {height}")
    print(f"    - FPS: {fps:.2f}")
    print(f"    - Tổng số frame: {total_frames}")
    print(f"    - Kích thước xuất: {target_size[0]} x {target_size[1]}")

    # Khoảng cách thời gian giữa 2 frame (nanoseconds)
    dt_ns = int(1e9 / fps) if fps > 0 else int(1e9 / 30.0)

    frame_count = 0
    csv_lines = []

    print("[*] Đang xử lý và xuất frames...")
    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # 1. CROP PHÂN NỬA BÊN PHẢI (Right half)
        # Nếu video là side-by-side 3840x1920 -> lấy từ x = 1920 đến 3840
        half_w = width // 2
        frame_right = frame[:, half_w:]

        # 2. SCALE / RESIZE VỀ 512x512
        # Sử dụng cv2.INTER_AREA tối ưu cho downscale
        frame_resized = cv2.resize(frame_right, target_size, interpolation=cv2.INTER_AREA)

        # 3. CHUYỂN SANG GRAYSCALE 8-BIT
        if len(frame_resized.shape) == 3:
            frame_gray = cv2.cvtColor(frame_resized, cv2.COLOR_BGR2GRAY)
        else:
            frame_gray = frame_resized

        # 4. TÍNH TIMESTAMP (ns) VÀ LƯU ẢNH
        current_ts_ns = start_time_ns + (frame_count * dt_ns)
        img_filename = f"{current_ts_ns}.png"
        img_full_path = os.path.join(data_dir, img_filename)

        cv2.imwrite(img_full_path, frame_gray)

        # Định dạng dòng data.csv: timestamp filename (dùng khoảng trắng để code C++ parse mượt nhất)
        csv_lines.append(f"{current_ts_ns} {img_filename}\n")

        frame_count += 1
        if frame_count % 100 == 0:
            print(f"    -> Đã xử lý {frame_count}/{total_frames} frames...")

    cap.release()

    # Ghi file data.csv không header
    with open(csv_path, "w", encoding="utf-8") as f:
        f.writelines(csv_lines)

    print(f"\n[V] Hoàn tất!")
    print(f"    - Đã lưu {frame_count} ảnh kích thước {target_size} tại: {data_dir}")
    print(f"    - File data.csv tại: {csv_path}")

if __name__ == "__main__":
    INPUT_INSV = "/home/thanh-huynhbao/git/umi-lerobot/insv/VID_20261105_154741_00_011.insv"
    OUTPUT_DATASET = "/home/thanh-huynhbao/git/umi-lerobot/insv-dataset/011"

    extract_insv_to_tum_vi_512(
        insv_path=INPUT_INSV,
        output_dir=OUTPUT_DATASET,
        target_size=(512, 512),
        start_time_ns=0
    )