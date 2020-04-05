#ifndef MARTONROBUST_H
#define MARTONROBUST_H
/*
    This class implements a version of the robust trilateration method presented by L Marton et al (isbn: 9781509025916)
    Adaptation is done to fit the use of angulation instead of lateration
    Adaptions include:
        - Angulation to visible anchor point is used together with a separate height estimation
            to find expression for possible circle in 3d
        - The final position estimation is used together with initial angulation data to estimate yaw angle
*/
#include <stdio.h>
#include <opencv2/opencv.hpp>
// Libraries for gsl nonlinear least squared optimization
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>
#include <math.h>       /* cos */
namespace robustPositioning{
/*

*/
class martonRobust{
        cv::Mat_<float> K;                      //Camera matrix
        cv::Mat_<float> K_inv;                  //Inverted Camera matrix
        cv::Mat_<float> T;                      //uav to camera transformation matrix
        cv::Mat_<float> T_inv;                  //Inverted uav to camera transformation matrix. (=camera to uav in x-y-z order)
        float k1_barrel;                        //Distortion coefficients for barrel distortion
        float k2_barrel;
        float k3_barrel;
        int polyParameters;//How many parameters? ie what degree should the polynomial be?
        int bufferSize; //How many previous locations are to be saved in circular buffer? (min value dep on polyParameters)
    public:
        void setKmat(cv::Mat_<float>);          //Camera matrix
        void setTmat(cv::Mat_<float>);          //uav -to- camera frame transformation matrix
        void setDistortCoefficents(float,float,float);//Set k1,k2,k3 distortion coefficients
        cv::Point2f unDistort(const cv::Point2f&);//Compensate for barrel distortion
        void pix2angles(const std::vector<cv::Point2f>&,std::vector<cv::Mat_<float>>&);//Converts image pixel coordinate(s) to apparent angles in x,y direction and around z
        void pix2angles(const std::vector<std::vector<cv::Point2f>>&,std::vector<cv::Mat_<float>>&);//Overloaded version for corner locations of anchors instead of center location
        void process(void);// template for complete process method. add arguments and return type when clear
    private:
        struct poly2_data; // Data struct for gsl containing n: t: y:
        int poly2_f (const gsl_vector * x, void *data, gsl_vector * f); // Cost function for gsl_multifit_nlinear (2nd order polynomial)
        int poly2_df (const gsl_vector * x, void *data, gsl_matrix * J);// Jacobian of cost function for gsl-multifit_nlinear (2nd order polynomial)
        int nlinear_lsqr(void);     //Perform the nonlinear least square optimization Add arguments when known
    };

}
#endif
