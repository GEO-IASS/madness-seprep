/*
  This file is part of MADNESS.
  
  Copyright (C) <2007> <Oak Ridge National Laboratory>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  
  For more information please contact:

  Robert J. Harrison
  Oak Ridge National Laboratory
  One Bethel Valley Road
  P.O. Box 2008, MS-6367

  email: harrisonrj@ornl.gov 
  tel:   865-241-3937
  fax:   865-572-0680

  
  $Id$
*/

/// \file testperiodic.cc
/// \brief test periodic convolutiosn

#define WORLD_INSTANTIATE_STATIC_TEMPLATES  
#include <mra/mra.h>
#include <mra/operator.h>
#include <constants.h>

using namespace madness;

typedef Vector<double,3> coordT;

double source(const coordT& r) {
    return cos(4*2*constants::pi*r[0])*cos(4*2*constants::pi*r[1])*cos(4*2*constants::pi*r[2]);
}

double potential(const coordT& r) {
    return -source(r)/3.0;
}

void test_periodic(World& world) {
    double maple[22] = {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        1.257380016185e-13,
        3.380170823023e-07,
        5.542104317870e-04,
        2.244102656751e-02,
        1.427995147839e-01,
        3.602207596042e-01,
        5.721234392206e-01,
        7.210248654090e-01,
        8.094322295048e-01,
        8.576214176506e-01,
        8.827814139059e-01,
        8.956368674033e-01,
        9.021346272394e-01,
        9.054011635752e-01,
        9.070388644916e-01,
        9.078588254753e-01,
        9.082690838913e-01};

    const long k = 14;
    const double thresh = 1e-12;
    const double L = 0.5;
    FunctionDefaults<3>::set_k(k);
    FunctionDefaults<3>::set_cubic_cell(-L,L);
    FunctionDefaults<3>::set_thresh(thresh);
    
    Function<double,3> f = FunctionFactory<double,3>(world).f(source);
    f.truncate();

    std::vector< SharedPtr< Convolution1D<double> > > ops(1);

    cout.precision(10);
    for (int i=-1; i<=20; i++) {
        double expnt = pow(2.0,double(i));
        double coeff = sqrt(expnt/constants::pi);
        ops[0] = SharedPtr< Convolution1D<double> >(new PeriodicGaussianConvolution1D<double>(k, 16, coeff, expnt));

        SeparatedConvolution<double,3> op(world, k, ops, false, true);

        Function<double,3> opf = apply(op,f);

        coordT r(0.49);

        opf.reconstruct();
        f.reconstruct();

        print(i, expnt, r, f(r), source(r), opf(r), opf(r)-maple[i+1]);

        //plot_line("plot.dat", 101, coordT(-L), coordT(L), f, opf);
    }

    world.gop.fence();

}


int main(int argc, char**argv) {
    MPI::Init(argc, argv);
    World world(MPI::COMM_WORLD);
    
    try {
        startup(world,argc,argv);
        
        test_periodic(world);

    } catch (const MPI::Exception& e) {
        //        print(e);
        error("caught an MPI exception");
    } catch (const madness::MadnessException& e) {
        print(e);
        error("caught a MADNESS exception");
    } catch (const madness::TensorException& e) {
        print(e);
        error("caught a Tensor exception");
    } catch (const char* s) {
        print(s);
        error("caught a c-string exception");
    } catch (char* s) {
        print(s);
        error("caught a c-string exception");
    } catch (const std::string& s) {
        print(s);
        error("caught a string (class) exception");
    } catch (const std::exception& e) {
        print(e.what());
        error("caught an STL exception");
    } catch (...) {
        error("caught unhandled exception");
    }

    world.gop.fence();
    MPI::Finalize();

    return 0;
}
