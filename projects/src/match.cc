#define NO 1

#include "tracy_lib.h"

int no_tps = NO;

// Initial conditions: alpha, beta, eta, etap.
const double ic[][2] =
  {{0.0, 0.0}, {5.34214, 2.00245}, {0.0, 0.0}, {0.0, 0.0}};

int loc[10], n;

double get_bn_s(const int Fnum, const int Knum, const int n);
void set_bn_s(const int Fnum, const int n, const double dbn);
void get_S(void);


struct param_type {
private:

public:
  int                 n_prm;
  double              bn_tol, step;
  std::vector<double> bn_min, bn_max, bn_scl;
  std::vector<int>    Fnum, n;

  void add_prm(const std::string Fname, const int n,
	       const double bn_min, const double bn_max,
	       const double bn_scl);
  void ini_prm(double *bn, double *bn_lim);
  void set_prm(double *bn) const;
  void prt_prm(double *bn) const;
};


param_type b2_prms;


void param_type::add_prm(const std::string Fname, const int n,
			 const double bn_min, const double bn_max,
			 const double bn_scl)
{
  Fnum.push_back(ElemIndex(Fname.c_str()));
  this->n.push_back(n);
  this->bn_min.push_back(bn_min);
  this->bn_max.push_back(bn_max);
  this->bn_scl.push_back(bn_scl);
  n_prm = Fnum.size();
}


void param_type::ini_prm(double *bn, double *bn_lim)
{
  int    i;
  double an;

  n_prm = Fnum.size();
  for (i = 1; i <= n_prm; i++) {
    bn_lim[i] = bn_max[i-1];
    if (n[i-1] > 0)
      // Multipole.
      get_bn_design_elem(Fnum[i-1], 1, n[i-1], bn[i], an);
    else if (n[i-1] == -1)
      // Drift.
      bn[i] = get_L(Fnum[i-1], 1);
    else if (n[i-1] == -2)
      // Placement.
      bn[i] = get_bn_s(-Fnum[i-1], 1, n[i-1]);
  }
}


void param_type::set_prm(double *bn) const
{
  int i, j;

  for (i = 1; i <= n_prm; i++) {
    if (n[i-1] > 0)
      for (j = 1; j <= GetnKid(Fnum[i-1]); j++)
	set_bn_design_elem(Fnum[i-1], j, n[i-1], bn[i], 0e0);
    else if (n[i-1] == -1) {
      set_L(Fnum[i-1], bn[i]); get_S();
    } else if (n[i-1] == -2)
      set_bn_s(-Fnum[i-1], n[i-1], bn[i]);
  }
}


void param_type::prt_prm(double *bn) const
{
  int i;

  const int n_prt = 8;

  for (i = 1; i <= n_prm; i++) {
    printf(" %9.5f", bn[i]);
    if (i % n_prt == 0) printf("\n");
  }
  if (n_prm % n_prt != 0) printf("\n");
}


double get_bn_s(const int Fnum, const int Knum, const int n)
{
  int    k;
  double bn, an;

  if (Fnum > 0)
    get_bn_design_elem(Fnum, Knum, n, bn, an);
  else {
    k = Elem_GetPos(abs(Fnum), Knum);

    switch (Cell[k-1].Elem.PName[1]) {
    case 'u':
      bn = Cell[k-1].Elem.PL;
      break;
    case 'd':
      bn = Cell[k+1].Elem.PL;
      break;
    default:
      printf("get_bn_s: configuration error %s (%d)\n",
	     Cell[k-1].Elem.PName, k);
      exit(1);
      break;
    }
  }

  return bn;
}


void set_bn_s(const int Fnum, const int Knum, const int n, const double bn)
{
  int k;

  if (Fnum > 0)
    set_bn_design_elem(Fnum, Knum, n, bn, 0e0);
  else {
    // point to multipole
    k = Elem_GetPos(abs(Fnum), Knum);

    switch (Cell[k-1].Elem.PName[1]) {
    case 'u':
      if (Cell[k+1].Elem.PName[1] == 'd') {
	set_L(Cell[k-1].Fnum, Cell[k-1].Knum, bn);
	set_L(Cell[k+1].Fnum, Cell[k+1].Knum, -bn);
      } else {
	printf("set_bn_s: configuration error %s (%d)\n",
	       Cell[k+1].Elem.PName, k+2);
	exit(1);
      }
      break;
    case 'd':
      if (Cell[k+1].Elem.PName[1] == 'u') {
	set_L(Cell[k-1].Fnum, Cell[k-1].Knum, -bn);
	set_L(Cell[k+1].Fnum, Cell[k+1].Knum, bn);
      } else {
	printf("set_bn_s: configuration error %s (%d)\n",
	       Cell[k+1].Elem.PName, k+2);
	exit(1);
      }
      break;
    default:
      printf("set_bn_s: configuration error %s (%d)\n",
	     Cell[k-1].Elem.PName, k);
      exit(1);
      break;
    }
  }
}


