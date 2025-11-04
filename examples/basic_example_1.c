#include "../src/pull_horizons.h"

int main(void){

    // Setting Horizons parameters
    char* target_body = "2024 YR4"; // Reminder: strings are case sensitive
    char* ephem_type = "Vector Table"; /* Options:
                                            - "Observer Table"
                                            - "Vector Table"
                                            - "Osculating Orbital Elements"
                                            - "Small-Body SPK File"
                                            - "Approach"
                                        */
    char* center = "@SSB"; /* Some common options: 
                                - "@0" or "@SSB"                     = Solar System Barycenter
                                - "500" or "Geocentric"              = Geocentric
                                - "500@10" or "@Sun"                 = Sun (body center)
                                - "500@3" or"@Earth-Moon Barycenter" = Earth-Moon Barycenter
                                - "500@301"                          = Moon (body center)
                            */
    char* ref_plane = "Equatorial"; /* Options: 
                                        - "Ecliptic"
                                        - "Equatorial"
                                        - "Body Mean"
                                    */ 
    char* start = "2025-05-05 00:00:00"; // Date must appear exactly as it does in Horizons
    char* stop  = "2032-12-23 00:00:00"; // Date must appear exactly as it does in Horizons
    char* step  = "1d"; /* Date abbreviations:
                            - "m"  = minutes
                            - "h"  = hours
                            - "d"  = days
                            - "mo" = months
                            - "y"  = years
                            */
    char* units = "AU-D"; /* <length unit>-<time unit>. Options:
                                - "KM-S"
                                - "AU-D"
                                - "KM-D"
                          */
    char* vec_table_set = "2x"; /* Only an option if ephem_type = "Vector Table"
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
    char* file_name = "./outputs/rv_YR4_SSB_eq_basic.txt"; 

    pull_horizons(target_body, 
                  ephem_type,
                  center, 
                  ref_plane, 
                  start, 
                  stop, 
                  step, 
                  units, 
                  vec_table_set,
                  file_name,
                  0); /* This final parameter writes the entire file, 
                      in general, use 0 here, see irregular_example.c 
                      for more information */

    return 0;
}
