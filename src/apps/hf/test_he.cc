#include <mra/mra.h>
#include <iostream>

#include "dft.h"
#include "hartreefock.h"
#include "poperator.h"
#include "util.h"

using std::cout;
using std::endl;

using namespace madness;

const double PI = 3.1415926535897932384;

typedef Vector<double,3> coordT;

/// Returns radius for smoothing nuclear potential with energy precision eprec
//*****************************************************************************
static double smoothing_parameter(double Z, double eprec) {
    // The min is since asymptotic form not so good at low acc.
    // The 2 is from two electrons in 1s closed shell.
    if (Z == 0.0) return 1.0;
    double Z5 = Z*Z*Z*Z*Z;
    double c = pow(std::min(1e-3,eprec)/2.0/0.00435/Z5,1.0/3.0);
    return c;
}
//*****************************************************************************

void printfunc(const World& world, Function<double,3> f, int npts)
{
  Tensor<double> LL = FunctionDefaults<3>::get_cell_width();
  double L = LL[0];
  double bstep = L / npts;
  f.reconstruct();
  for (int i = 0; i <= npts; i++)
  {
    Vector<double,3> p(-L/2 + i * bstep);
    if (world.rank() == 0) printf("%.2f\t\t%.8f\n", p[0], f(p));
  }
  if (world.rank() == 0) printf("\n");
}

/// Regularized 1/r potential.

/// Invoke as \c u(r/c)/c where \c c is the radius of the
/// smoothed volume.
//*****************************************************************************
static double smoothed_potential(double r) {
    const double THREE_SQRTPI = 5.31736155271654808184;
    double r2 = r*r, pot;
    if (r > 6.5){
        pot = 1.0/r;
    } else if (r > 1e-8){
        pot = erf(r)/r + (exp(-r2) + 16.0*exp(-4.0*r2))/(THREE_SQRTPI);
    } else{
        pot = (2.0 + 17.0/3.0)/sqrt(PI);
    }

    return pot;
}
//*****************************************************************************

//*****************************************************************************
static double psi_func_he(const coordT& r)
{
  const double x=r[0], y=r[1], z=r[2];
  return 6.0*exp(-2.0*sqrt(x*x+y*y+z*z)+1e-4);
}
//*****************************************************************************

//*****************************************************************************
static double V_func_he(const coordT& r)
{
  const double x=r[0], y=r[1], z=r[2];
  double rr = sqrt(x*x + y*y + z*z);
  double c = smoothing_parameter(2.0, 1e-7);
  return -2.0 * smoothed_potential(rr/c) / c;
}
//*****************************************************************************

//*****************************************************************************
static double rho_func_he(const coordT& rr)
{
  const double x=rr[0], y=rr[1], z=rr[2];
//  double e1 = 100.0;
//  double coeff = pow(e1/PI, 1.5);
//  return -1.0 * coeff * exp(-e1 * (x*x + y*y + z*z));
  double c = 0.1;
  double r = sqrt(x*x + y*y + z*z);
  r = r / c;
  const double RPITO1P5 = 0.1795871221251665617; // 1.0/Pi^1.5
  return 2.0 * ((-3.0/2.0+(1.0/3.0)*r*r)*exp(-r*r)+(-32.0+(256.0/3.0)*r*r)*exp(-4.0*r*r))*RPITO1P5/c/c/c;
}
//*****************************************************************************

