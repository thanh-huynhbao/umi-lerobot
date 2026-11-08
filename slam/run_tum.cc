/**
* This file is part of ORB-SLAM3
*
* Copyright (C) 2017-2021 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
* Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
*
* ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/

#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <System.h>

using namespace std;

void LoadImages(const string &strImagePath, const string &strPathTimes,
                vector<string> &vstrImages, vector<double> &vTimeStamps);

int main(int argc, char **argv)
{
    if(argc < 5)
    {
        cerr << endl << "Usage: ./run_tum path_to_vocabulary path_to_settings path_to_sequence_folder_1 path_to_times_file_1 (trajectory_file_name)" << endl;
        return 1;
    }

    const int num_seq = (argc - 3) / 2;
    cout << "num_seq = " << num_seq << endl;
    bool bFileName = (((argc - 3) % 2) == 1);
    string file_name;
    if (bFileName)
    {
        file_name = string(argv[argc - 1]);
        cout << "File name output: " << file_name << endl;
    }

    // Load tất cả sequences
    int seq;
    vector< vector<string> > vstrImageFilenames(num_seq);
    vector< vector<double> > vTimestampsCam(num_seq);
    vector<int> nImages(num_seq);

    int tot_images = 0;
    for (seq = 0; seq < num_seq; seq++)
    {
        cout << "Loading images for sequence " << seq << "... ";

        string pathSeq(argv[(2 * seq) + 3]);
        string pathTimeStamps(argv[(2 * seq) + 4]);

        string pathCam0 = pathSeq + "/mav0/cam0/data";

        LoadImages(pathCam0, pathTimeStamps, vstrImageFilenames[seq], vTimestampsCam[seq]);
        cout << "LOADED " << vstrImageFilenames[seq].size() << " frames!" << endl;

        nImages[seq] = vstrImageFilenames[seq].size();
        tot_images += nImages[seq];

        if(nImages[seq] <= 0)
        {
            cerr << "ERROR: Failed to load images for sequence " << seq << endl;
            return 1;
        }
    }

    // Khởi tạo SLAM System
    // Đặt tham số cuối là false để tắt Pangolin Viewer (tránh crash OpenGL khi chạy qua SSH)
    // Nếu bạn đang cắm màn hình trực tiếp và có GUI, có thể đổi thành true
    bool bUseViewer = true;
    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::MONOCULAR, bUseViewer);
    float imageScale = SLAM.GetImageScale();

    vector<float> vTimesTrack(tot_images);
    double t_resize = 0.f;
    double t_track = 0.f;

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;

    int proccIm = 0;
    for (seq = 0; seq < num_seq; seq++)
    {
        cv::Mat im;
        for(int ni = 0; ni < nImages[seq]; ni++, proccIm++)
        {
            // Đọc ảnh nguyên gốc từ file
            im = cv::imread(vstrImageFilenames[seq][ni], cv::IMREAD_UNCHANGED);
            double tframe = vTimestampsCam[seq][ni];

            if(im.empty())
            {
                cerr << endl << "Failed to load image at: " << vstrImageFilenames[seq][ni] << endl;
                continue;
            }

            // XỬ LÝ ĐẶC THÙ CHO DATASET 16-BIT (TUM-VI 512_16):
            // Nếu ảnh là 16-bit (CV_16U), bắt buộc chuyển đổi sang 8-bit (CV_8U)
            if (im.depth() == CV_16U)
            {
                im.convertTo(im, CV_8U, 1.0 / 256.0);
            }
            // Nếu ảnh có 3 hoặc 4 kênh màu, chuyển sang Grayscale 1 kênh
            if (im.channels() == 3)
            {
                cv::cvtColor(im, im, cv::COLOR_BGR2GRAY);
            }
            else if (im.channels() == 4)
            {
                cv::cvtColor(im, im, cv::COLOR_BGRA2GRAY);
            }

            if(imageScale != 1.f)
            {
#ifdef REGISTER_TIMES
                std::chrono::steady_clock::time_point t_Start_Resize = std::chrono::steady_clock::now();
#endif
                int width = im.cols * imageScale;
                int height = im.rows * imageScale;
                cv::resize(im, im, cv::Size(width, height));
#ifdef REGISTER_TIMES
                std::chrono::steady_clock::time_point t_End_Resize = std::chrono::steady_clock::now();
                t_resize = std::chrono::duration_cast<std::chrono::duration<double, std::milli> >(t_End_Resize - t_Start_Resize).count();
                SLAM.InsertResizeTime(t_resize);
#endif
            }

            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

            // Truyền ảnh vào SLAM
            SLAM.TrackMonocular(im, tframe);

            std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

#ifdef REGISTER_TIMES
            t_track = t_resize + std::chrono::duration_cast<std::chrono::duration<double, std::milli> >(t2 - t1).count();
            SLAM.InsertTrackTime(t_track);
#endif

            double ttrack = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();
            vTimesTrack[ni] = ttrack;

            // Đồng bộ thời gian thực giữa các frame
            double T = 0;
            if(ni < nImages[seq] - 1)
                T = vTimestampsCam[seq][ni + 1] - tframe;
            else if(ni > 0)
                T = tframe - vTimestampsCam[seq][ni - 1];

            if(ttrack < T)
                usleep((T - ttrack) * 1e6);
        }

        if(seq < num_seq - 1)
        {
            cout << "Changing the dataset" << endl;
            SLAM.ChangeDataset();
        }
    }

    // Tắt các luồng SLAM
    SLAM.Shutdown();

    // Xuất quỹ đạo
    if (bFileName)
    {
        const string kf_file = "kf_" + file_name + ".txt";
        const string f_file = "f_" + file_name + ".txt";
        SLAM.SaveTrajectoryEuRoC(f_file);
        SLAM.SaveKeyFrameTrajectoryEuRoC(kf_file);
        cout << "Đã lưu quỹ đạo vào: " << f_file << " và " << kf_file << endl;
    }
    else
    {
        SLAM.SaveTrajectoryEuRoC("CameraTrajectory.txt");
        SLAM.SaveKeyFrameTrajectoryEuRoC("KeyFrameTrajectory.txt");
        cout << "Đã lưu quỹ đạo vào CameraTrajectory.txt và KeyFrameTrajectory.txt" << endl;
    }

    return 0;
}

void LoadImages(const string &strImagePath, const string &strPathTimes,
                vector<string> &vstrImages, vector<double> &vTimeStamps)
{
    ifstream fTimes;
    fTimes.open(strPathTimes.c_str());
    vTimeStamps.reserve(10000);
    vstrImages.reserve(10000);

    string s;
    while(getline(fTimes, s))
    {
        // Bỏ qua dòng trống và dòng comment/header bắt đầu bằng '#'
        if(s.empty() || s[0] == '#')
            continue;

        // Chuẩn hóa dấu phẩy thành khoảng trắng
        for (char &c : s) {
            if (c == ',') c = ' ';
        }

        stringstream ss(s);
        double t;
        string imgFile;
        if (ss >> t >> imgFile)
        {
            // Timestamp từ nanoseconds sang seconds
            vTimeStamps.push_back(t / 1e9);

            // Tự động bổ sung đuôi .png nếu chưa có
            if (imgFile.find('.') == string::npos) {
                imgFile += ".png";
            }

            vstrImages.push_back(strImagePath + "/" + imgFile);
        }
    }
}