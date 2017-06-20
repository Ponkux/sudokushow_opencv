//RM sudoku show program
//@author: DJTU TOE
//2017/06/20
//

#include <iostream>
#include <random>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

/*---------------------change the path here-------------------------*/
const string normal_path = "/Users/ding/Desktop/rm_pics/normal/";
const string target_path = "/Users/ding/Desktop/rm_pics/target/";
const string number_path = "/Users/ding/Desktop/rm_pics/newsudoku/";
/*------------------------------------------------------------------*/

int uniform_intx(int a, int b) {
    static std::default_random_engine e{std::random_device{}()}; // avoid "Most vexing parse"
    static std::uniform_int_distribution<int> u;
    return u(e, std::uniform_int_distribution<int>::param_type(a, b));
}

Mat Random_pic(const string &path){
    ostringstream filepos;
    filepos<<path<<uniform_intx(1,9)<<".png";
    return imread(filepos.str(),CV_LOAD_IMAGE_GRAYSCALE);
}

vector<Mat> Old_Result(int &target_c,int position,const string &normal_path,const string &target_path){
    vector<Mat> result_old;
    for(int i=0;i<9;i++){
        if(i != position-1){
            result_old.push_back(Random_pic(normal_path));
        }else{
            ostringstream nm;
            nm<<target_path<<1<<".png";
            result_old.push_back(imread(nm.str(),CV_LOAD_IMAGE_GRAYSCALE));
            target_c = 1;
        }
    }
    return result_old;
}

vector<Mat> New_result(const string &path){
    vector<Mat> result;
    vector<int> alignment;
    while(alignment.size()<9){
        size_t size = alignment.size();
        if(size == 0){
            alignment.push_back(uniform_intx(1, 9));
        }else if(size == 8){
            for(int num=1;num<10;num++){
                int i = 0;
                for(i=0;i<8;i++){
                    if(alignment[i] == num) break;
                }
                if(i==8){
                    alignment.push_back(num);
                    break;
                }
            }
        }else{
            int num = uniform_intx(1, 9);
            int i = 0;
            for(i=0;i<size;i++){
                if(num == alignment[i]) break;
            }
            if(i == size) alignment.push_back(num);
        }
    }
    
    //int randsize[9] = {1135,1032,1010,982,892,958,1028,974,1009};
    int randsize[9] = {96,96,96,96,96,96,96,96,96};
    
    for(int i=0;i<9;i++){
        ostringstream name;
        name<<path<<alignment[i]<<"/"<<uniform_intx(1, randsize[alignment[i]-1])<<".png";
        result.push_back(imread(name.str(),CV_LOAD_IMAGE_GRAYSCALE));
    }
    return result;
}

int main(int argc, const char * argv[]) {
    int old_position=5,position = 5; // init
    int key;
    double target_delay = 0.25,target_total = 0;
    double time0,time_total = 1.6;
    vector<Mat> result;
    int target_c = 0;
    namedWindow("Adjust");
    namedWindow("show");
    int width = 1280,height = 800; // init window size
    int UPBoundary = 0;
    int windowsize = 100;
    int roiwidth = width/3*28/37 ,roiheight = height/3*16/22;
    int xbias = (width/3 - roiwidth)/2, ybias = (height/3 - roiheight)/2;
    int changetime = 200;
    createTrackbar("width", "Adjust", &roiwidth, width/3 - 30);
    createTrackbar("height", "Adjust", &roiheight, height/3 - 30);
    createTrackbar("windowsize", "Adjust",&windowsize, 100);
    createTrackbar("UPBoundary", "Adjust", &UPBoundary, 200);
    createTrackbar("timedelay", "Adjust", &changetime, 1000);
    int mode = 0; // 0 old 1 new
    Mat final,init;
    while(1){
        time0 = static_cast<double>(getTickCount());
        init = Mat::zeros(height, width, CV_8UC1);
        if(time_total >= 1.5 && mode == 0){
            result.clear();
            time_total = 0;
            old_position = position;
            do{
                position = uniform_intx(1, 9);
            }while(position == old_position);
            result = Old_Result(target_c,position,normal_path,target_path);
        }
        
        if(time_total >= 1.5 && mode == 1){
            if(time_total <= 1.5+changetime/1000.0){
                for(int i=0;i<9;i++){
                    result[i] = Mat::zeros(roiheight, roiwidth, CV_8UC1)+255;
                }
            }else{
                time_total = 0;
                result.clear();
                result = New_result(number_path);
            }
        }
        if(mode == 0){
            if(target_total > target_delay){
                target_total = 0;
                if(target_c < 9){
                    target_c++;
                }else{
                    target_c = 1;
                }
                ostringstream nm;
                nm<<target_path<<target_c<<".png";
                result[position-1] = imread(nm.str(),CV_LOAD_IMAGE_GRAYSCALE);
            }
        }
        xbias = (width/3 - roiwidth)/2;
        ybias = (height/3 - roiheight)/2;
        
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                Rect rect(xbias+width/3*j,ybias+height/3*i,roiwidth,roiheight);
                Mat roi = init(rect);
                Mat result_pic = result[3*i+j];
                resize(result_pic, result_pic, Size(roiwidth,roiheight));
                /*if(mode == 1){
                    threshold(result_pic, result_pic, 100, 255, THRESH_BINARY_INV);
                }*/
                result_pic.copyTo(roi);
            }
        }
        if(windowsize != 100 || UPBoundary != 0){
            Mat plusblack = Mat::zeros(height*windowsize/100 + UPBoundary, width*windowsize/100, CV_8UC1);
            resize(init, final, Size(width*windowsize/100,height*windowsize/100));
            Rect finalrect(0,UPBoundary,width*windowsize/100,height*windowsize/100);
            Mat roi = plusblack(finalrect);
            final.copyTo(roi);
            final = plusblack.clone();
        }else{
            final = init.clone();
        }
        
        imshow("show", final);
        key = waitKey(20);
        
        time0 = ((double)getTickCount() - time0) / getTickFrequency();
        time_total += time0;
        target_total += time0;
        
        if(key == 'q') break;
        if(key == 'e') {
            mode = !mode;
            time_total = 1.6;
        }
    }
    return 0;
}
