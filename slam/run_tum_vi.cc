
/**
* Monocular-Inertial Runner for ORB-SLAM3 (EuRoC / TUM-VI format)
*/

#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <System.h>
#include <ImuTypes.h>

using namespace std;

// Hàm đọc danh sách ảnh
void LoadImages(const string &strImagePath, const string &strPathTimes,
                vector<string> &vstrImages, vector<double> &vTimeStamps);

// Hàm đọc dữ liệu IMU
void LoadIMU(const string &strImuPath, vector<ORB_SLAM3::IMU::Point> &vImuMeas);

int main(int argc, char **argv)
{
    if(argc < 5)
    {
        cerr << endl << "Usage: ./run_mono_inertial path_to_vocabulary path_to_settings path_to_sequence_folder path_to_cam_csv (output_trajectory_name)" << endl;
        cerr << "Example: ./run_mono_inertial Vocabulary/ORBvoc.txt config/TUM-VI.yaml ../insv-dataset/011 ../insv-dataset/011/mav0/cam0/data.csv (my_trajectory)" << endl;
        return 1;
    }

    string strVocFile = string(argv[1]);
    string strSettingsFile = string(argv[2]);
    string pathSeq = string(argv[3]);
    string pathCamCsv = string(argv[4]);

    bool bFileName = (argc >= 6);
    string file_name = bFileName ? string(argv[5]) : "CameraTrajectory";

    // 1. Tải danh sách ảnh
    vector<string> vstrImageFilenames;
    vector<double> vTimestampsCam;
    string pathCam0Data = pathSeq + "/mav0/cam0/data";

    cout << "--> Loading Camera images from: " << pathCam0Data << endl;
    LoadImages(pathCam0Data, pathCamCsv, vstrImageFilenames, vTimestampsCam);
    int nImages = vstrImageFilenames.size();
    cout << "--> Loaded " << nImages << " images." << endl;

    if(nImages <= 0)
    {
        cerr << "ERROR: No images loaded! Check paths." << endl;
        return 1;
    }

    // 2. Tải dữ liệu IMU
    vector<ORB_SLAM3::IMU::Point> vImuMeas;
    string pathImuCsv = pathSeq + "/mav0/imu0/data.csv";
    cout << "--> Loading IMU measurements from: " << pathImuCsv << endl;
    LoadIMU(pathImuCsv, vImuMeas);
    cout << "--> Loaded " << vImuMeas.size() << " IMU samples." << endl;

    if(vImuMeas.empty())
    {
        cerr << "ERROR: No IMU measurements found at: " << pathImuCsv << endl;
        return 1;
    }
    cout << endl << "=================== TIMESTAMP DIAGNOSTIC ===================" << endl;
    cout << "First Camera Frame TS: " << vTimestampsCam.front() << " s (" 
         << (long long)(vTimestampsCam.front() * 1e9) << " ns)" << endl;
    cout << "Last  Camera Frame TS: " << vTimestampsCam.back() << " s (" 
         << (long long)(vTimestampsCam.back() * 1e9) << " ns)" << endl;
    cout << "First IMU Sample   TS: " << vImuMeas.front().t << " s (" 
         << (long long)(vImuMeas.front().t * 1e9) << " ns)" << endl;
    cout << "Last  IMU Sample   TS: " << vImuMeas.back().t << " s (" 
         << (long long)(vImuMeas.back().t * 1e9) << " ns)" << endl;

    // 3. Khởi tạo hệ thống SLAM với chế độ IMU_MONOCULAR
    // bUseViewer = false để chạy ổn định qua headless/SSH
    bool bUseViewer = true;
    ORB_SLAM3::System SLAM(strVocFile, strSettingsFile, ORB_SLAM3::System::IMU_MONOCULAR, bUseViewer);
    float imageScale = SLAM.GetImageScale();

    cout << endl << "-------" << endl;
    cout << "Start processing Monocular-Inertial sequence ..." << endl;

    // Con trỏ theo dõi mẫu IMU theo thời gian
    int first_imu = 0;
    while(first_imu < vImuMeas.size() && vImuMeas[first_imu].t < vTimestampsCam[0])
        first_imu++;

    cv::Mat im;
    for(int ni = 0; ni < nImages; ni++)
    {
        // Đọc ảnh
        im = cv::imread(vstrImageFilenames[ni], cv::IMREAD_UNCHANGED);
        double tframe = vTimestampsCam[ni];

        if(im.empty())
        {
            cerr << "Failed to load image at: " << vstrImageFilenames[ni] << endl;
            continue;
        }

        // Tự động chuyển 16-bit về 8-bit
        if (im.depth() == CV_16U)
        {
            im.convertTo(im, CV_8U, 1.0 / 256.0);
        }
        // Chuyển sang Grayscale 1 kênh
        if (im.channels() == 3)
        {
            cv::cvtColor(im, im, cv::COLOR_BGR2GRAY);
        }
        else if (im.channels() == 4)
        {
            cv::cvtColor(im, im, cv::COLOR_BGRA2GRAY);
        }

        // Resize nếu cấu hình yaml có scale
        if(imageScale != 1.f)
        {
            int width = im.cols * imageScale;
            int height = im.rows * imageScale;
            cv::resize(im, im, cv::Size(width, height));
        }

        // Gom các mẫu IMU phát sinh từ frame trước đến frame hiện tại
        vector<ORB_SLAM3::IMU::Point> vImuMeasCurrent;
        while(first_imu < vImuMeas.size() && vImuMeas[first_imu].t <= tframe)
        {
            vImuMeasCurrent.push_back(vImuMeas[first_imu]);
            first_imu++;
        }

        // Truyền frame ảnh và vector IMU vào SLAM
        SLAM.TrackMonocular(im, tframe, vImuMeasCurrent);

        // Đồng bộ thời gian để chạy theo tốc độ thực
        double T = 0;
        if(ni < nImages - 1)
            T = vTimestampsCam[ni + 1] - tframe;
        else if(ni > 0)
            T = tframe - vTimestampsCam[ni - 1];

        // Có thể mở rộng usleep nếu muốn xem trực tiếp GUI
        // usleep(5000); 
    }

    // 4. Shutdown hệ thống
    SLAM.Shutdown();

    // 5. Lưu quỹ đạo (Kiểm tra an toàn tránh crash nếu không có KeyFrame)
    cout << endl << "Saving camera trajectories..." << endl;
    const string kf_file = "kf_" + file_name + ".txt";
    const string f_file = "f_" + file_name + ".txt";

    SLAM.SaveTrajectoryEuRoC(f_file);
    SLAM.SaveKeyFrameTrajectoryEuRoC(kf_file);
    cout << "--> Exported: " << f_file << " and " << kf_file << endl;

    return 0;
}