void set_bn_s(const int Fnum, const int n, const double bn)
{
  int k;

  for (k = 1; k <= GetnKid(abs(Fnum)); k++)
    set_bn_s(Fnum, k, n, bn);
}


void get_S(void)
{
  int    j;
  double S;

  S = 0e0;
  for (j = 0; j <= globval.Cell_nLoc; j++) {
    S += Cell[j].Elem.PL; Cell[j].S = S;
  }
}


void get_dnu(const int i0, const int i1, const double alpha0[],
	     const double beta0[], const double dp, double dnu[])
{
  long int     lastpos;
  int          k;
  double       m11, m12;
  ss_vect<tps> map;

  map.identity(); map[delta_] += dp;
  Cell_Pass(i0+1, i1, map, lastpos);

  for (k = 0; k < 2; k++) {
    m11 = map[2*k][2*k]; m12 = map[2*k][2*k+1];
    dnu[k] = atan(m12/(beta0[k]*m11-alpha0[k]*m12))/(2e0*M_PI);
    if (m11 < 0e0) dnu[k] += (m12 >= 0)? 0.5e0 : -0.5e0;
    if (dnu[k] < 0e0) dnu[k] += 1e0;
  }

  printf("\n%10.8f %10.8f\n", dnu[X_], dnu[Y_]);
}


void get_dnu_dp(const int i0, const int i1, const double alpha0[],
		const double beta0[], const double dp, double dksi[])
{
  // To evaluate linear chromaticity the linear dispersion for the periodic
  // solution must be known.
  int    k;
  double dnu1[2], dnu0[2];

  get_dnu(loc[0], loc[1], alpha0, beta0, dp, dnu1);
  get_dnu(loc[0], loc[1], alpha0, beta0, -dp, dnu0);

  for (k = 0; k < 2; k++)
    dksi[k] = (dnu1[k]-dnu0[k])/(2e0*dp);

  printf("\n%10.8f %10.8f\n", dksi[X_], dksi[Y_]);

  Ring_GetTwiss(true, dp);
  printf("\n%10.8f %10.8f\n", globval.TotalTune[X_], globval.TotalTune[Y_]);
  Ring_GetTwiss(true, -dp);
  printf("%10.8f %10.8f\n", globval.TotalTune[X_], globval.TotalTune[Y_]);
}


void prt_match(const param_type &b2_prms, const double *b2)
{
  FILE   *outf;

  std::string file_name = "match.out";

  outf = file_write(file_name.c_str());

  fprintf(outf, "l5h: drift, l = %7.5f;\n", get_L(ElemIndex("l5h"), 1));
  fprintf(outf, "l6h: drift, l = %7.5f;\n", get_L(ElemIndex("l6h"), 1));
  fprintf(outf, "l7:  drift, l = %7.5f;\n", get_L(ElemIndex("l7"), 1));
  fprintf(outf, "l8:  drift, l = %7.5f;\n", get_L(ElemIndex("l8"), 1));

  fprintf(outf, "\nbm:  bending, l = 0.14559, t = 0.5, k = %9.5f, t1 = 0.0"
	  ", t2 = 0.0,\n     gap = 0.00, N = Nbend, Method = Meth;\n", b2[1]);
  fprintf(outf, "qm:  quadrupole, l = 0.2, k = %9.5f, N = Nquad"
	  ", Method = Meth;\n", b2[4]);
  fprintf(outf, "qfe: quadrupole, l = 0.1, k = %9.5f, N = Nquad"
	  ", Method = Meth;\n", b2[2]);
  fprintf(outf, "qde: quadrupole, l = 0.1, k = %9.5f, N = Nquad"
	  ", Method = Meth;\n", b2[3]);

  fclose(outf);
}


