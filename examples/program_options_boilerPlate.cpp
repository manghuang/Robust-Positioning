#include <iostream>
#include <fstream> //Input stream from file
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <opencv2/opencv.hpp>

/*
--------------Boilerplate program for boost::program_options-------------------
-Options defined in one or more options_description objects. Possibly with init values
-Options passed either on command line or with configuration file
-Priority:  1. Command line
            2. Configuration file
            3. [if defined] default value

All options are to be defined in every program to keep relevance



Compilation command:
g++ -std=c++11 -fvisibility=hidden /usr/local/lib/libboost_program_options.a `pkg-config --cflags --libs opencv` examples/program_options_boilerPlate.cpp -o bin/example

########### Example configuration file ###########
######### Initial values:
XYZ_INIT =[0,0,-1.8]                    #Initial position expressed as [X,Y,Z] coordinate floats
ROLL_INIT = 0                           #Initial roll of UAV, radians
PITCH_INIT = 0                          #Initial pitch of UAV, radians
YAW_INIT =0                             #Initial yaw of UAV, radians

######## Program settings:
OUT = outFile.csv                       #Write output data to specified file. No output is not set
TILT_COLUMNS =[4,3]                     #Specifies which columns of csv file that contains [roll,pitch] data (0-indexed)
DIST_COLUMN =2                          #Specifies which column of csv file that contains distance (lidar) data
PATH_TO_ARUCO_DATABASE = anchors.csv     #Path to anchor database from base path

*/


/*
    Method that takes a string, and converts it to a opencv mat_<float>
    Possibly overload this with int versions
    Matrices given as string in matlab style using ',' as column separator, ';' as row separator
*/
int string2CVMat(std::string str0, cv::Mat_<float>& M){
    boost::trim_if(str0,boost::is_any_of("[]"));//Trim brackets
    std::vector<std::string> SplitVec;
    boost::split(SplitVec, str0, boost::is_any_of(";"));//Split into rows
    int rows = SplitVec.size();

    std::vector<float> V;
    int cols = -1;
    for(std::string rowStr:SplitVec){//
        std::vector<std::string> row;//Split row string to string elements
        boost::split(row, rowStr, boost::is_any_of(","));//Must have ',' as column delimiter
        cols = row.size();//Will be redefined for every row but must always be same so whatever
        for(std::string i:row){
            std::string elementSTR = boost::trim_copy(i);
            float element;
            try{
                element = std::stof(elementSTR);
            }catch(...){
                std::cerr << "ERROR: could not convert element '" << elementSTR <<"' to float." << std::endl;
                std::cerr << "In string2CVMat" << std::endl;
                return 1;
            }
            V.push_back(element);//remove whitespaces, convert to float and push back
        }
    }
    cv::Mat_<float> V2;
    try{
        V2 = cv::Mat(V).reshape(cols);
    }catch(...){
        std::cerr << "ERROR: Specified matrix does not have consistent number of columns" << std::endl;
        std::cerr << "In string2CVMat" << std::endl;
        throw(1);
    }
    try{
        V2.copyTo(M); //Do this inside another try block to catch specific error
        return 0;
    }catch(...){
        std::cerr << "ERROR: Could not copy parsed matrix onto inputoutput cv mat." << std::endl;
        std::cerr << "In string2CVMat" << std::endl;
        throw(1);
    }
return 1;
}


