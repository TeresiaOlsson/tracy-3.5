/* Tracy-2

   J. Bengtsson, CBP, LBL      1990 - 1994   Pascal version
                 SLS, PSI      1995 - 1997
   M. Boege      SLS, PSI      1998          C translation
   L. Nadolski   SOLEIL        2002          Link to NAFF, Radia field maps
   J. Bengtsson  NSLS-II, BNL  2004 -        


   To generate a lattice flat file.

   Type codes:

     marker     -1
     drift	 0
     multipole   1
     cavity      2
     thin kick   3
     wiggler     4

   Integration methods:

     linear, matrix style (obsolete)              0
     2nd order symplectic integrator (obsolete)   2
     4th order symplectic integrator              4

   Format:

     name, family no, kid no, element no
     type code, integration method, no of integration steps, reverse
     apertures: xmin, xmax, ymin, ymax

   The following lines follows depending on element type.

     type

     drift:	 L

     multipole:  hor., ver. displ., roll angle (design), roll angle (error)
                 L, 1/rho, entrance angle, exit angle
		 no of nonzero multipole coeff., n design
		 n, b , a
		     n   n
		     .
		     .
		     .

     wiggler:    L [m], Lambda [m]
                 no of harmonics
                 harm no, kxV [1/m], BoBrhoV [1/m], kxH, BoBrhoH, phi
                    ...

     cavity:	 cavity voltage/beam energy [eV], omega/c, beam energy [eV],
                 phi

     thin kick:	 hor., ver. displacement, roll angle (total)
		 no of nonzero multipole coeff.
		 n, b , a
		     n   n
		     .
		     .
		     .

     kick_map:   scale order <file name>

*/


#define snamelen   10

// numerical type codes
#define marker_   -1
#define drift_     0
#define mpole_     1
#define cavity_    2
#define thinkick_  3
#define wiggler_   4
#define kick_map   6
#define map_       7


std::ifstream  inf;

const int line_max = 200;

void get_kind(const int kind, ElemType &Elem)
{
  MpoleType  *M;
  CavityType *C;

  switch (kind) {
  case marker_:
    Elem.Pkind = PartsKind(marker);
    break;
  case drift_:
    Elem.Pkind = PartsKind(drift);
    // Drift_Alloc(Elem);
    break;
  case mpole_:
    Elem.Pkind = PartsKind(Mpole);
    // Mpole_Alloc(Elem);
    M = dynamic_cast<MpoleType*>(&Elem);
    M->Pthick = pthicktype(thick);
    break;
  case cavity_:
    Elem.Pkind = PartsKind(Cavity);
    C = dynamic_cast<CavityType*>(&Elem);
    // Cav_Alloc(C);
    break;
  case thinkick_:
    Elem.Pkind = PartsKind(Mpole);
    // Mpole_Alloc(Elem);
    M = dynamic_cast<MpoleType*>(&Elem);
    M->Pthick = pthicktype(thin);
    break;
  case wiggler_:
    Elem.Pkind = PartsKind(Wigl);
    // Wiggler_Alloc(Elem);
    break;
  case kick_map:
    Elem.Pkind = PartsKind(Insertion);
    // Insertion_Alloc(Elem);
    break;
  case map_:
    Elem.Pkind = PartsKind(Map);
    // Map_Alloc(Elem);
    break;
  default:
    std::cout << "get_kind: unknown type " << kind << " " << Elem.PName
	      << std::endl;
    exit_(1);
    break;
  }
}