double f_match(double *b2)
{
  static double chi2_ref = 1e30;

  int          i;
  double       chi2;
  ss_vect<tps> Ascr;

  const int n_prt = 50;
  b2_prms.set_prm(b2);

  Ascr = get_A(ic[0], ic[1], ic[2], ic[3]);
  Cell_Twiss(loc[0]+1, loc[4], Ascr, false, false, 0e0);

  chi2 = 0e0;
  // chi2 += sqr(1e5*(Cell[loc[1]].Eta[X_]));
  // chi2 += sqr(1e5*(Cell[loc[1]].Etap[X_]));
  // chi2 += sqr(5e1*(Cell[loc[1]].Beta[Y_]-20e0));

  for (i = 1; i <= b2_prms.n_prm; i++)
    if (fabs(b2[i]) > b2_prms.bn_max[i-1]) chi2 += 1e10;

  if (chi2 < chi2_ref) {
    n++;

    if (n % n_prt == 0) {
      printf("\n%3d chi2: %12.5e -> %12.5e\n", n, chi2_ref, chi2);
      printf("b2s:\n");
      b2_prms.prt_prm(b2);

     // prt_match(b2_prms, b2);

      prtmfile("flat_file.fit");
      prt_lat(loc[0]+1, loc[4], "linlat1.out", globval.bpm, true);
      prt_lat(loc[0]+1, loc[4], "linlat.out", globval.bpm, true, 10);
    }
  }

  chi2_ref = min(chi2, chi2_ref);

  return chi2;
}


void fit_match(param_type &b2_prms)
{
  int          n_b2, i, j, iter;
  double       alpha0[2], beta0[2], dnu[2], *b2, *b2_lim, **xi, fret;
  ss_vect<tps> Ascr;

  n_b2 = b2_prms.n_prm;

  b2 = dvector(1, n_b2); b2_lim = dvector(1, n_b2);
  xi = dmatrix(1, n_b2, 1, n_b2);

  loc[0] = 0;
  loc[1] = globval.Cell_nLoc;

  Ascr = get_A(ic[0], ic[1], ic[2], ic[3]);
  Cell_Twiss(loc[0]+1, loc[1], Ascr, false, false, 0e0);

  prt_lat(loc[0], loc[1], "linlat1.out", globval.bpm, true);
  prt_lat(loc[0], loc[1], "linlat.out", globval.bpm, true, 10);

  for (j = 0; j < 2; j++) {
    alpha0[j] = Cell[loc[0]].Alpha[j]; beta0[j] = Cell[loc[0]].Beta[j];
  }
  get_dnu_dp(loc[0], loc[1], alpha0, beta0, 1e-5, dnu);
  exit(0);

  b2_prms.ini_prm(b2, b2_lim);

  // Set initial directions (unit vectors).
  for (i = 1; i <= n_b2; i++)
    for (j = 1; j <= n_b2; j++)
      xi[i][j] = (i == j)? 1e0 : 0e0;

  n = 0;
  dpowell(b2, xi, n_b2, 1e-16, &iter, &fret, f_match);

  Ascr = get_A(ic[0], ic[1], ic[2], ic[3]);
  Cell_Twiss(loc[0]+1, loc[3], Ascr, false, false, 0e0);

  prt_lat(loc[0]+1, loc[5], "linlat1.out", globval.bpm, true);
  prt_lat(loc[0]+1, loc[5], "linlat.out", globval.bpm, true, 10);

  free_dvector(b2, 1, n_b2);  free_dvector(b2_lim, 1, n_b2);
  free_dmatrix(xi, 1, n_b2, 1, n_b2);
}


int main(int argc, char *argv[])
{
  ss_vect<tps> Ascr;

  globval.H_exact    = false; globval.quad_fringe = false;
  globval.Cavity_on  = false; globval.radiation   = false;
  globval.emittance  = false; globval.IBS         = false;
  globval.pathlength = false; globval.bpm         = 0;

  if (true)
    Read_Lattice(argv[1]);
  else
    rdmfile(argv[1]);

  no_sxt();

  Ring_GetTwiss(true, 0e0); printglob();

  // prt_lat("linlat1.out", globval.bpm, true);
  // prt_lat("linlat.out", globval.bpm, true, 10);

  if (true) {
    // b2_prms.add_prm("q01",  2, 0.0, 25.0, 1.0);
    // b2_prms.add_prm("q02",  2, 0.0, 25.0, 1.0);
    // b2_prms.add_prm("q03",  2, 0.0, 25.0, 1.0);

    b2_prms.bn_tol = 1e-6; b2_prms.step = 1.0;

    fit_match(b2_prms);
  }
}
