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
//#include "geometry_msgs/Vector2.h"

//try:
//#include <sensor_msgs/ChannelFloat32>


static const std::string OPENCV_WINDOW = "Id window";
static const std::string depth_wind = "depth window";

class ImageConverter
{
  ros::NodeHandle nh_;

  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Subscriber depth_sub_;
  image_transport::Publisher image_pub_;

  ros::Subscriber gettarget_;
  //ros::Publisher Target_pub_;
  ros::Publisher pub_center;
  //ros::Vector2 target;
  cv::Point2i center;


public:
  ImageConverter()
    : it_(nh_)
  {
    // Subscrive to input video feed and publish output video feed
    
    center=cv::Point2i (0,0);
    
    image_sub_ = it_.subscribe("/camera/depth/image_raw", 1,
      &ImageConverter::imageCb,this);
    gettarget_=nh_.subscribe("center_of_object", 10,
      &ImageConverter::targetCb,this);
    // image_sub_ = it_.subscribe("/camera/depth/image_raw", 1,
    //   &ImageConverter::depthCb, this);
    image_pub_ = it_.advertise("/image_converter/depth_video",1);

    //pub_center = nh_.advertise<std_msgs::Int32MultiArray>("center_of_object", 1);
    //Target_pub_= nh_.advertise<geometry_msgs::Int32MultiArray>("center_of_object", 1);

    cv::namedWindow(OPENCV_WINDOW);
    // cv::namedWindow(depth_wind);
  }

  ~ImageConverter()
  {
    cv::destroyWindow(OPENCV_WINDOW);
    // cv::destroyWindow(depth_wind);
  }

  // void depthCb(const sensor_msgs::ImageConstPtr& msg)
  // {
  //   cv_bridge::CvImagePtr cv_ptr;
  //   try
  //   {
  //     cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::TYPE_32UC1);
  //   }
  //   catch (cv_bridge::Exception& e)
  //   {
  //     ROS_ERROR("cv_bridge exception: %s", e.what());
  //     return;
  //   }
  //   cv::imshow(depth_wind, cv_ptr->image);
  //   //cv::imshow(OPENCV_WINDOW, cv_ptr->image);
  //   cv::waitKey(3);    // Output modified video stream

  // }
  void targetCb(const std_msgs::Int32MultiArrayConstPtr& msg)
  {
    ROS_INFO("find target");
    center=cv::Point2i(msg->data[0],msg->data[1]);

  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    ROS_INFO("go to image");
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::TYPE_32FC1);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    // Draw an example circle on the video stream
    // if (cv_ptr->image.rows > 60 && cv_ptr->image.cols > 60)
    //   cv::circle(cv_ptr->image, cv::Point(50, 50), 10, CV_RGB(255,0,0));

    cv::Mat DepthImage=cv_ptr->image;
    
    //cv::Mat croppedDepthImage = DepthImage(Range(10,460), Range(0,600));

    cv::Rect myROI(37, 60, 481, 362);

    // Crop the full image to that image contained by the rectangle myROI
    // Note that this doesn't copy the data
    cv::Mat croppedDepthImage = DepthImage(myROI);




    std_msgs::Int32MultiArray array;

    int rows = croppedDepthImage.rows;
    int cols = croppedDepthImage.cols;

    ROS_INFO("size of image %d, %d",rows,cols);
    



    //pub_center.publish(array);
    cv::Scalar red(255,255,255);


        //target=ros::Vector2(center[i].x,center[i].y);

    //remove noise yo
    //cv::Mat str_el = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
    //morphologyEx(croppedDepthImage, croppedDepthImage, cv::MORPH_OPEN, str_el);
    //morphologyEx(croppedDepthImage, croppedDepthImage, cv::MORPH_CLOSE, str_el);
    

    //color image is in size 640 480

    //cv::Point2i pt(center.x+cols/2,center.y+rows/2);
    //cv::Point2i pt(center.x*cols/640+cols/2,center.y*rows/480+rows/2);
    cv::Point2i pt(center.x*cols/640,center.y*rows/480);
    
    cv::circle(croppedDepthImage, pt, 45, red, 3);


    //ROS_INFO("depth %f",croppedDepthImage(center.x*cols/640+cols/2,center.y*rows/480+rows/2));
    //ROS_INFO("%i", croppedDepthImage.depth());

    // Update GUI Window
    cv::imshow(OPENCV_WINDOW, croppedDepthImage);
    //cv::imshow(OPENCV_WINDOW, cv_ptr->image);
    cv::waitKey(3);    // Output modified video stream


    image_pub_.publish(cv_ptr->toImageMsg());




  }
};

int main(int argc, char** argv)
{
  ros::MultiThreadedSpinner spinner(2); 
  ros::init(argc, argv, "image_depth");
  ImageConverter ic;
  spinner.spin();
  return 0;
}