void LoadImages(const string &strImagePath, const string &strPathTimes,
                vector<string> &vstrImages, vector<double> &vTimeStamps)
{
    ifstream fTimes;
    fTimes.open(strPathTimes.c_str());
    if(!fTimes.is_open())
    {
        cerr << "Failed to open camera csv: " << strPathTimes << endl;
        return;
    }

    vTimeStamps.reserve(10000);
    vstrImages.reserve(10000);

    string s;
    while(getline(fTimes, s))
    {
        if(s.empty() || s[0] == '#')
            continue;

        for (char &c : s) {
            if (c == ',') c = ' ';
        }

        stringstream ss(s);
        double t;
        string imgFile;
        if (ss >> t >> imgFile)
        {
            vTimeStamps.push_back(t / 1e9);

            if (imgFile.find('.') == string::npos) {
                imgFile += ".png";
            }
            vstrImages.push_back(strImagePath + "/" + imgFile);
        }
    }
}

void LoadIMU(const string &strImuPath, vector<ORB_SLAM3::IMU::Point> &vImuMeas)
{
    ifstream fImu;
    fImu.open(strImuPath.c_str());
    if(!fImu.is_open())
    {
        cerr << "Failed to open IMU file: " << strImuPath << endl;
        return;
    }

    vImuMeas.reserve(500000);

    string s;
    while(getline(fImu, s))
    {
        if(s.empty() || s[0] == '#')
            continue;

        for (char &c : s) {
            if (c == ',') c = ' ';
        }

        stringstream ss(s);
        double t;
        cv::Point3f acc, gyr;
        // Định dạng EuRoC IMU: timestamp [ns], w_x, w_y, w_z, a_x, a_y, a_z
        if (ss >> t >> gyr.x >> gyr.y >> gyr.z >> acc.x >> acc.y >> acc.z)
        {
            vImuMeas.push_back(ORB_SLAM3::IMU::Point(acc.x, acc.y, acc.z,
                                                     gyr.x, gyr.y, gyr.z,
                                                     t / 1e9));
        }
    }
}