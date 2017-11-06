#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

//for publicing center as multiarray: ,... maybe not all needed
#include <stdio.h>
#include <stdlib.h>
#include "std_msgs/MultiArrayLayout.h"
#include "std_msgs/MultiArrayDimension.h"
#include "std_msgs/Int32MultiArray.h"
#include "std_msgs/String.h"

//try:
//#include <sensor_msgs/ChannelFloat32>


//static const std::string OPENCV_WINDOW = "Image window";

//try:
namespace cv
{
    using std::vector;

}

class ImageColor
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  //image_transport::Publisher image_pub_;
  ros::Publisher pub_;
  std_msgs::String  message;


public:
  ImageColor()
    : it_(nh_)
  {
    // Subscrive to input video feed and publish output video feed
     image_sub_ = it_.subscribe("/camera/rgb/image_raw", 1,
       &ImageColor::imageCb, this);
    // image_pub_ = it_.advertise("/image_converter/output_video",1);

    pub_ = nh_.advertise<std_msgs::String>("Object_detction/color", 1);

    // cv::namedWindow(OPENCV_WINDOW);
  }

  ~ImageColor()
  {
    // cv::destroyWindow(OPENCV_WINDOW);
  }

  int getIndexOfLargestElement(std::vector<int>& arr, int size) {
      int largestIndex = 0;
      for (int index = largestIndex; index < size; index++) {
          if (arr[largestIndex] < arr[index]) {
              largestIndex = index;
          }
      }
      return largestIndex;
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    //ROS_INFO("rows = %i, cols= %i", cv_ptr->image.rows, cv_ptr->image.cols);



    // Draw an example circle on the video stream
    //if (cv_ptr->image.rows > 60 && cv_ptr->image.cols > 60)
    //  cv::circle(cv_ptr->image, cv::Point(50, 50), 10, CV_RGB(255,0,0));

    cv::Mat HSVImage;
    cv::Mat ThreshImage;
    cv::vector<cv::vector<cv::Point> > contours;
    cv::vector<cv::Vec4i> heirarchy;
    cv::vector<cv::Point2i> center;
    cv::vector<int> radius;

    bool enableRadiusCulling = 1;
    int minTargetRadius = 50;


    cv::Scalar min;
    cv::Scalar max;
    


    std::vector<cv::Scalar> maxlist;
    std::vector<cv::Scalar> minlist;


    //for blue:
    min= new cv::Scalar(80, 50, 50);
    minlist.push_back(min);
    max= new cv::Scalar(110, 255, 255);
    maxlist.push_back(max);


    // for green

    min= new cv::Scalar(40, 80, 50);
    minlist.push_back(min);
    max= new cv::Scalar(80, 255, 255);
    maxlist.push_back(max);

    //for yellow
    min= new cv::Scalar(20, 100, 50);
    minlist.push_back(min);
    max= new cv::Scalar(40, 255, 255);
    maxlist.push_back(max);




    //Transform the colors into HSV
    cv::cvtColor(cv_ptr->image,HSVImage,CV_BGR2HSV);

    float rads[3]={0,0,0};

    for (int i=1;i<3;i++)
    {
      int rad[i]=findobj(HSVImage,minlist[i],maxlist[i],ThreshImage);
    }
    float32 max=0;
    int order=0;
    for (int i=0;i<3;i++)
    {
      if (rad[i]>max)
      {
        order=i;
        max=rad[i];
      }
    }
    if (max==0) message="N/A";
    else if (order==0) message="Blue";
    else if (order==1) message="Green";
    else if (order==2) message="Yellow";
    pub_.publish(message);



    
    //extract color
    // cv::inRange(HSVImage,min,max,ThreshImage);
    // //remove noise
    // cv::Mat str_el = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
    // morphologyEx(ThreshImage, ThreshImage, cv::MORPH_OPEN, str_el);
    // morphologyEx(ThreshImage, ThreshImage, cv::MORPH_CLOSE, str_el);

    // cv::findContours( ThreshImage.clone(), contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

    // size_t count = contours.size();

    // //center.clear();
    // //radius.clear();
    // for( int i=0; i<count; i++)
    // {
    //     cv::Point2f c;
    //     float r;
    //     cv::minEnclosingCircle( contours[i], c, r);

    //     if (!enableRadiusCulling || r >= minTargetRadius)
    //     {
    //         center.push_back(c);
    //         radius.push_back(r);
    //     }
    // }



    // std_msgs::Int32MultiArray array;
    // size_t count2 = center.size();


    // //ROS_INFO("%zd", count2);


    // if(count2>0){

    //     int x = center[getIndexOfLargestElement(radius, int(radius.size()))].x;
    //     int y = center[getIndexOfLargestElement(radius, int(radius.size()))].y;

    //     // array that contains center of circle / object

    //     array.data.clear();

    //     array.data.push_back(x);
    //     array.data.push_back(y);

    // }
    // else{
    //     array.data.push_back(0);
    //     array.data.push_back(0);
    // }

    // pub_center.publish(array);
    // cv::Scalar red(255,0,0);

    // for( int i = 0; i < count2; i++)
    // {
    //     cv::circle(ThreshImage, center[i], radius[i], red, 3);
    // }


    // // Update GUI Window
    // cv::imshow(OPENCV_WINDOW, ThreshImage);
    // //cv::imshow(OPENCV_WINDOW, cv_ptr->image);
    // cv::waitKey(3);    // Output modified video stream


    //image_pub_.publish(cv_ptr->toImageMsg());

  }

  float32 findobj(cv::Mat &HSVImage, cv::Scalar min, cv::Scalar max,cv::Mat &ThreshImage)
  {
    cv::inRange(HSVImage,min,max,ThreshImage);
    cv::vector<cv::vector<cv::Point> > contours
    cv::vector<cv::Point2i> center;
    cv::vector<int> radius;

    //remove noise
    cv::Mat str_el = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
    morphologyEx(ThreshImage, ThreshImage, cv::MORPH_OPEN, str_el);
    morphologyEx(ThreshImage, ThreshImage, cv::MORPH_CLOSE, str_el);

    cv::findContours( ThreshImage.clone(), contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

    size_t count = contours.size();
    //center.clear();
    //radius.clear();
    for( int i=0; i<count; i++)
    {
        cv::Point2f c;
        float r;
        cv::minEnclosingCircle( contours[i], c, r);

        if (!enableRadiusCulling || r >= minTargetRadius)
        {
            center.push_back(c);
            radius.push_back(r);
        }
    }
    if (count==0) return -1;
    else return radius[0];


  }


};



int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_color detect");
  ImageColor ic;
  ros::spin();
  return 0;
}
