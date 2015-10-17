#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include "zbar.h"  
#include <iostream>  
using namespace cv;  
using namespace std;  
using namespace zbar;  

const double known_obj_width = 0.155;
const double known_obj_dist  = 1;
const double first_known_pix_width = 88.257744;

double get_focal_length( );
double dist( double pixel_obj_width, double focal_length );

int main(int argc, char* argv[])  
{  

    int camchoice;
    if(argc > 1)
        camchoice = (int)(argv[1][0] - '0');    /*User can set which camera is used*/
    else
        camchoice = 0;  /*Default on Camera Zero (usually built in webcam, or nothing*/

    /*Open the camera*/ 
    VideoCapture cap(camchoice); 


    /*Handle failure*/
    if (!cap.isOpened())
    {  
        cout << "Error opening video camera" << endl;  
        return -1;  
    }  
    
    /*initialize focal length*/
    double f = get_focal_length( );
    printf("focal length = %f\n", f);
    /*Configure the qrscanner*/
    ImageScanner qrscan;   
    qrscan.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);  
    
    /*get width and height of video stream*/ 
    double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); 
    
    /*Print info*/
    cout << "Frame size : " << dWidth << " x " << dHeight << endl;  
    
    /*open display window for video stream*/
    namedWindow("Video Stream",CV_WINDOW_AUTOSIZE);
    
    /*Video loop, break out by pressing ESC*/
    while (1)  
    {
        /*Read in each video frame*/
        Mat frame;  
        bool bSuccess = cap.read(frame); 
        if (!bSuccess)
        {  
            /*break if video stream fails*/
            cout << "Failure to read image from video stream..." << endl;  
            break;  
        }  

        /*convert matrix to gray,
        read in raw values*/
        Mat gray;  
        cvtColor(frame,gray,CV_BGR2GRAY);  
        int width = frame.cols;   
        int height = frame.rows;   
        uchar *raw = (uchar *)gray.data;   
        Image image(width, height, "Y800", raw, width * height);   

        /*scan for barcode*/
        int n = qrscan.scan(image);   
        /*extract results*/
        for(Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) 
        {   
            vector<Point> vp;   
            
            /*Results*/
            //cout << "decoded " << symbol->get_type_name() << " symbol \"" << symbol->get_data() << '"' <<" "<< endl;   
            int n = symbol->get_location_size();   
            for(int i=0;i<n;i++)
            {   
                vp.push_back(Point(symbol->get_location_x(i),symbol->get_location_y(i)));   
            }   
            
            /*draw rectangle around the QR code (this is where the image's pixel
            width is calculated*/
            RotatedRect r = minAreaRect(vp);   
            Point2f pts[4];   
            r.points(pts);   
            
            for(int i=0;i<4;i++)
            {   
                line(frame,pts[i],pts[(i+1)%4],Scalar(255,0,0),3);   
            }   
        
            //cout << (double)r.size.width << endl;
            
            double w = r.size.width;
            


            printf("\033[F");
            printf("QR apparent width: %f\n",w);
            printf("QR distance: %f\n",dist(w,f));
            printf("\r[");
            //cout<<"Angle: "<<r.angle<<endl;   
        }   
        
        /*Display each frame*/
        imshow("Video Stream", frame);
        
        if (waitKey(50) == 27) 
        {  
            /*Break out of the program*/
            cout << "Closing Program..." << endl;  
            break;   
        }  
    }  
    return 0;  
}

double get_focal_length( )
{
    return first_known_pix_width * ( known_obj_dist / known_obj_width );
}

double dist( double pixel_obj_width, double f )
{   
    return known_obj_width  * ( f / pixel_obj_width );
}