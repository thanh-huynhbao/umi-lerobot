import telemetry_parser

tp = telemetry_parser.Parser('/home/thanh-huynhbao/git/umi-lerobot/insv/VID_20261105_154741_00_011.insv')
print('Camera: ', tp.camera)
print('Model: ', tp.model)

# return only gyro and accel with timestamps, normalized to a single orientation and scaled to deg/s and m/s2
print('Normalized IMU', tp.normalized_imu())