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
#include "geometry_msgs/PointStamped.h"
#include "visualization_msgs/Marker.h"//#include "geometry_msgs/Vector2.h"

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
  ros::Publisher pub_target;
  ros::Publisher pub_view;
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

    pub_target = nh_.advertise<geometry_msgs::PointStamped>("/target/camera_frame", 10);
    pub_view = nh_.advertise<visualization_msgs::Marker>("/target/view", 10);
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

  void publish_location(float d, float u, float v)
  {
    float alpha=622.4961970701949;
    float s= 0.0;
    float u0= 332.6030279162762;
    float beta=623.3265166773533;
    float v0=241.27796239580363;
    float z=d;
    float y=((v-v0)*z)/beta;
    float x=(z*(u-u0)-s*y)/alpha;
    geometry_msgs::PointStamped msg;
    msg.header.frame_id="/camera_link";
    msg.header.stamp=ros::Time::now();
    msg.point.x=x/1000;
    msg.point.y=y/1000;
    msg.point.z=z/1000;
    pub_target.publish(msg);
    visualization_msgs::Marker marker;
    marker.header.frame_id = "/camera_link";
    marker.header.stamp    = ros::Time::now();
    marker.ns = "obj";
    marker.id = 0;
    marker.type = 2;
    marker.action = 0;
    marker.pose.position.x=z/1000;
    marker.pose.position.y=y/1000;
    marker.pose.position.z=x/1000;
    marker.pose.orientation.x = 0;
    marker.pose.orientation.y = 0;
    marker.pose.orientation.z = 0;
    marker.pose.orientation.w = 1.0;
    marker.scale.x = 0.05;
    marker.scale.y = 0.05;
    marker.scale.z = 0.05;

    marker.color.r = 0.0;
    marker.color.g = 1.0;
    marker.color.b = 0.0;
    marker.color.a = 1.0;

    marker.lifetime.sec=0;

    pub_view.publish(marker);

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


    cv::Mat DepthImage=cv_ptr->image;
    
    //cv::Mat croppedDepthImage = DepthImage(Range(10,460), Range(0,600));


    int colmin = 0;
    int rowmin = 60;
    int colmax = 460;
    int rowmax = 433;

    // x, y, height, width !!!
    cv::Rect myROI(colmin, rowmin, colmax-colmin, rowmax-rowmin);

    // Crop the full image to that image contained by the rectangle myROI
    // Note that this doesn't copy the data
    cv::Mat croppedDepthImage = DepthImage(myROI);







    int rows = croppedDepthImage.rows;
    int cols = croppedDepthImage.cols;

    //ROS_INFO("size of image %d, %d",rows,cols);
    

    cv::Scalar red(255,255,255);


        //target=ros::Vector2(center[i].x,center[i].y);

    //remove noise yo
    // cv::Mat str_el = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
    // morphologyEx(croppedDepthImage, croppedDepthImage, cv::MORPH_OPEN, str_el);
    // morphologyEx(croppedDepthImage, croppedDepthImage, cv::MORPH_CLOSE, str_el);
    

    //color image is in size 640 480

    //cv::Point2i pt(center.x+cols/2,center.y+rows/2);
    //cv::Point2i pt(center.x*cols/640+cols/2,center.y*rows/480+rows/2);
    if (center.x!=0, center.y!=0)
    {
    cv::Point2i pt(center.x*cols/640,center.y*rows/480);
    ROS_INFO("x = %i, y= %i", pt.x, pt.y);
    //ROS_INFO("rows = %i, cols= %i", rows, cols);


    //The following if statement is needed so that the program doesn't crash in some situations
    if ((pt.x<cols-50) && (pt.y<rows)){

        //std::cout << typeid(pt.x).name() << '\n';
    float depth_value=croppedDepthImage.at<float>(pt.x,pt.y);

    
    if(depth_value!=0)
    {
      ROS_INFO("value %f",depth_value);
      publish_location(depth_value, pt.x/cols*640, pt.y/rows*480);
      //publish_location(depth_value, center.x+320, center.y+240);
    
      int circle_radius = 80;

      cv::circle(croppedDepthImage, pt, circle_radius, red, 3);
    }
    }
  }


    //ROS_INFO("depth %f",croppedDepthImage(center.x*cols/640+cols/2,center.y*rows/480+rows/2));
    //ROS_INFO("%i", croppedDepthImage.depth());

    // Update GUI Window
    cv::imshow(OPENCV_WINDOW, croppedDepthImage);
    //cv::imshow(OPENCV_WINDOW, cv_ptr->image);
    cv::waitKey(3);    // Output modified video stream
    //image_pub_.publish(cv_ptr->toImageMsg());




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
