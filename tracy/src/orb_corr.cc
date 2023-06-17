

std::vector<long int> get_elem(const long int i0, const long int i1,
			       const std::vector<std::string> &names)
{
  long int              loc;
  int                   j, k, Fnum;
  std::vector<long int> elems;

  for (j = 0; j < (int)names.size(); j++) {
    Fnum = ElemIndex(names[j]);
    for (k = 1; k <= GetnKid(Fnum); k++) {
      loc = Elem_GetPos(Fnum, k);
      if ((i0 <= loc) && (loc <= i1)) elems.push_back(loc);
    }
  }

  return elems;
}


void prt_bpm_corr(const int m, const int n, const std::vector<long int> &bpms,
		  const std::vector<long int> &corrs)
{
  int k;

  const int n_prt = 6;

  printf("\nbpms:\n  ");
  for (k = 0; k < m; k++) {
    printf("%8s", Cell[bpms[k]].Elem.PName);
    if ((k+1) % n_prt == 0) printf("\n  ");
  }
  if (m % n_prt != 0) printf("\n");

  printf("\ncorrs:\n  ");
  for (k = 0; k < n; k++) {
    printf("%8s", Cell[corrs[k]].Elem.PName);
    if ((k+1) % n_prt == 0) printf("\n  ");
  }
  if (n % n_prt != 0) printf("\n");
}


void orb_corr_type::alloc(const long int i0, const long int i1,
			  const long int i2,
			  const std::vector<string> &bpm_Fam_names,
			  const std::vector<string> &corr_Fam_names,
			  const bool hor, const bool periodic,
			  const double eps)
{
  this->hor = hor; this->periodic = periodic; this->eps = eps;

  bpms  = get_elem(i0, i2, bpm_Fam_names);
  corrs = get_elem(i0, i1, corr_Fam_names);

  m = bpms.size(); n = corrs.size();

  A = dmatrix(1, m, 1, n); U = dmatrix(1, m, 1, n);
  w = dvector(1, n);       V = dmatrix(1, n, 1, n);
  b = dvector(1, m);       x = dvector(1, n);

  if (!periodic)
    get_trm_mat();
  else
    get_orm_mat();

  svd_decomp();

  printf("\nalloc: plane = %d n_bpm = %d, n_corr = %d\n", hor, m, n);
  prt_bpm_corr(m, n, bpms, corrs);
}


void orb_corr_type::alloc(const std::vector<string> &bpm_Fam_names,
			  const std::vector<string> &corr_Fam_names,
			  const bool hor, const bool periodic,
			  const double eps)
{
  alloc(0, globval.Cell_nLoc, globval.Cell_nLoc, bpm_Fam_names, corr_Fam_names,
	hor, periodic, eps);
}


void orb_corr_type::dealloc(void)
{
  free_dmatrix(A, 1, m, 1, n); free_dmatrix(U, 1, m, 1, n);
  free_dvector(w, 1, n);       free_dmatrix(V, 1, n, 1, n);
  free_dvector(b, 1, m);       free_dvector(x, 1, n);

  printf("\ndealloc: n_bpm = %d, n_corr = %d\n", m, n);
}


void orb_corr_type::get_trm_mat(void)
{
  int      plane, i, j;
  long int loc_bpm, loc_corr;
  double   betai, betaj, nui, nuj;

  plane = (hor)? 0 : 1;

  for (i = 0; i < m; i++) {
    loc_bpm = bpms[i];
    betai = Cell[loc_bpm].Beta[plane]; nui = Cell[loc_bpm].Nu[plane];
    for (j = 0; j < n; j++) {
      loc_corr = corrs[j];
      betaj = Cell[loc_corr].Beta[plane]; nuj = Cell[loc_corr].Nu[plane];
      A[i+1][j+1] = (loc_bpm > loc_corr)?
	sqrt(betai*betaj)*sin(2.0*M_PI*(nui-nuj)) : 0e0;
    }
  }
}


void orb_corr_type::get_orm_mat(void)
{
  int      plane, i, j;
  long int loc_bpm, loc_corr;
  double   nu, betai, betaj, nui, nuj, spiq;

  plane = (hor)? 0 : 1;

  nu = globval.TotalTune[plane]; spiq = sin(M_PI*nu);

  for (i = 0; i < m; i++) {
    loc_bpm = bpms[i];
    betai = Cell[loc_bpm].Beta[plane]; nui = Cell[loc_bpm].Nu[plane];
    for (j = 0; j < n; j++) {
      loc_corr = corrs[j];
      betaj = Cell[loc_corr].Beta[plane]; nuj = Cell[loc_corr].Nu[plane];
      A[i+1][j+1] = 
	sqrt(betai*betaj)/(2.0*spiq)*cos(nu*M_PI-fabs(2.0*M_PI*(nui-nuj)));
    }
  }
}


