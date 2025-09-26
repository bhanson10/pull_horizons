#include "../src/pull_horizons.h"

int main(void){

    // Setting Horizons parameters
    char* target_body = "2024 YR4"; // Reminder: strings are case sensitive
    char* ephem_type = "Osculating Orbital Elements"; /* Options:
                                                            - "Observer Table"
                                                            - "Vector Table"
                                                            - "Osculating Orbital Elements"
                                                            - "Small-Body SPK File"
                                                            - "Approach"
                                                      */
    char* center = "500@10"; /* Some common options: 
                                - "@0" or "@SSB"                     = Solar System Barycenter
                                - "500@" or "Geocentric"             = Earth (body center)
                                - "500@10" or "@Sun"                 = Sun (body center)
                                - "500@3" or"@Earth-Moon Barycenter" = Earth-Moon Barycenter
                                - "500@301"                          = Moon (body center)
                             */
    char* ref_plane = "Ecliptic"; /* Options: 
                                        - "Ecliptic"
                                        - "Equatorial"
                                        - "Body Mean"
                                  */ 
    char* units = "KM-S"; /* <length unit>-<time unit>. Options:
                                - "KM-S"
                                - "AU-D"
                                - "KM-D"
                          */
    char* vec_table_set = NULL; /* Only an option if ephem_type = "Vector Table"
                                    - "1" = Position components {x, y, z} only
                                        - Available modifiers:
                                            - "1x" = XYZ uncertainties (ICRF or FK4/B1950)
                                            - "1a" = ACN uncertainties (along-track, cross-track, normal)
                                            - "1r" = RTN uncertainties (radial, transverse, normal)
                                            - "1p" = POS uncertainties (plane-of-sky; radial, RA, and DEC components)
                                            - Note: multiple of these may be combined: "1xa" returns position components along with their uncertainties, and with uncertainties in the ACN system also.
                                    - "2" = State vector {x, y, z, vx, vy, vz}
                                    - Available modifiers:
                                            - "2x" = XYZ uncertainties (ICRF or FK4/B1950)
                                            - "2a" = ACN uncertainties (along-track, cross-track, normal)
                                            - "2r" = RTN uncertainties (radial, transverse, normal)
                                            - "2p" = POS uncertainties (plane-of-sky; radial, RA, and DEC components)
                                            - Note: multiple of these may be combined: "2xarp" returns position and velocity, with uncertainties in all four coordinate systems.
                                    - "3" = State vector + 1-way light-time + range + range-rate
                                    - "4" = Position + 1-way light-time + range + range-rate
                                    - "5" = Velocity components {vx, vy, vz} only
                                    - "6" = 1-way light-time + range + range-rate
                                    if ephem_type != "Vector Table", set vec_table_settings = NULL 
                                */
    char* file_name = "./outputs/rv_YR4_SSB_eq_irregular.txt"; // Output file where Horizons ephemeris is written
    char* t_file_name = "./times/YR4_times.txt"; // Time file containing irregular dates (must be in calendar form)
    int N = 241; // Number of dates in t_file_name
    
    pull_horizons_irreg(target_body, 
                        ephem_type, 
                        center, 
                        ref_plane, 
                        units, 
                        vec_table_set, 
                        file_name, 
                        N,
                        t_file_name); 
    
    return 0;
}
