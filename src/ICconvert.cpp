#include <ini.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
// include the container from C++ STL
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
// boost part for string split
#include <highfive/H5File.hpp>
// header only library for hdf5 IO
using namespace std;
using namespace boost;
using namespace HighFive;

// define the memory check function
void check_alloc_success(void* pointer)
{
    if (pointer == NULL) {
        cerr << "Out of memory!" << endl;
        exit(101);
    }
}

// define the ICfile class
class ICfile {
public:
    ICfile(string in_dir);  // constructor with a string of the dir of file
    ~ICfile();
    string get_format() { return this->format; }
    void show_comps();  // the interface to show the components in the IC
    void text_to_hdf5();

protected:
    int ncomp = 0;  // the number of component in the IC file
    // INI part
    string INI;  // the INI parameter file
    // Input part
    vector<string> infile;  // the filename of the input IC file(s)
    string in_dir;          // the directory of the input IC file(s)
    string format;          // the file format of the input IC file(s)
    vector<string> comps;   // components in the IC file(s)
    vector<int> partnums;   // particle numbers of different components
    // Output part
    string out_dir;  // the directory to hold the output IC file
    string outfile;  // the filename of the output IC file

    bool has_gas = false;        // wheter there is gas component in IC, daulft
    bool has_disk = false;       // similar but for disk
    bool has_halo = false;       // similar but for halo
    bool has_bulge = false;      // similar but for bulge
    bool has_perturber = false;  // similar but for perturber

private:
    void read_text(const char* textfile);  // the function to read in text IC
    void
    has_comps();  // judge whether there are gas, disk, halo, bulge, perturber
    void from_text();
    void to_hdf5();
    // vectors used to hold the coordinates, velocities and masses of the whole
    // IC
    vector<vector<double>> comps_coordinates;
    vector<vector<double>> comps_velocities;
    vector<double> comps_masses;
    // to get the individual info of each component, using partnums to split
    // them
};

// constructor
ICfile ::ICfile(string ini_dir)
{
    cout << "Object <ICfile> [Initial condition file] is being created with :"
         << ini_dir << endl;
    this->INI = ini_dir;
    mINI::INIFile file(this->INI.c_str());  // create the instance of the file
    mINI::INIStructure inidata;  // creat the ini structure to hold data
    bool readsuccess =
        file.read(inidata);  // read data from <file> into <inidata>

    if (!readsuccess) {
        cerr << "[Error]: File not found: " << this->INI << endl;
    }
    else {
        this->format =
            inidata.get("Input").get("format");  // IC format of input file

        // input directory of ICs
        this->in_dir = inidata.get("Input").get("dir");  // IC input directory
        cout << "Input directory of IC: " << this->in_dir << endl;

        // filenames of input ICs
        split(this->infile, inidata.get("input").get("files"),
              is_any_of(" ,;*-+."), token_compress_on);
        cout << "IC files: ";
        for (vector<string>::iterator it = this->infile.begin();
             it != this->infile.end(); it++) {
            cout << *it;
            if (it != this->infile.end() - 1) cout << ", ";
        }
        cout << endl;

        // components of input ICs
        split(this->comps, inidata.get("input").get("comps"),
              is_any_of(" ,;*-+."), token_compress_on);
        this->has_comps();  // judege whether there is certain component
        this->ncomp = this->comps.size();
        cout << "There are " << this->ncomp << " components: ";
        for (vector<string>::iterator it = this->comps.begin();
             it != this->comps.end(); it++) {
            cout << *it;
            if (it != this->comps.end() - 1) cout << ", ";
        }
        cout << endl;

        // particle numbers of input ICs
        vector<string> temp;
        split(temp, inidata.get("input").get("particles"), is_any_of(" ,;*-+."),
              token_compress_on);
        cout << "Particle numbers of particles: ";
        for (vector<string>::iterator it = temp.begin(); it != temp.end();
             it++) {
            cout << *it;
            if (it != temp.end() - 1) cout << ", ";
            this->partnums.push_back(stol(*it));
        }
        cout << endl;

        // check whether the amount of particle numbers is consistent with
        // components
        if (this->comps.size() != this->partnums.size()) {
            cout << "[Error]: the number of particle numbers and components "
                    "are inconsistent with each other!"
                 << endl
                 << "[Erorr]: " << this->partnums.size()
                 << " particles numbers are given, "
                 << "but "
                 << "there are " << this->ncomp << " components!" << endl;
            exit(102);
        }

        // output directory of ICs
        this->out_dir =
            inidata.get("Output").get("dir");  // IC output directory
        cout << "Output directory of converted IC: " << this->out_dir << endl;

        // output IC filename
        this->outfile = inidata.get("Output").get("file");
        cout << "Output IC filename: " << this->outfile << endl;
    }
}

