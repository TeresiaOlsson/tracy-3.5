#define NO 1

#include "tracy_lib.h"

int no_tps = NO;


void err_and_corr(const string &param_file)
{
  bool            cod = false;
  long int        lastpos;
  int             j;
  param_data_type params;
  orb_corr_type   orb_corr[2];
  FILE            *fp;

  // BESSY-III: I_b = 300 mA, h = 538.
  const int 
    Nb          = 538;
  const double
    C           = 350.3,
    Ib          = 300e-3,
    eps[]       = {101e-12, 8e-12},
    sigma_s     = 2.3e-3,
    sigma_delta = 0.90e-3,
    delta_RF    = 5e-2,
    T0          = C/c0,
    Qb          = Ib*T0/Nb;

  const string file_name = "mom_aper.out";

  params.get_param(param_file);

  globval.dPcommon = 1e-10;
  globval.CODeps = 1e-10;

  Ring_GetTwiss(true, 0e0); printglob();

  params.err_and_corr_init(param_file, orb_corr);

  if (params.fe_file != "") params.LoadFieldErr(false, 1e0, true);
  if (params.ae_file != "") {
    // Load misalignments; set seed, no scaling of rms errors.
    params.LoadAlignTol(false, 1e0, true, 1);
    // Beam based alignment.
    if (params.bba) params.Align_BPMs(Quad, -1e0, -1e0, -1e0);

    cod = params.cod_corr(params.n_cell, 1e0, params.h_maxkick,
			  params.v_maxkick, params.n_bits, orb_corr);
  } else
    cod = getcod(0e0, lastpos);

  params.Orb_and_Trim_Stat(orb_corr);

  if (params.N_calls > 0) {
    params.ID_corr(params.N_calls, params.N_steps, false, 1);
    // cod = params.cod_corr(params.n_cell, 1e0, orb_corr);
  }

  prtmfile("flat_file.dat");

  if (cod) {
    printf("\nerr_and_corr: orbit correction completed\n");
    prt_cod("cod.out", globval.bpm, true);
 
    globval.delta_RF = delta_RF; globval.Cavity_on = true;

    Touschek(Qb, globval.delta_RF, eps[X_], eps[Y_], sigma_delta, sigma_s);
      
    double  sum_delta[globval.Cell_nLoc+1][2];
    double  sum2_delta[globval.Cell_nLoc+1][2];
//    double  mean_delta_s[globval.Cell_nLoc+1][2];
//    double  sigma_delta_s[globval.Cell_nLoc+1][2];

    for(j = 0; j <= globval.Cell_nLoc; j++){
      sum_delta[j][X_] = 0e0; sum_delta[j][Y_] = 0e0;
      sum2_delta[j][X_] = 0e0; sum2_delta[j][Y_] = 0e0;
    }
 
    Touschek(Qb, globval.delta_RF, false,
	     eps[X_], eps[Y_], sigma_delta, sigma_s,
	     params.n_track_DA, false, sum_delta, sum2_delta);

    fp = file_write(file_name.c_str()); 
    for(j = 0; j <= globval.Cell_nLoc; j++)
      fprintf(fp, "%4d %7.2f %5.3f %6.3f\n",
	      j, Cell[j].S, 1e2*sum_delta[j][X_], 1e2*sum_delta[j][Y_]);
    fclose(fp);
  } else
    chk_cod(cod, "error_and_correction");

  params.err_and_corr_exit(orb_corr);
}


int main(int argc, char *argv[])
{
  globval.H_exact    = false; globval.quad_fringe    = false;
  globval.Cavity_on  = false; globval.radiation      = false;
  globval.emittance  = false; globval.IBS            = false;
  globval.pathlength = false; globval.Aperture_on    = false;
  globval.Cart_Bend  = false; globval.dip_edge_fudge = true;

  globval.mat_meth = false;

  if (argc < 2) {
    printf("*** bad command line\n");
    exit(1);
  }

  err_and_corr(argv[1]);
}