//*****************************************************************************
void test_hf_he(World& world)
{
  cout << "Running test application HartreeFock ..." << endl;

  typedef Vector<double,3> coordT;
  typedef SharedPtr< FunctionFunctorInterface<double,3> > functorT;

  // Dimensions of the bounding box
  double bsize = 22.4;
//  for (int i=0; i<3; i++)
//  {
//    FunctionDefaults<3>::cell(i,0) = -bsize;
//    FunctionDefaults<3>::cell(i,1) = bsize;
//  }
  // Function defaults
  int funck = 8;
  double thresh = 1e-6;
  FunctionDefaults<3>::set_k(funck);
  FunctionDefaults<3>::set_thresh(thresh);
  FunctionDefaults<3>::set_refine(true);
  FunctionDefaults<3>::set_cubic_cell(-bsize, bsize);

  // Nuclear potential (He atom)
  const coordT origin(0.0);
  cout << "Creating Function object for nuclear charge density ..." << endl;
  Function<double,3> rhon = FunctionFactory<double,3>(world).f(rho_func_he);
  Function<double,3> vnuc = FunctionFactory<double,3>(world).f(V_func_he);
  rhon.truncate();
  vnuc.truncate();

  //  if (world.rank() == 0) cout << "Operating on nuclear charge density ..." << endl;
//  SeparatedConvolution<double,3> op = CoulombOperator<double,3>(world, FunctionDefaults<3>::get_k(),
//      1e-8, thresh);
//  Function<double,3> V_from_rho_nuc = apply(op, rhon);
//  if (world.rank() == 0) printf("\n");
//  double L = 2.0 * bsize;
//  double bstep = L / 100.0;
//  vnuc.reconstruct();
//  V_from_rho_nuc.reconstruct();
//  for (int i=0; i<101; i++)
//  {
//    coordT p(-L/2 + i*bstep);
//    double error = fabs(vnuc(p) - V_from_rho_nuc(p));
//    if (world.rank() == 0) printf("%.2f\t\t%.8f\t%.8f\t%.8f\t%.8f\n", p[0], vnuc(p), V_from_rho_nuc(p), error, error / vnuc(p));
//  }
//  if (world.rank() == 0) printf("\n");
//

  // Guess for the wavefunction
  if (world.rank() == 0) cout << "Creating wavefunction psi ..." << endl;
  Function<double,3> psi = FunctionFactory<double,3>(world).f(psi_func_he);
  psi.scale(1.0/psi.norm2());

  // Create lists
  std::vector<Function<double,3> > phis;
  std::vector<double> eigs;
  phis.push_back(psi);
  eigs.push_back(-0.6);

  // Create DFT object
  if (world.rank() == 0) cout << "Creating DFT object ..." << endl;
  DFT<double,3> dftcalc(world, rhon, phis, eigs, thresh, true);
  if (world.rank() == 0) cout << "Running DFT calculation ..." << endl;
//  dftcalc.print_matrix_elements(psi, psi);
  dftcalc.solve(13);
//  printfunc(world, dftcalc.get_phi(0), 100);
//  HartreeFock hf(world, Vnuc, phis, eigs, true, true, thresh);
//  hf.hartree_fock(10);

//  double ke = 2.0 * hf.calculate_tot_ke_sp();
//  double pe = 2.0 * hf.calculate_tot_pe_sp();
//  double ce = hf.calculate_tot_coulomb_energy();
//  double ee = hf.calculate_tot_exchange_energy();
//  printf("Kinetic energy:\t\t\t %.8f\n", ke);
//  printf("Potential energy:\t\t %.8f\n", pe);
//  printf("Two-electron energy:\t\t %.8f\n", 2.0*ce - ee);
//  printf("Total energy:\t\t\t %.8f\n", ke + pe + 2.0*ce - ee);
}
//*****************************************************************************

//*****************************************************************************
void test_he_potential(World& world)
{
  cout << "Running test application HartreeFock ..." << endl;

  typedef Vector<double,3> coordT;
  typedef SharedPtr< FunctionFunctorInterface<double,3> > functorT;

  // Dimensions of the bounding box
  double bsize = 22.4;
//  for (int i=0; i<3; i++)
//  {
//    FunctionDefaults<3>::cell(i,0) = -bsize;
//    FunctionDefaults<3>::cell(i,1) = bsize;
//  }
  // Function defaults
  int funck = 8;
  double thresh = 1e-6;
  FunctionDefaults<3>::set_k(funck);
  FunctionDefaults<3>::set_thresh(thresh);
  FunctionDefaults<3>::set_cubic_cell(-bsize, bsize);

  // Nuclear potential (He atom)
  const coordT origin(0.0);
  cout << "Creating Function object for nuclear charge density ..." << endl;
  Function<double,3> rhon = FunctionFactory<double,3>(world).f(rho_func_he);
  Function<double,3> vnuc0 = FunctionFactory<double,3>(world).f(V_func_he);
  rhon.truncate();
  vnuc0.truncate();

  // Guess for the wavefunction
  cout << "Creating wavefunction psi ..." << endl << endl;
  Function<double,3> psi = FunctionFactory<double,3>(world).f(psi_func_he);
  psi.scale(1.0/psi.norm2());

  cout << "Creating electronic density from psi ..." << endl << endl;
  Function<double,3> rho = square(psi);
  rho.truncate();
  rho.scale(2);

  cout << "Creating nuclear and electronic ops ..." << endl << endl;
  Tensor<double> cellsize = FunctionDefaults<3>::get_cell_width();
  SeparatedConvolution<double,3>* cop = PeriodicCoulombOpPtr<double,3>(world, funck,1e-10, thresh, cellsize);
  SeparatedConvolution<double,3> op = CoulombOperator<double,3>(world, funck,1e-10, thresh);

  cout << "Building potentials ..." << endl << endl;
  Function<double,3> vnuc = apply(op, rhon);
  Function<double,3> velec = apply(op, rho);
  Function<double,3> totalV = vnuc + velec;
  Function<double,3> totalV2 = apply(*cop, rho + rhon);
  // printing out
  double L = 2.0 * bsize;
  double bstep = L / 100.0;
  rho.reconstruct();
  rhon.reconstruct();
  vnuc0.reconstruct();
  vnuc.reconstruct();
  velec.reconstruct();
  totalV.truncate();
  totalV.reconstruct();
  for (int i=0; i<101; i++)
  {
    coordT p(-L/2 + i*bstep);
    printf("%.2f\t\t%.8f\t%.8f\n", p[0], totalV(p), totalV2(p));
  }
  printf("\n");

  cout.precision(8);
  cout << "energy (periodic) is " << inner(totalV2, rho) << endl << endl;
  cout << "energy (non-periodic) is " << inner(totalV, rho) << endl << endl;

  cout << "Trace of rhon is " << rhon.trace() << endl << endl;
  cout << "Trace of rho is " << rho.trace() << endl << endl;
//  cout << "Trace of totalV is " << totalV.trace() << endl << endl;
//  cout << "Trace of totalV2 is " << totalV2.trace() << endl << endl;
//  cout << "value: " << totalV.trace()/L/L/L << endl << endl;
//
//  // matrix elements
//  printf("matrix element for totalV: %.8f\n\n", inner(psi, totalV * psi));
//  printf("matrix element for totalV2: %.8f\n\n", inner(psi, totalV2 * psi));
}
//*****************************************************************************