// destructor
ICfile ::~ICfile() { cout << "Object <ICfile> is being cleaned." << endl; }

// judge whether there is certain component in the IC
void ICfile ::has_comps()
{
    string targets[] = {"gas", "halo", "disk", "bulge", "perturber"};

    for (vector<string>::iterator it = this->comps.begin();
         it != this->comps.end(); it++) {
        transform((*it).begin(), (*it).end(), (*it).begin(), ::tolower);
        // transform all string to lowercase, for convience of substring find
        for (int i = 0; i < 5; i++) {
            if ((*it).find(targets[i]) != string::npos) {
                switch (i) {
                    // if has components, set the <has_xxx> as <true>
                    case 0:
                        this->has_gas = true;
                        break;
                    case 1:
                        this->has_halo = true;
                        break;
                    case 2:
                        this->has_disk = true;
                        break;
                    case 3:
                        this->has_bulge = true;
                        break;
                    case 4:
                        this->has_perturber = true;
                        break;
                }
            }
        }
    }
}

void ICfile ::show_comps()
{
    if (this->has_gas) cout << "The IC has gas component." << endl;
    if (this->has_halo) cout << "The IC has halo component." << endl;
    if (this->has_disk) cout << "The IC has disk component." << endl;
    if (this->has_bulge) cout << "The IC has bulge component." << endl;
    if (this->has_perturber) cout << "The IC has perturber component." << endl;
}

void ICfile ::from_text()
{
    // text reading part
    stringstream ss;  // stringstream to read one line
    string cur;       // temprory string to hold data(single)
    char temp_str[300];
    for (int i = 0; i < this->ncomp; i++) {
        ifstream fin;
        string file = this->in_dir + "/" + this->infile.at(i);
        fin.open(file, ios::in);
        // whether seccussfuly open file
        if (!fin.is_open()) {
            cerr << "File not found." << endl;
            exit(100);
        }
        fin.getline(temp_str, sizeof(temp_str));  // ignore header line
        for (int j = 0; j < (int)this->partnums.at(i); j++) {
            fin.getline(temp_str, sizeof(temp_str));  // ignore header line
            vector<double>
                cur_coor_vector;  // cooridates 3by1 vector of this line
            ss.str(temp_str);     // set the stringstream
            for (int k = 0; k < 3; k++) {
                ss >> cur;
                cur_coor_vector.push_back(stold(cur)); /* push at the end,
                stold: string to long double*/
            }
            this->comps_coordinates.push_back(
                cur_coor_vector);  // push at the end

            vector<double> cur_vel_vector;  // similar but for velcoties
            for (int k = 0; k < 3; k++) {
                ss >> cur;
                cur_vel_vector.push_back(stold(cur));  // long double again
            }
            this->comps_velocities.push_back(cur_vel_vector);
            ss >> cur;
            this->comps_masses.push_back(stold(cur));
            ss.clear();  // clear the stringstream
        }
        fin.close();
    }
}