void rdmfile(const char *mfile_dat)
{
  const int n_ps = 6;

  char          line[line_max], file_name[line_max];
  int           j, k, nmpole, kind, method, n, reverse;
  long int      i;
  double        dTerror, val[n_ps];
  ss_vect<tps>  Id;
  MpoleType     *M;
  CavityType    *C;
  WigglerType   *W, *Wp;
  InsertionType *ID;
  MapType       *Mapp;
  ElemFamType   *elemfamp;

  const bool prt = false;

  std::cout << std::endl;
  std::cout << "reading machine file: " << mfile_dat << std::endl;

  file_rd(inf, mfile_dat);

  while (inf.getline(line, line_max)) {
    if (prt) printf("%s\n", line);
    sscanf(line, "%*s %*d %*d %ld", &i);

    Cell[i]->dS[X_] = 0.0; Cell[i]->dS[Y_] = 0.0;
    Cell[i]->dT[X_] = 1.0; Cell[i]->dT[Y_] = 0.0;

    sscanf(line, "%s %d %d", Cell[i]->PName, &Cell[i]->Fnum, &Cell[i]->Knum);

    // For compability with lattice parser.
    k = 0;
    while (Cell[i]->PName[k] != '\0')
      k++;
    for (j = k; j < SymbolLength; j++)
      Cell[i]->PName[j] = ' ';

    if (Cell[i]->Knum == 1) {
      strcpy(ElemFam[Cell[i]->Fnum-1].ElemF->PName, Cell[i]->PName);
      globval.Elem_nFam = max((long)Cell[i]->Fnum, globval.Elem_nFam);
    }

    if (i > 0) {
      ElemFam[Cell[i]->Fnum-1].KidList[Cell[i]->Knum-1] = i;
      ElemFam[Cell[i]->Fnum-1].nKid =
	max(Cell[i]->Knum, ElemFam[Cell[i]->Fnum-1].nKid);
    }

    inf.getline(line, line_max);
    if (prt) printf("%s\n", line);
    sscanf(line, "%d %d %d %d", &kind, &method, &n, &reverse);
    get_kind(kind, *Cell[i]);
    if (i > 0)
      ElemFam[Cell[i]->Fnum-1].ElemF->Pkind = Cell[i]->Pkind;

    inf.getline(line, line_max);
    if (prt) printf("%s\n", line);
    sscanf(line, "%lf %lf %lf %lf",
	   &Cell[i]->maxampl[X_][0], &Cell[i]->maxampl[X_][1],
	   &Cell[i]->maxampl[Y_][0], &Cell[i]->maxampl[Y_][1]);

    Cell[i]->PL = 0.0;
    Cell[i]->Reverse = (reverse == 1);

    switch (Cell[i]->Pkind) {
    case undef:
      std::cout << "rdmfile: unknown type " << i << std::endl;
      exit_(1);
      break;
    case marker:
      break;
    case drift:
      inf.getline(line, line_max);
      if (prt) printf("%s\n", line);
      sscanf(line, "%lf", &Cell[i]->PL);
      break;
    case Cavity:
      C = dynamic_cast<CavityType*>(Cell[i]);
      inf.getline(line, line_max);
      if (prt) printf("%s\n", line);
      sscanf(line, "%lf %lf %d %lf %lf",
	     &C->Pvolt, &C->Pfreq, &C->Ph, &globval.Energy, &C->phi);
      globval.Energy *= 1e-9;
      C->Pvolt *= globval.Energy*1e9;
      C->Pfreq *= c0/(2.0*M_PI);
     break;
    case Mpole:
      M = dynamic_cast<MpoleType*>(Cell[i]);
      M->Pmethod = method; M->PN = n;

      if (M->Pthick == thick) {
	inf.getline(line, line_max);
	if (prt) printf("%s\n", line);
	sscanf(line, "%lf %lf %lf %lf",
	       &Cell[i]->dS[X_], &Cell[i]->dS[Y_],
	       &M->PdTpar, &dTerror);
	Cell[i]->dT[X_] = cos(dtor(dTerror+M->PdTpar));
	Cell[i]->dT[Y_] = sin(dtor(dTerror+M->PdTpar));
	M->PdTrms = dTerror - M->PdTpar;
	M->PdTrnd = 1e0;

	inf.getline(line, line_max);
	if (prt) printf("%s\n", line);
	sscanf(line, "%lf %lf %lf %lf %lf",
	       &Cell[i]->PL, &M->Pirho, &M->PTx1, &M->PTx2, &M->Pgap);
	if (M->Pirho != 0.0) M->Porder = 1;
      } else {
	inf.getline(line, line_max);
	if (prt) printf("%s\n", line);
	sscanf(line, "%lf %lf %lf",
	       &Cell[i]->dS[X_], &Cell[i]->dS[Y_], &dTerror); 
	Cell[i]->dT[X_] = cos(dtor(dTerror));
	Cell[i]->dT[Y_] = sin(dtor(dTerror));
	M->PdTrms = dTerror; M->PdTrnd = 1e0;
      }

      M->Pc0 = sin(Cell[i]->PL*M->Pirho/2.0);
      M->Pc1 = cos(dtor(M->PdTpar))*M->Pc0;
      M->Ps1 = sin(dtor(M->PdTpar))*M->Pc0;

      inf.getline(line, line_max);
      if (prt) printf("%s\n", line);
      sscanf(line, "%d %d", &nmpole, &M->n_design);
      for (j = 1; j <= nmpole; j++) {
	inf.getline(line, line_max);
	if (prt) printf("%s\n", line);
	sscanf(line, "%d", &n);
	sscanf(line, "%*d %lf %lf", &M->PB[HOMmax+n], &M->PB[HOMmax-n]);
	M->PBpar[HOMmax+n] = M->PB[HOMmax+n];
	M->PBpar[HOMmax-n] = M->PB[HOMmax-n];
	M->Porder = max(n, M->Porder);
      }

      if (globval.mat_meth && (M->Pthick == thick))
	M->M_lin = get_lin_map(*Cell[i], 0e0);
      break;
    case Wigl:
      W = dynamic_cast<WigglerType*>(Cell[i]);
      W->Pmethod = method; W->PN = n;

      inf.getline(line, line_max);
      if (prt) printf("%s\n", line);
      sscanf(line, "%lf %lf", &Cell[i]->PL, &W->Lambda);

      inf.getline(line, line_max);
      if (prt) printf("%s\n", line);
      sscanf(line, "%d", &W->n_harm);

      if (Cell[i]->Knum == 1) {
	elemfamp = &ElemFam[Cell[i]->Fnum-1];
	// Wiggler_Alloc(*elemfamp.ElemF);
	Wp = dynamic_cast<WigglerType*>(elemfamp->ElemF);
      }
      for (j = 0; j < W->n_harm; j++) {
	inf.getline(line, line_max);
	if (prt) printf("%s\n", line);
	sscanf(line, "%d %lf %lf %lf %lf %lf", &W->harm[j],
	       &W->kxV[j], &W->BoBrhoV[j], &W->kxH[j], &W->BoBrhoH[j],
	       &W->phi[j]);
	Wp->BoBrhoV[j] = W->BoBrhoV[j];
	Wp->BoBrhoH[j] = W->BoBrhoH[j];
      }
      break;
    case Insertion:
      ID = dynamic_cast<InsertionType*>(Cell[i]);
      ID->Pmethod = method; ID->PN = n;

      inf.getline(line, line_max);
      if (prt) printf("%s\n", line);
      sscanf(line, "%lf %d %s", &ID->scaling, &n, file_name);

      if (n == 1) {
	ID->firstorder = true;
	ID->secondorder = false;

	strcpy(ID->fname1, file_name);
	Read_IDfile(ID->fname1, Cell[i]->PL, ID->nx, ID->nz,
		    ID->tabx, ID->tabz, ID->thetax1, ID->thetaz1,
		    ID->long_comp, ID->B2);
      } else if (n == 2) {
	ID->firstorder = false;
	ID->secondorder = true;

	strcpy(ID->fname2, file_name);
	Read_IDfile(ID->fname2, Cell[i]->PL, ID->nx, ID->nz,
		    ID->tabx, ID->tabz, ID->thetax, ID->thetaz,
		    ID->long_comp, ID->B2);
      } else {
	std::cout << "rdmfile: undef order " << n << std::endl;
	exit_(1);
      }

      if (ID->Pmethod == 1)
	ID->linear = true;
      else
	ID->linear = false;

      if (!ID->linear) {
	ID->tx = dmatrix(1, ID->nz,
				      1, ID->nx);
	ID->tz = dmatrix(1, ID->nz,
				      1, ID->nx);
	ID->tab1 = (double *)malloc((ID->nx)*sizeof(double));
	ID->tab2 = (double *)malloc((ID->nz)*sizeof(double));
	ID->f2x = dmatrix(1, ID->nz, 1, ID->nx);
	ID->f2z = dmatrix(1, ID->nz, 1, ID->nx);
	Matrices4Spline(ID);
      }

/*      free_matrix(tx, 1, nz, 1, nx); free_matrix(tz, 1, nz, 1, nx);
      free(tab1); free(tab2);
      free_matrix(f2x, 1, nz, 1, nx); free_matrix(f2z, 1, nz, 1, nx); */
      break;
    case FieldMap:
      break;
    case Map:
      Mapp = dynamic_cast<MapType*>(Cell[i]);
      Id.identity(); Mapp->M.zero();
      for (j = 0; j < n_ps; j++) {
	inf.getline(line, line_max);
	if (prt) printf("%s\n", line);
	sscanf(line, "%lf %lf %lf %lf %lf %lf",
	       &val[0], &val[1], &val[2], &val[3], &val[4], &val[5]);
	for (k = 0; k < n_ps; k++)
	  Mapp->M[j] += val[k]*Id[k];
      }
      break;
    default:
      std::cout << "rdmfile: unknown type" << std::endl;
      exit_(1);
      break;
    }

    if (i == 0)
      Cell[i]->S = 0.0;
    else
      Cell[i]->S = Cell[i-1]->S + Cell[i]->PL;
  }
  
  globval.Cell_nLoc = i;
 
  globval.dPcommon = 1e-8; globval.CODeps = 1e-14; globval.CODimax = 40;

  SI_init();

  std::cout << std::endl;
  std::cout  << std::fixed << std::setprecision(5)
	<< "rdmfile: read " << globval.Cell_nLoc << " elements, C = "
	<< Cell[globval.Cell_nLoc]->S << std::endl;

  inf.close();
}