#define TO_STRING(s) TO_STRING2(s)
#define TO_STRING2(s) #s

//*****************************************************************************
int main(int argc, char** argv)
{
  MPI::Init(argc, argv);
  World world(MPI::COMM_WORLD);
  if (world.rank() == 0)
  {
    print("");
    print("--------------------------------------------");
    print("   MADNESS", " multiresolution testsuite");
    print("--------------------------------------------");
    print("");
    print("   number of processors ...", world.size());
    print("    processor frequency ...", cpu_frequency());
    print("            host system ...", TO_STRING(HOST_SYSTEM));
    print("             byte order ...", TO_STRING(MADNESS_BYTE_ORDER));
    print("          configured by ...", MADNESS_CONFIGURATION_USER);
    print("          configured on ...", MADNESS_CONFIGURATION_HOST);
    print("          configured at ...", MADNESS_CONFIGURATION_DATE);
    print("                    CXX ...", MADNESS_CONFIGURATION_CXX);
    print("               CXXFLAGS ...", MADNESS_CONFIGURATION_CXXFLAGS);
#ifdef WORLD_WATCHDOG
    print("               watchdog ...", WATCHDOG_BARK_INTERVAL,
        WATCHDOG_TIMEOUT);
#endif
#ifdef OPTERON_TUNE
    print("             tuning for ...", "opteron");
#elif defined(CORE_DUO_TUNE)
    print("             tuning for ...", "core duo");
#else
    print("             tuning for ...", "core2");
#endif
#ifdef BOUNDS_CHECKING
    print(" tensor bounds checking ...", "enabled");
#endif
#ifdef TENSOR_INSTANCE_COUNT
    print("  tensor instance count ...", "enabled");
#endif
    print(" ");
  }

  try
  {
    printf("WSTHORNTON: Starting up the world ... \n");

    startup(world,argc,argv);
    if (world.rank() == 0) print("Initial tensor instance count", BaseTensor::get_instance_count());
    test_hf_he(world);
  }
  catch (const MPI::Exception& e)
  {
    //        print(e);
    error("caught an MPI exception");
  }
  catch (const madness::MadnessException& e)
  {
    print(e);
    error("caught a MADNESS exception");
  }
  catch (const madness::TensorException& e)
  {
    print(e);
    error("caught a Tensor exception");
  }
  catch (const char* s)
  {
    print(s);
    error("caught a string exception");
  }
  catch (const std::string& s)
  {
    print(s);
    error("caught a string (class) exception");
  }
  catch (const std::exception& e)
  {
    print(e.what());
    error("caught an STL exception");
  }
  catch (...)
  {
    error("caught unhandled exception");
  }

  if (world.rank() == 0)
    print("entering final fence");
  world.gop.fence();
  if (world.rank() == 0)
    print("done with final fence");
  if (world.rank() == 0)
    print("Final tensor instance count", BaseTensor::get_instance_count());
  MPI::Finalize();

  return 0;
}
//*****************************************************************************
