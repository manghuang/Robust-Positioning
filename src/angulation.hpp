#ifndef ANGULATION_H
#define ANGULATION_H

/*
    This class implements a framework for angulation from a database of anchors with known ID:s and 3d locations
    The angulation is calculated in 3d and azimuth, from the AZIPE algorithm. Pixel coordinates of anchors and their ID
    are taken as input. if enough anchors are present, full AZIPE will be calculated.
*/


namespace ang{
    const int NO_ANCHORS = 0;
    const int TOO_FEW_ANCHORS = 1;
    const int AZIPE_FAIL = 2;
    const int AZIPE_SUCCESS = 3;

    class angulation{
        cv::Mat_<float> K;                      //Camera matrix
        cv::Mat_<float> K_inv;                  //Inverted Camera matrix
        cv::Mat_<float> T;                      //uav to camera transformation matrix
        cv::Mat_<float> T_inv;                  //Inverted uav to camera transformation matrix. (=camera to uav in x-y-z order)
        int maxId;                              //Max size of database. Specified upon initiation
        int minAnchors;                         //Minimum amount of anchors to try angulation
        std::vector<bool> activeAnchors;        //Part of database specifying what IDs are known
        std::vector<cv::Mat_<int>> IDs;         //IDs of all known anchors
        std::vector<cv::Mat_<float>> dataBase;  //All known anchors and their coordinates are kept here
        std::vector<std::string> parse(std::string);//Parses a csv line into vector<string>
        int readToDataBase(std::string,std::vector<int>&, std::vector<cv::Mat_<float>>&);//Reads csv file into anchor database
        void pix2uLOS(const std::vector<cv::Point2f>&,std::vector<cv::Mat_<float>>&);//Converts image pixel coordinates to uLOS vectors in uav frame
        int dataBase2q(const std::vector<int>&,std::vector<cv::Mat_<float>>&,std::vector<bool>&);//Retrieves Q vectors from database given the IDs
    public:
        angulation(int,std::string);
        void setKmat(cv::Mat_<float>);          //Camera matrix
        void setTmat(cv::Mat_<float>);          //uav -to- camera frame transformation matrix
        int addAnchor(int,cv::Mat_<float>);     //Add an anchor to database (id and 3d coordinate)
        int calculate(std::vector<cv::Point2f>&, std::vector<int>&,std::vector<bool>&,cv::Mat_<float>&,float&,float,float); //Take pixel coordinates and IDs of anchors and calculate AZIPE
        int calculate(std::vector<std::vector<cv::Point2f>>&, std::vector<int>&,std::vector<bool>&,cv::Mat_<float>&,float&,float,float);// Overloaded. takes mean of pixel group first
    };
}
#endif