void ICfile ::to_hdf5()
{
    unsigned long int id = 1;  // id for PartcleIDs

    string output_file = this->out_dir + "/" + this->outfile + ".hdf5";

    File ic_h5(output_file, File::Truncate);  // open hdf5 file
    int cur_position = 0; /* the index used to split the continuous vector into
      vectors of each component */
    for (int i = 0; i < this->ncomp; i++) {
        string group_name;  // group name of each component, keep the Gadget
                            // convention PartTypeN
        if (!this->has_gas) {
            group_name = "PartType" + to_string(i + 1);
        }
        else {
            group_name = "PartType" + to_string(i);
        }  // if has gas, start from PartType0, otherwise start from PartType1

        vector<vector<double>>::iterator First;
        vector<vector<double>>::iterator End;

        First = this->comps_coordinates.begin() +
                (int)cur_position;  // begin position
        End = this->comps_coordinates.begin() + (int)cur_position +
              (int)(this->partnums.at(i));               // end position
        vector<vector<double>> Coordinates(First, End);  // assign value

        First = this->comps_velocities.begin() +
                (int)cur_position;  // similar but for velocities
        End = this->comps_velocities.begin() + (int)cur_position +
              (int)(this->partnums.at(i));
        vector<vector<double>> Velocities(First, End);

        vector<double>::iterator first;  // similar but for masses
        vector<double>::iterator end;
        first = this->comps_masses.begin() + (int)cur_position;
        end = this->comps_masses.begin() + (int)cur_position +
              (int)(this->partnums.at(i));
        vector<double> Masses(first, end);

        DataSet coordinate_writor = ic_h5.createDataSet<double>(
            group_name + "/Coordinates", DataSpace::From(Coordinates));
        // dataset of coordinates
        DataSet velocity_writor = ic_h5.createDataSet<double>(
            group_name + "/Velocities", DataSpace::From(Velocities));
        // dataset of velocities
        coordinate_writor.write(Coordinates);  // write coordinates
        velocity_writor.write(Velocities);     // write velocities
        ic_h5.createDataSet(group_name + "/Masses", Masses);  // write masses

        vector<long unsigned int> ParticleIDs;
        for (int j = 0; j < (int)this->partnums.at(i); j++)
            ParticleIDs.push_back(id++);
        ic_h5.createDataSet(group_name + "/ParticleIDs",
                            ParticleIDs);  // write ParticleIDs

        cur_position += (int)this->partnums.at(i);
    }

    Group header = ic_h5.createGroup("Header");

    string Attr1_partnums = "NumPart_ThisFile", Attr2_parttot = "NumPart_Total",
           Attr3_mass = "MassTable", Attr4_time = "Time",
           Attr5_redshift = "Redshift", Attr6_box = "BoxSize",
           Attr7_numfiles = "NumFilesPerSnapshot";

    vector<int> particle_numbers;
    for (vector<int>::iterator it = this->partnums.begin();
         it != this->partnums.end(); it++)
        particle_numbers.push_back(*it);
    if (!this->has_gas) particle_numbers.insert(particle_numbers.begin(), 0);
    Attribute Attr1 = header.createAttribute<unsigned int>(
        Attr1_partnums, DataSpace::From(particle_numbers));
    Attr1.write(particle_numbers);

    vector<int> particle_tot;
    for (vector<int>::iterator it = this->partnums.begin();
         it != this->partnums.end(); it++)
        particle_tot.push_back(*it);
    if (!this->has_gas) particle_tot.insert(particle_tot.begin(), 0);
    Attribute Attr2 = header.createAttribute<unsigned long int>(
        Attr2_parttot, DataSpace::From(particle_tot));
    Attr2.write(particle_tot);

    vector<double> Masses(this->ncomp, 0);
    if (!this->has_gas) Masses.insert(Masses.begin(), 0.0);
    Attribute Attr3 =
        header.createAttribute<double>(Attr3_mass, DataSpace::From(Masses));
    Attr3.write(Masses);

    vector<double> Time = {0.0};
    Attribute Attr4 = header.createAttribute(Attr4_time, Time);
    Attr4.write(Time);

    vector<double> RedShift = {0.0};
    Attribute Attr5 = header.createAttribute(Attr5_redshift, RedShift);
    Attr5.write(RedShift);

    vector<double> BoxSize = {0.0};
    Attribute Attr6 = header.createAttribute(Attr6_box, BoxSize);
    Attr6.write(BoxSize);

    vector<int> FilesPerSnap = {0};
    Attribute Attr7 = header.createAttribute(Attr7_numfiles, FilesPerSnap);
    Attr7.write(FilesPerSnap);

    if (!this->has_gas)
        ic_h5.createGroup(
            "PartType0");  // add in PartType0 if there is no gas in IC
}

void ICfile ::text_to_hdf5()
{
    this->from_text();
    this->to_hdf5();
}

int main(int argc, char* argv[])
{
    string file;
    if (argc != 2) {
        cerr << "Executable complaints: ICconvert require one and only one "
                "parameter, which specify the ini files to be used."
             << endl;
    }
    else {
        file = argv[1];
    }

    ICfile agama_ic(file);
    agama_ic.text_to_hdf5();

    return 0;
}
