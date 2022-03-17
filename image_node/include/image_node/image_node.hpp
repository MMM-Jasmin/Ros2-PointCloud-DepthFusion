#pragma once
// SYSTEM
#include <chrono>
#include <iostream>
// ROS
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
// OPENCV
#include <opencv2/opencv.hpp>

/**
 * @brief Image viewer node class for receiving and visualizing fused image.
 */
class ImageNode : public rclcpp::Node
{
	typedef std::chrono::high_resolution_clock::time_point time_point;
	typedef std::chrono::high_resolution_clock hires_clock;

public:
	ImageNode();
	void init();

private:
	//topic = "/fused_image";
	// topic = "/color/image_raw";
	std::string m_topic_depth  		= "/camera_left/depth/image";
	std::string m_topic_frameset    = "/camera_left/frameset";
	std::string m_window_name	 	= "Frame";

	

	time_point m_callback_time = hires_clock::now();
	double m_loop_duration = 0.0;

	rclcpp::QoS m_qos_profile = rclcpp::SensorDataQoS();
	//rclcpp::QoS m_qos_profile = rclcpp::SystemDefaultsQoS();

	rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_depth_subscription;
	//rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_frameset_subscription;

	void depthCallback(sensor_msgs::msg::Image::SharedPtr img_msg);
	//void framesetCallback(camera_interfaces::msg::DepthFrameset::UniquePtr fset_msg)
	
};
