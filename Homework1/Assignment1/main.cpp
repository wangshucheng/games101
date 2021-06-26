#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    float a = rotation_angle / 180.0 * MY_PI;
    Eigen::Matrix4f translate;
    translate << std::cos(a), -std::sin(a), 0, 0, 
        std::sin(a), std::cos(a), 0, 0, 
        0, 0, 1, 0, 
        0, 0, 0, 1;
    
    model = translate * model;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    Eigen::Matrix4f persp2ortho;
    persp2ortho << zNear, 0, 0, 0, 
                0, zNear, 0, 0, 
                0, 0, zNear+zFar, -zNear*zFar, 
                0, 0, 1, 0;
    
    float top = tan(eye_fov / 180.0 * MY_PI / 2) * zNear;
    float bottom = -top;
    float right = top * aspect_ratio;
    float left = -right;
    Eigen::Matrix4f ortho_scale, ortho_translate;
    ortho_scale << 2/(right - left), 0, 0, 0,
        0, 2/(top - bottom), 0, 0,
        0, 0, 2/(zNear - zFar), 0,
        0, 0, 0, 1;

    ortho_translate << 1, 0, 0, -(right + left)/2,
        0, 1, 0, -(top + bottom)/2,
        0, 0, 1, -(zFar + zNear)/2,
        0, 0, 0, 1;

    projection = ortho_scale * ortho_translate * persp2ortho;

    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    float a = angle / 180.0 * MY_PI;

    Eigen::Matrix3f tr;
    Eigen::Matrix3f mul;
    
    mul << 0, -axis[2], axis[1],
        axis[2], 0, -axis[0],
        -axis[0], axis[0], 0;
    
    tr = cos(a)*Eigen::Matrix3f::Identity() + (1-cos(a))*axis*axis.adjoint() + mul*sin(a);
    
    model << tr(0,0), tr(0,1), tr(0,2), 0,
            tr(1,0), tr(1,1), tr(1,2), 0,
            tr(2,0), tr(2,1), tr(2,2), 0,
            0, 0, 0, 1;
    
    return model;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