/*
    Extracts the base path including last '/' from a given filepath. If just filename is given it returns
    an empty string
*/
std::string basePathFromFilePath(std::string str){
    std::size_t found = str.find_last_of("/");
    if(found==str.npos){
        return "";
    }else{
        return str.substr(0,found+1);
    }
}
/*
    This function is to specify all options. Unique for all programs.
*/
int readCommandLine(int argc, char** argv,boost::program_options::variables_map& vm){

    // Declare a group of options that will be
    // allowed only on command line
    namespace po=boost::program_options;
    po::options_description generic("Command line options");
    generic.add_options()
        ("help,h", "produce help message")
        ("file,f",po::value<std::string>(),"configuration file")// Possibly set this as first positional option?
        ("BASE_PATH,p",po::value<std::string>(),"Base path from which I/O paths are relative. Defaults to pwd but may be overridden with this flag.\nGive as either absolute or relative path.")
    ;

    po::options_description parameters("Parameters");
    parameters.add_options()
        //Parameters
        ("RES_XY",  po::value<std::string>(), "Camera resolution in X and Y direction")
        ("K_MAT",  po::value<std::string>(), "Camera K matrix specified in matlab style. ',' as column separator and ';' as row separator") //Tänk om man kan definiera denna direkt som en opencv mat och ge 9 argument på rad?
        ("T_MAT",  po::value<std::string>()->default_value("[0,1,0;-1,0,0;0,0,1]"), "UAV - camera T matrix specified as float numbers row by row separated by whitespace")
        ("CAMERA_BARREL_DISTORTION",    po::value<std::string>()->default_value("[0.2486857357354474,-1.452670730319596,2.638858641887943]"), "Barrel distortion coefficients given as [K1,K2,K3]")
        ("OPTICAL_FLOW_GRID",           po::value<int>()->default_value(4),"Sqrt of number of optical flow vectors")//Single int
        ("ROI_SIZE",po::value<int>()->default_value(150), "Side length of VO ROI. Used to edit K mat of VO alg.")
        ;

    po::options_description initValues("Initial values");
    initValues.add_options()
        ("XYZ_INIT",                    po::value<std::string>()->default_value("[0,0,-1.8]"), "Initial position expressed as [X,Y,Z] coordinate floats")
        ("ROLL_INIT", po::value<float>()->default_value(0),"Initial roll of UAV, radians")
        ("PITCH_INIT", po::value<float>()->default_value(0),"Initial pitch of UAV, radians")
        ("YAW_INIT", po::value<float>()->default_value(0),"Initial yaw of UAV, radians")
        ;
    po::options_description modes("Program settings");
    modes.add_options()
        ("OUT,o",   po::value<std::string>()->default_value("[9,9,9]"), "Write output data to specified file. No output is not set")// Single string argument
        ("TILT_COLUMNS", po::value<std::string>()->default_value("[4,3]"),"Specifies which columns of csv file that contains [roll,pitch] data (0-indexed)")
        ("DIST_COLUMN", po::value<int>()->default_value(1),  "Specifies which column of csv file that contains distance (lidar) data")
        ("PATH_TO_ARUCO_DATABASE", po::value<std::string>()->default_value("anchors.csv"),"Path to anchor database from base path")
        ;
    po::options_description hidden("Hidden settings");
    hidden.add_options()
        ("BASE_PATH", po::value<std::string>()->default_value(""),"base path")
        ;


/*
    How to handle paths? Some functionalities we want:

    Give a settings file for each
*/








    // Parse command line
    po::options_description all("All options");
    all.add(generic).add(parameters).add(initValues).add(modes);
    po::store(po::parse_command_line(argc, argv, all), vm);//Read command line
    po::notify(vm);
    /*Produce help message */
    if (vm.count("help")) {
        std::cout << generic<< std::endl;
        std::cout << "All parameters below are to be defined in a configuration file specified with flag -f" << std::endl;
        std::cout << "Format: \nPARAMETER_FLAG_1 = <value>   #Disregarded comment\nPARAMETER_FLAG_2 = <value>   #Some other comment" << std::endl;
        std::cout << parameters<< std::endl;
        std::cout << initValues<< std::endl;
        std::cout << modes << std::endl;
        std::cout << "---------------" << std::endl;
        return 0;
    }
    /*Read settings from file if specified*/
    if(vm.count("file")){
        std::string iniFile = vm["file"].as<std::string>();
        std::cout << "Reading configuration file " << iniFile << "..." << std::endl;
        std::ifstream ini_file(iniFile);//Try catch block?
        po::store(po::parse_config_file(ini_file, all, true), vm);
        po::notify(vm);



        basePathFromFilePath(iniFile);
    }



/*

    po::store(po::parse_command_line(argc, argv, parameters), vm);
    po::notify(vm);

    po::store(po::parse_command_line(argc, argv, initValues), vm);
    po::notify(vm);
    po::store(po::parse_command_line(argc, argv, modes), vm);
    po::notify(vm);
std::cout << "Do we need to notify(vm) after every store or just once?" << std::endl;

*/

/*    if(vm.count("K_MAT")){
        std::cout<< "Read K mat as string: " << vm["K_MAT"].as<std::string>() << std::endl;
        cv::Mat_<float> M;
        string2CVMat(vm["K_MAT"].as<std::string>(), M);
        std::cout << "K mat as cv float mat:\n" << M << std::endl;


    }*/
    return 0;
}

/*
INI file of following syntax


ROLL_COLUMN=4 #Works
OUT=testi 1 2 4     #Works
ROLL_INIT=1.2   #Works



Will add string2Mat which will allow:
K_MAT = [1,2,3;1,2,4;1,2.5,3]
*/


//https://www.pyimagesearch.com/2015/04/27/installing-boost-and-boost-python-on-osx-with-homebrew/
int main(int argc, char** argv)
{

    boost::program_options::variables_map vm;

    readCommandLine(argc, argv,vm);

    std::cout << "Checking program options..." << std::endl;
    if(vm.count("K_MAT")){
        std::cout<< "Read K mat as string: " << vm["K_MAT"].as<std::string>() << std::endl;
        cv::Mat_<float> M;
        string2CVMat(vm["K_MAT"].as<std::string>(), M);
        std::cout << "K mat as cv float mat:\n" << M << std::endl;
    }
    std::cout << "Checking init values..." << std::endl;
    if(vm.count("XYZ_INIT")){
        std::cout<< "XYZ init: " << vm["XYZ_INIT"].as<std::string>() << std::endl;
    }
    std::cout << "Checking dist column..." << std::endl;
    if(vm.count("DIST_COLUMN")){
        std::cout<< "DIST_COLUMN =  " << vm["DIST_COLUMN"].as<int>() << std::endl;
    }






}

//g++ -std=c++11 -fvisibility=hidden /usr/local/lib/libboost_program_options.a examples/boost_compile_test.cpp -o bin/example