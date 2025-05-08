# pull_horizons

The [JPL Horizons app](https://ssd.jpl.nasa.gov/horizons/app.html#/) is a handy tool for generating the ephemerides of planets, satellites, and spacecraft. One of its downsides is that requests must have regular intervals between epochs. Using the [JPL Horizons API](https://ssd-api.jpl.nasa.gov/doc/horizons.html#vec_table), users can request ephemerides at varying epochs, but doing this manually, especially for large data sets, is cumbersome. **pull\_horizons** uses _curl_ to scrape the JPL Horizons API at irregular calendar dates in an efficient manner, creating ephemerides exactly like the ones provided by the JPL Horizons app, but with the added flexibility of irregular epoch intervals. 

## Installation

To install **pull\_horizons**, first clone the Github repository. In an empty directory, run:

    git clone https://github.com/bhanson10/pull_horizons.git

All else that is required is the command line tool [_curl_](https://curl.se/). Mac users can install _curl_ via _homebrew_:

    brew install curl

If you do not have _homebrew_, documentation for intallation can be found [here](https://docs.brew.sh/Installation). For Windows/Linux users, please refer to the [_curl_ documentation](https://curl.se/download.html). 

Once _curl_ has been installed, the only thing left to do is to update the directory location of _homebrew_ in the examples makefile. In _./examples/makefile_ are the following lines:

    CFLAGS=-pedantic -Wall -std=c99 -D_GNU_SOURCE -fPIC -I<path/to/homebrew/include>
    LDFLAGS=-L<path/to/homebrew/lib>

To ensure the makefile compiles correctly, change the _<path/to/homebrew/include>_ and _<path/to/homebrew/lib>_ to the correct paths. If you are not sure where your _homebrew_ is installed, run:

    brew --prefix

This will mostly likely return _/opt/homebrew_. In this case, you would replace _<path/to/homebrew/include>_ and _<path/to/homebrew/lib>_ with _/opt/homebrew/include_ and _/opt/homebrew/lib_, respectively. Now you are ready to start using **pull\_horizons**!

## Basic example

To begin, we assume all steps from **Installation** have been followed. This example generates the ephemeris of 2024 YR4, an asteroid with a non-negligible probability of hitting the Moon in December 2032. For this example, we pull from the JPL Horizons API the ephemeris of 2024 YR4 from May 5, 2025 to December 23, 2032 with 1 day, regularly space intervals. Almost all of the functionality provided by the JPL Horizons app is available in this script via configuration of the parameters (ephemeris type, center, reference plance, start/stop date, step size, units, etc). 

First, enter into the _examples_ directory:

    cd </path/to/pull_horizons>/examples

To compile and execute the basic YR4 example, run the makefile in basic mode:

    make basic

The generated ephemeris will by default be saved to _"</path/to/pull_horizons>/examples/outputs/rv_YR4_SSB_eq_basic.txt"_ in the exact same form as it would be downloaded off the JPL Horizons app. 

## Irregular example

This irregular example demonstrates the key functionality of **pull_horizons**. Located at _</path/to/pull_horizons>/examples/times_ is a .txt file with irregularly-spaced epochs in calendar form. We are going to call the JPL Horizons API to generate an ephemeris of the same form as would be output by the JPL Horizons app, but with the added flexibility of choosing the exact (and irregular) epochs. 

If you haven't already, enter into the _examples_ directory:

    cd </path/to/pull_horizons>/examples
    
Again, we are using 2024 YR4 as our object of interest. This time, we are outputting the osculating orbital elements of YR4 relative to the Sun. To compile and execute the irregular YR4 example, run the makefile in irregular mode:

    make irreg

This example will take slightly longer than the first, but you should see live the default output _"</path/to/pull_horizons>/examples/outputs/rv_YR4_SSB_eq_irregular.txt"_ being updated as the orbital element of YR4 at each calendar date from _</path/to/pull_horizons>/examples/times/YR4_times.txt_ are generated. The final result is an irregularly-spaced Horizons ephemeris of YR4!

## License
**pull_horizons** is freely distributed under the [BSD 3-Clause License](https://opensource.org/license/bsd-3-clause).