void orb_corr_type::prt_svdmat(void)
{
  int  plane, i, j;
  FILE *outf;

  plane = (hor)? 0 : 1;

  if (plane == 0)
    outf=fopen("svdh.dat","w");
  else
    outf=fopen("svdv.dat","w");

  fprintf(outf,"# total monitors: %d\n", GetnKid(globval.bpm));
  fprintf(outf,"\n# total available monitors: %d\n", GetnKid(globval.bpm));

  if (plane == 0)
    fprintf(outf,"# total horizontal correctors: %d\n", GetnKid(globval.hcorr));
  else
    fprintf(outf,"# total vertical correctors: %d\n", GetnKid(globval.vcorr));
  fprintf(outf,"\n# total available correctors: %d\n#\n",
	  GetnKid(globval.vcorr));

  fprintf(outf, "#A [%d][%d]= \n",m,n);
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++)
      fprintf(outf, "% .3e ", A[i+1][j+1]);
    fprintf(outf, "\n");
  }

  fprintf(outf, "#U [%d][%d]= \n",m,n);
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++)
      fprintf(outf, "% .3e ", U[i+1][j+1]);
    fprintf(outf, "\n");
  }

  fprintf(outf, "#w [%d]= \n",n);
  for (j = 0; j < n; j++)
    fprintf(outf, "% .3e ", w[j+1]);
  fprintf(outf, "\n#V [%d][%d]= \n",n,n);

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
      fprintf(outf, "% .3e ", V[i+1][j+1]);
    fprintf(outf, "\n");
  }

  fprintf(outf,"#A^-1=V.w.U^T [%d][%d]= \n",n,m);
  for (j = 0; j < n; j++) {
    for (i = 0; i < m; i++)
      fprintf(outf,"% .3e ", Ai[j+1][i+1]);
    fprintf(outf,"\n");
  }

  fclose(outf);
}


void orb_corr_type::svd_decomp(void)
{
  int i, j;

  const int n_prt = 5;

  for (i = 1; i <= m; i++)
    for (j = 1; j <= n; j++)
      U[i][j] = A[i][j];

  dsvdcmp(U, m, n, w, V);

  printf("\ngtcmat singular values:\n");
  for (j = 1; j <= n; j++) {
    printf("%11.3e", w[j]);
    if (w[j] < eps) {
      w[j] = 0e0;
      printf(" (zeroed)");
    }
    if (j % n_prt == 0) printf("\n");
  }
  if (n % n_prt != 0) printf("\n");
}


void orb_corr_type::solve(const double scl) const
{
  int      plane, j;
  long int loc;

  plane = (hor)? 0 : 1;

  for (j = 0; j < m; j++) {
    loc = bpms[j];
    b[j+1] = -Cell[loc].BeamPos[2*plane] + Cell[loc].dS[plane];
  }
      
  dsvbksb(U, w, V, m, n, b, x);

  for (j = 0; j < n; j++) {
    loc = corrs[j];
    if (plane == 0)
      set_dbnL_design_elem(Cell[loc].Fnum, Cell[loc].Knum, Dip,
			   -scl*x[j+1], 0e0);
    else
      set_dbnL_design_elem(Cell[loc].Fnum, Cell[loc].Knum, Dip,
			   0e0, scl*x[j+1]);
  }
}


void orb_corr_type::clr_trims(void)
{
  long int loc;
  int      j;

  for (j = 0; j < n; j++) {
    loc = corrs[j];
    set_bnL_design_elem(Cell[loc].Fnum, Cell[loc].Knum, Dip, 0e0, 0e0);
  }
}


void codstat(double mean[], double sigma[], double xmax[], const long lastpos,
	     const bool all, const std::vector<long int> &bpms)
{
  long    i, n, loc;
  int     j;
  Vector2 sum, sum2;

  for (j = 0; j < 2; j++) {
    sum[j] = 0e0; sum2[j] = 0e0; xmax[j] = 0e0;
  }

  n = 0;
  if (all) {
    for (i = 0; i < lastpos; i++) {
      n++;
      for (j = 0; j < 2; j++) {
	sum[j]  += Cell[i].BeamPos[j*2];
	sum2[j] += sqr(Cell[i].BeamPos[j*2]);
	xmax[j] =  max(xmax[j], fabs(Cell[i].BeamPos[j*2]));
      }
    }
  } else {
    for (i = 0; i < (int)bpms.size(); i++) {
      n++;
      for (j = 0; j < 2; j++) {
	loc = bpms[i];
	sum[j]  += Cell[loc].BeamPos[j*2];
	sum2[j] += sqr(Cell[loc].BeamPos[j*2]);
	xmax[j] =  max(xmax[j], fabs(Cell[loc].BeamPos[j*2]));
      }
    }
  }

  for (j = 0; j < 2; j++) {
    if (n != 0)
      mean[j] = sum[j] / n;
    else
      mean[j] = -1e0;
    if (n != 0 && n != 1) {
      sigma[j] = (n*sum2[j]-sqr(sum[j]))/(n*(n-1e0));
    } else
      sigma[j] = 0e0;
    if (sigma[j] >= 0e0)
      sigma[j] = sqrt(sigma[j]);
    else
      sigma[j] = -1e0;
  }
}


void cod_ini(const std::vector<string> &bpm_Fam_names,
	     const std::vector<string> corr_Fam_names[],
	     orb_corr_type orb_corr[])
{
  int j;

  const double eps = 1e-4;

  for (j = 0; j < 2; j++)
    orb_corr[j].alloc(bpm_Fam_names, corr_Fam_names[j], j == 0, true, eps);

  printf("\ncod_ini:\n");
  printf("  no bpms %lu, no of hor corr. %lu, no of ver. corr. %lu\n",
	 orb_corr[0].bpms.size(), orb_corr[0].corrs.size(),
	 orb_corr[1].corrs.size());
}


void thread_beam(const int n_cell, const string &Fam_name,
		 const std::vector<string> &bpm_Fam_names,
		 const std::vector<string> corr_Fam_names[],
		 const int n_thread, const double scl)
{
  // Thread beam one super period at the time.
  // Assumes a marker at entrance, center, and exit of each super period.

  long int              lastpos, i0, i1;
  int                   i, j, Fnum;
  Vector2               mean, sigma, max;
  ss_vect<double>       ps;
  orb_corr_type         orb_corr[2];
  std::vector<long int> bpms;

  const double eps = 1e-4;

  Fnum = ElemIndex(Fam_name);

  ps.zero(); Cell_Pass(0, globval.Cell_nLoc, ps, lastpos);
  codstat(mean, sigma, max, lastpos, true, orb_corr[X_].bpms);
  printf("\nthread_beam, initial rms trajectory (all):"
	 "   x = %7.1e mm, y = %7.1e mm\n",
	 1e3*sigma[X_], 1e3*sigma[Y_]);

  for (i = 0; i < n_cell; i++) {
    i0 = Elem_GetPos(Fnum, i+1); i1 = Elem_GetPos(Fnum, i+2);
    for (j = 0; j < 2; j++)
      orb_corr[j].alloc(i0, i1, i1, bpm_Fam_names, corr_Fam_names[j],
			j == 0, false, eps);
    if (trace)
      printf("\n  i = %d (%d) i0 = %ld i1 = %ld (s = %5.3f)\n",
	     i+1, n_cell, i0, i1, Cell[i1].S);
    for (j = 0; j < 2; j++) {
      orb_corr[j].corrs = get_elem(i0, i1, corr_Fam_names[j]);
      orb_corr[j].bpms = get_elem(i0, i1, bpm_Fam_names);
    }

    printf("\n");
    for (j = 1; j <= n_thread; j++) {
      ps.zero();
      Cell_Pass(0, i1, ps, lastpos);
      if (trace) {
	cout << setw(3) << j
	     << scientific << setprecision(5) << setw(13) << ps << "\n";
	if (lastpos != i1)
	  printf("thread_beam: n_thread = %2d beam lost at %4ld (%4ld)\n",
		 j, lastpos, globval.Cell_nLoc);
      }
      orb_corr[0].solve(scl); orb_corr[1].solve(scl);
    }

    for (j = 0; j < 2; j++)
      orb_corr[j].dealloc();
  }

  ps.zero(); Cell_Pass(0, globval.Cell_nLoc, ps, lastpos);
  codstat(mean, sigma, max, lastpos, true, orb_corr[X_].bpms);
  printf("\nthread_beam, corrected rms trajectory (all):"
	 " x = %7.1e mm, y = %7.1e mm\n",
	 1e3*sigma[X_], 1e3*sigma[Y_]);
}


bool cod_correct(const int n_orbit, const double scl, orb_corr_type orb_corr[])
{
  bool     cod = false;
  long int lastpos;
  int      j;
  Vector2  mean, sigma, max;

  if (trace) printf("cod_correct:\n");

  for (j = 1; j <= n_orbit; j++) {
    cod = getcod(0e0, lastpos);
    if (cod) {
      if (j == 1) {
	codstat(mean, sigma, max, globval.Cell_nLoc, true, orb_corr[X_].bpms);
	printf("\ncod_correct, initial rms cod (all):"
	       "      x = %7.1e mm, y = %7.1e mm\n",
	       1e3*sigma[X_], 1e3*sigma[Y_]);
	codstat(mean, sigma, max, globval.Cell_nLoc, false, orb_corr[X_].bpms);
	printf("cod_correct, initial rms cod (bpms):"
	       "     x = %7.1e mm, y = %7.1e mm\n",
	       1e3*sigma[X_], 1e3*sigma[Y_]);
      }

      orb_corr[0].solve(scl); orb_corr[1].solve(scl);

      codstat(mean, sigma, max, globval.Cell_nLoc, false, orb_corr[X_].bpms);
      printf("Corrected rms orbit (bpms): x = %7.1e mm, y = %7.1e mm\n",
	     1e3*sigma[X_], 1e3*sigma[Y_]);
    } else
      printf("\ncod_correct: orbit correction failed");
  }

  codstat(mean, sigma, max, globval.Cell_nLoc, true, orb_corr[X_].bpms);
  printf("Corrected rms orbit (all):  x = %7.1e mm, y = %7.1e mm\n",
	 1e3*sigma[X_], 1e3*sigma[Y_]);

  return cod;
}
