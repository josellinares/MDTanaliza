#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#define TRUE  1
#define FALSE 0
#define PI 3.14159265
#define CURRENV "1.2"                                                   /*!< version to check config files */
#define OUTPUTPATH 1100                                                  /*!< limit to write txt or kml files in flow models 1 and 2  */
#define ZPOINTS 10000                                                   /*!< total init z point allowed */
#define BEDRIVER 500000                                                 /*!< total bed river point allowed in each flow path */
#define LAN 1                                                           /*!< Language 1 Spanish 2 English */
#define REPIT 5000                                                      /*!< Mtramdom, number of time same cell is selected */
#define SIZE 100000                                                     /*!< FIFO size in multiflow model */

/**
* Copyright (C) 2009-2017  Jose M. Marrero <josemarllin@gmail.com> and Ramon Ortiz <ramonortizramis@gmail.com>
* You may use, distribute and modify this code under the terms of the MIT License.
* The authors will not be held responsible for any damage or losses or for any implications 
* whatsoever resulting from downloading, copying, compiling, using or otherwise handling this 
* source code or the program compiled.
* Code name: MDTanaliza
* Version: 2.1-2018-04-06 
* Compiled command line: gcc MDTanaliza.c -o MDTanaliza -lm
* Execute command line:  ./MDTanaliza configfilename.cfg
*/ 

/**
* GLOBAL VARIABLES - INPUT-OUTPUT NAMES
*/
char cwd[256], dir_in[256];
char name_incfg[80], nom_grd[256], mask_grd[256], name_newz[256];
char dir_out[256], dirfinout[256];
char nom_trayec[256], nom_sink[256], nom_newdem[256], nom_rast1[256], nom_rast2[256], nom_rast3[256];
char nom_outviakl[256], nom_mask[256], nom_resum[256];
/**
* GLOBAL VARIABLES - CONFIG VAR
*/
char name_invents[256], version[80];
int mod;
double nullval;
/**
* GLOBAL VARIABLES - AUX FUNCTIONS
*/
int vcol, vrow;
int chksum;
int FIFO[SIZE][2];
int gauss[1000], gaussb[1000];
double c1, c2, c3, c4, c5, c6, c7, c8, c9;
double top[8], h[8];
double sum1, sum2, sum3, dlbox;
double longitud, latitud;
double globx, globy;
/**
* GLOBAL VARIABLES - DEM VAR
*/
char header[8];
int opendem, dem_type, recort, totdemcel;
int col1, row1, col2, row2, col3, row3, col4, row4, colx, rowy;
int nx, ny, nxc, nyc, ok;
double demmax, demmin;
double xlo, xhi, ylo, yhi, zlo, zhi, resx, resy, invresx, invresy, resxx, resyy;
double xlonew, xhinew, ylonew, yhinew;
double xmin, xmax, ymin, ymax;
double xc, yc, dx, dy, ddx, ddy, dx2, dy2;
double **topo;
double **topoless;
double **rast1;
double **rast2;
double **rast3;
/**
* GLOBAL VARIABLES - MODIFY MASK VAR
*/
char header2[8];
int modidem;
int totalnwz;
int mask_type, fase, openmask, chkmod1, chkmod2, chkmod3;
int totmask, totmodmask;
int nx2, ny2, nn2, col2x, row2y;
double supmask, supmodmask, volummask;
double xmin2, xmax2, ymin2, ymax2;
double xlo2, xhi2, ylo2, yhi2, zlo2, zhi2, resx2, resy2, invresx2, invresy2, resxx2, resyy2;
double **mask;
/**
* GLOBAL VARIABLES - MORPHOMETRY
*/
int ifsink, metsink, chksink, chksink2;
int ifasp, metasp, chkasp1, chkasp2, cuenta[10];
int ifslope, metslop, chkslop1,chkslop2, chkslop3;
int higcell, iqucell, result;
int totalsink;
/**
* GLOBAL VARIABLES - FLOW PATH
*/
int ncentros, flowtyp, huso, hemis, forcevar;
int maxval, chksing1, chksing2, chksing3, chksing4;
int errcsv[ZPOINTS], errkml[ZPOINTS];
int nfile, totalpt, totalfix;
int nitera, iterafin;
int fcolm, frowm, precol, prerow;
int puntero, test, sizearray;
int resflow, tocelflw, totskflw;
int glpasreach, gllimext, glmxdisreach, glbadcell, glnowayout;
int coin[8], roin[8];
float lmax, hl, hl2, incre, distpt, distran;
double centros[5000][2], maximun;
double mxdisflw, areaflow, acumldis, acumarea;
double *inxdir, *inydir;
long firsepoch, iniepoch, endepoch, difepoch;

/**
* LANGUAGE MENSSAGES
*/
char const * const EStrings[] = 
{
    "Antencion, el archivo de configuracion no se corresponde con la version actual, actualice el archivo\n",
    "Imposible almacenar los puntos de inicio, reduzca el numero o amplie la capacidad de memoria\n",
    "Modifique la variable ZPOINTS\n",
    "Antencion, no hay puntos de inicio para el calculo de trayectorias\n",
	"Antencion, existe directorio de salida, borramos archivos csv y kml",
	"Inicio de lectura del archivo de configuracion\n",
	"Inicio de lectura del MDT\n", //6
	"Modificacion MDT activada\n",
	"Fase 1\n",
	"Fase 2\n",
	"Antencion, el MDT y la Mascara no son iguales\n",
	"Por favor, copruebe la mascara y repita de nuevo\n",
	"Verificando la presencia de sumideros en el DTM\n", //12
	"Calculando la Pendiente-orientacion\n",
	"Calculando la Pendiente-gradiente\n",
	"Calculando trayectorias\n",
	"Calcula trayectoria desde el punto",
	"Error, centro emision fuera de los limites del MDT\n\n",
	"Liberando memoria\n", //18
	"Programa finalizado\n\n",
	"Antención, area de recorte fuera de limites, revise coordenadas\n",
	"Antención, mascara menor que área de recorte, reduzca el area de recorte\n",
	"Alg: Detección de sumideros en MDT\n",
	"Alg: Detección y modificación de sumideros en MDT\n",
	"Antención, se detectaron problemas, revise el archivo resumen\n", //24
	"ATENCION: EL PROGRAMA HA SIDO DETENIDO",
	"Error abriendo archivo MDT, revise el formato y tipo de archivo\n",
	"Error abriendo archivo MASCARA, revise el formato y tipo de archivo\n",
	"Error, dimensiones del MDT y mascara son distintas\n",
	"ATENCION, ocurrió un error al generar el archivo xyz de salida\n",
	"ATENCION, ocurrió un error al leer el archivo xyzz\n", 
	"Simulación correcta, no se detectaron alertas"
    
};
char const * const ENtrings[] = 
{
    "Warning, the config file does not match with the current MDTanaliza version, check the config file\n"
    "The init z points can not stored, decrease the number or increase the memory capacity\n",
    "Change the variable ZPOINTS\n",
    "Warning, There are not available init z points to flow path calculation\n",
	"Warning, The output directory exists, the csv and kml files will be deleted",
	"Reading config file\n",
	"Reading DEM input file\n", //6
	"Initiating DEM modification\n",
	"Step 1\n",
	"Step 2\n",
	"Warning, DEM and Mask are not equal\n",
	"Please, check mask and try again\n",
	"Checking surface depression in DEM\n", //12
	"Calculating slope-aspect\n",
	"Calculating slope-gradient\n",
	"Calculating flow paths\n",
	"Calculating flow path from init z point ",
	"Error, init z point out of DEM limits\n\n"
	"Releasing memory\n", //18
	"End Program\n\n",
	"Warning: clip area out of DEM limits, check coordinates\n",
	"Warning, mask smaller than clip area, decrease the clip area\n",
	"Alg: Surface depression detection in DEM\n",
	"Alg: Surface depression detection and modification in DEM\n",
	"Warning, there were some problems, check resume file\n", //24
	"WARNING: THE PROGRAM HAS BEEN STOPPED",
	"Error opening the DEM file, check format and type file\n",
	"Error opening the MASK file, check format and type file\n",
	"Error, DEM and MASK size is different\n",
	"WARNING, an error occurred while creating xyz output file\n",
	"WARNING, an error occurred while reading xyz input file\n",
	"Correct simulation, warning not detected"
    
};
//----------------------------------------------------------------------
/**
* STRUCT DEFINITION
*/
struct tm;

typedef struct  
{  
	int    cjerar;
	int    crio;
	int    ctramo;
	int    cidpt;
	float  cdist;
	double cxcoor;
	double cycoor;
	double czcoor;
	double clong;
	double clat;
	float  cdx;
	float  cdt;
	int    quality;
}CAUCE;
CAUCE caucerio[BEDRIVER];                                               /*!< Store cell attributes from each flow path */

typedef struct  
{  
	double ptxcoor;
	double ptycoor;
}INITPT;
INITPT puntos[ZPOINTS];                                                 /*!< Store init z points */
/**
* FUNCTION DECLARATION SECTION
*/
int read_cfg(void);
int read_bingrd(void);
int read_ascgrd(void);
int read_grdbinmask(void);
int read_grdascmask(void);
//--
const char * wrst(int n);
double** Make2DDoubleArray(int arraySizeX, int arraySizeY) ;
double box_muller(double m, double s);
void campana(void);
int  convert_coor(double coorx, double coory);
int  getmovingcell(int ix, int jy, int withdif);
void calc_index(double vx, double vy);
void get_coor(int col, int row);
void calc_levelfill(int col, int row);
int  calc_isosink(int coli, int rowj);
//---
void fifoini(void);
void fifodesplaza(void);
void fifocarga(void);
//--
int getnpt(void);
int readchangenewz(void);
//--
int  fix_sinks(void);
int calc_saspect(void);
int calc_sgrad(void);
//---
void calc_singflow(void);
void calc_montflow(void);
void calc_mulflow(void);
//--
int write_newdem(int typ);
int write_rast1(int typ, int mod);
int write_rast2(int typ, int mod);
int write_rast3(int typ, int mod);
//---
int write_pathcsv(void);
int write_outkmll(void);
//---
int write_flowres(void);
int write_resum(void);


//---------------------------------------------------------------------------------------
//***************************************************************************************
//*********************************END HEAD FILE*****************************************
//***************************************************************************************
//********************************READING FUNCTIONS**************************************
//***************************************************************************************
//---------------------------------------------------------------------------------------

/*! READING CONFIGURATION FILE */
int read_cfg(void)
{
FILE *file;
DIR* dir;
int i;
char texto[80],  comando[256]; 
char dirout[80], dem[80], namemask[80], namenewz[80];
double txcoor, tycoor;
	
	
	
	printf("\n***Reading cfg file, %s***\n", name_incfg);
	if ((file = fopen(name_incfg,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        return 0;
    }
    else
    {	
		/**
		* CHECK CONFIG FILE VERSION
		*/
		fscanf(file,"%s %s", texto, version); 
		printf("CURRENT VERSION = %s\n", CURRENV);
		printf("CONFIG FILE VERSION = %s\n", version);
		if(strcmp(CURRENV,version)!=0)
		{
			printf("-----------ERROR--------------\n");
			printf("%s\n", wrst(25));
			printf("%s\n", wrst(0));
			exit(0);
		}
		else
		{	
			/**
			* DEM'S FILENAME AND OUTPUT DIRECTORY
			*/
			fscanf(file,"%s %s", texto, dem);
			fscanf(file,"%s %s", texto, dirout);
			/**
			* DEM MODIFIED SECTION
			* Raster mask should have the same center cell coordinate and resolution
			*/
			fscanf(file,"%s", texto);  
			fscanf(file,"%s %i", texto, &modidem);                      /*!< If DEM will be modified value 1-0 */ 
			fscanf(file,"%s %i", texto, &fase);                         /*!< 1 Detection / 2 Modification. Yo won't get a depressionless DEM */
			fscanf(file,"%s %i", texto, &mask_type);                    /*!< 1 Binary / 2 ASCII */
			fscanf(file,"%s %s", texto, namemask);                      /*!< STEP 1, raster mask filename */
			fscanf(file,"%s %s", texto, namenewz);                      /*!< STEP 2, txt xyzz filename */
			/**
			* DEM'S PARAMETERS
			*/
			fscanf(file,"%s", texto);  
			fscanf(file,"%s %i",  texto, &dem_type);                    /*!< 1 Binary / 2 ASCII */
			fscanf(file,"%s %lf",  texto, &demmax);                      /*!< Max. Z value in DEM or interest area */
			fscanf(file,"%s %lf",  texto, &demmin);                      /*!< Min. Z value in DEM or interest area */
			fscanf(file,"%s %lf", texto, &nullval);                     /*!< Null value to be used at internal level and in output raster */
			fscanf(file,"%s %i",  texto, &recort);                      /*!< If DEM will be clipped value 1-0 */
			fscanf(file,"%s %lf", texto, &xlonew);                      /*!< X min. coord. bottom-left corner */
			fscanf(file,"%s %lf", texto, &xhinew);                      /*!< X max. coord. bottom-right corner */
			fscanf(file,"%s %lf", texto, &ylonew);                      /*!< Y min. coord. bottom-left corner */
			fscanf(file,"%s %lf", texto, &yhinew);                      /*!< Y max. coord. bottom-right corner */
			fscanf(file,"%s %lf", texto, &resx);                        /*!< X resolution */
			fscanf(file,"%s %lf", texto, &resy);                        /*!< Y resolution */
			/**
			* SURFACE DEPRESSION AND MORPHOMETRY
			*/
			fscanf(file,"%s", texto);  
			fscanf(file,"%s %i", texto, &metsink);                      /*!< If higher than 0, surface depression will be activated - algorithms 1 Detect 2 Modify */
			fscanf(file,"%s %i", texto, &metasp);                       /*!< If higher than 0, slope-aspect will be activated - algorithms 1 LHM 2 SSM */  
			fscanf(file,"%s %i", texto, &metslop);                      /*!< If higher than 0, slope-gradient will be activated - algorithms 1-9 */
			/**
			* FLOW PATHS
			*/
			fscanf(file,"%s", texto); 
			fscanf(file,"%s %i", texto, &flowtyp);                      /*!< If higher than 0, flow path will be activated - algorithms 1-4 */
			fscanf(file,"%s %f", texto, &lmax);                         /*!< Max. distance in meters (1-4) */
			fscanf(file,"%s %f", texto, &hl);                           /*!< Critical height in meters (1-4) */
			fscanf(file,"%s %f", texto, &distran);                      /*!< Restrict multiplow in % (4)*/
			fscanf(file,"%s %f", texto, &incre);                        /*!< Fill increase in meters (1-3)*/
			fscanf(file,"%s %i", texto, &forcevar);						/*!< Force interaction between different paths (1 yes - 0 No)*/
			fscanf(file,"%s %i", texto, &nitera);                       /*!< Total number of Interations (3) */
			fscanf(file,"%s %i", texto, &mod); 							/*!< Writing raster mode, 0 unified raster, 1 raster per point */
			fscanf(file,"%s %i", texto, &huso);                         /*!< Huso/UTM grid zones */
			fscanf(file,"%s %i", texto, &hemis);                        /*!< Hemisphere North 0 South 1 */
			fscanf(file,"%s", texto);
			fscanf(file,"%s %i", texto, &ncentros);                     /*!< Total number of init z points */
			fscanf(file,"%s", texto);
			if(ncentros > 0)
			{
				if(ncentros > ZPOINTS)                                  /*!< Check if there is enough memory to store init z points */
				{
					printf("-----------ERROR--------------\n");
					printf("%s\n", wrst(25));
					printf("%s\n", wrst(1));
					printf("%s\n", wrst(2));
					exit(0);
				}	
				else
				{
					/**
					* Reading init z points coordinates
					*/
					for(i=0;i<ncentros;i++)
					{	
						fscanf(file,"%lf %lf", &txcoor, &tycoor);
						puntos[i].ptxcoor = txcoor;                     /*!< Store the init z points in puntos struc */
						puntos[i].ptycoor = tycoor;
						/**
						* For debugging
						* printf("%i %lf %lf\n", i, txcoor, tycoor);
						*/
					}
				}	
			}
			if(ncentros == 0 && flowtyp > 0)                            /*!< Check if flow path has been selected and there are init z points */
			{
				printf("-----------ERROR--------------\n");
				printf("%s\n", wrst(25));
				printf("%s\n", wrst(3));	
				exit(0);
			}	
		}	
			
	}
	fclose(file);
	/**
	* Create absolute paths
	*/
	sprintf(nom_grd, "%s%s", cwd, dem);
	sprintf(dir_out, "%s%s", cwd, dirout);
	sprintf(mask_grd, "%s%s", cwd, namemask);
	sprintf(name_newz, "%s%s", cwd, namenewz);
	sprintf(dirfinout, "%s", dir_out);
	/**
	* PRINT CONFIG FILE VARIABLES
	*/
	printf("\n");
	printf("[SEC_DIR]\n");
    printf("DEMIn  %s\n", nom_grd);
    printf("DirOut %s\n", dir_out);
    //-----------
    printf("[SEC_DEM]\n");    
    printf("DEM TYPE = %i\n",dem_type);
    printf("DEM MAX VALUE = %lf\n",demmax);
	printf("DEM MIN VALUE = %lf\n",demmin);
    printf("DEM NULL DATA = %.2lf\n",nullval);
	printf("CLIP DEM = %i\n",recort);
	printf("XCOOR MIN VALUE = %lf\n",xlonew);
	printf("XCOOR MAX VALUE = %lf\n",xhinew);
	printf("YCOOR MIN VALUE = %lf\n",ylonew);
	printf("YCOOR MAX VALUE = %lf\n",yhinew);
    printf("DEM RESOL X = %lf\n",resx);
    printf("DEM RESOL Y = %lf\n",resy); 
    //-----------
    printf("[SEC_MODYZVAL]\n"); 
    printf("ACTIVA NEWXYZ %i\n", modidem);
    printf("FASE %i\n", fase);
    printf("MASK TYPE = %i\n",mask_type);
    printf("DATA IN MASCI %s\n", mask_grd); 
    printf("DATA IN FILE %s\n", name_newz);  
    //-----------
    printf("[DEM_MORPHOMETRIC]\n"); 
    printf("SINK %i\n", metsink);
    printf("S-ASPECT %i\n", metasp);
    printf("S-GRADIENT %i\n", metslop);
    //-----------
    printf("[SEC_TRAYECT]\n");
    printf("Algorithm type %i\n", flowtyp);
    printf("Dist. max %.2f\n", lmax);
    printf("Crit. height %.2f\n", hl);
    printf("Rest. Multiflow %f\n", distran);
    printf("Fill incre. %.2f\n", incre);
    printf("Force interac. %i\n", forcevar);
    printf("Iterations %i\n", nitera);
    printf("Writing raster mode %i\n", mod);
    printf("UTM grid zone %i\n", huso);
    printf("Hemisphere %i\n", hemis);
    //-----------
    printf("[SEC_POINTS]\n");
    printf("N Z points %i\n", ncentros); 
    if( ncentros < 10)
    { 
		for(i=0;i<ncentros;i++)printf("%lf %lf\n", puntos[i].ptxcoor, puntos[i].ptycoor);
	}		 
    printf("-------------\n");
    /**
	* Create output directories
	*/
    if ((dir = opendir(dir_out))!= NULL)                                /*!< If exist */
	{
		if(flowtyp == 1 || flowtyp == 2)                                /*!< if flow path algorithms 1 or 2 has been selected  */
		{
			//printf("%s", msgstxt5);
			printf("%s\n", wrst(4)); 
			sprintf(comando, "rm -rf %s*.csv", dir_out);                /*!< delete csv files  */
				printf("Deleted txt files\n\n");
				system(comando);
			sprintf(comando, "rm -rf %s*.kml", dir_out);                /*!< delete kml files  */
				printf("Deleted kml files\n\n");
				system(comando);
		}		
	}
	else                                                                /*!< If not */
	{	
		printf("Creating output directoy");
		sprintf(comando,"mkdir %s", dir_out);                           /*!< create output directory  */
			printf("\n%s\n",comando);
			system(comando);
	}	
	return 1;
}

/*! READING DEM IN BINARY FORMAT */
int read_bingrd(void)
{
FILE *in;
char cabecera[8];
short int datoin[4];
double datodo[8], minz, maxz;
float datofl[4];
int i, j, k, l, colini, rowini;

	printf("DEM FILE %s\n",nom_grd);
    if((in=fopen(nom_grd,"rb"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Reading golden software format head in binary
	*/
    fread(&cabecera,4,1,in);
    fread(&datoin,2*sizeof(short int),1, in);
    fread(&datodo,6*sizeof(double),   1, in);
    cabecera[4] = 0;
    nx  = datoin[0];
	ny  = datoin[1];
	xlo = datodo[0];
	xhi = datodo[1]; 
	ylo = datodo[2];
	yhi = datodo[3];
	zlo = datodo[4];
	zhi = datodo[5];
    if(recort != 1)                                                     /*!< if not clip section */
    {
		resx  = (xhi-xlo)/(double)(nx-1);
		resy  = (yhi-ylo)/(double)(ny-1);
		colx = nx;
		rowy = ny;
		colini = 0;
		rowini = 0;
		xmin = xlo;
		xmax = xhi;
		ymin = ylo;
		ymax = yhi;
	}	
	if(recort == 1)                                                     /*!< if clip section */
	{
		minz   = 10000;                                                 /*!< Init minz var - Recal z min val */
		maxz   = 0;                                                     /*!< Init maxz var - Recal z max val */
		colx   = (xhinew-xlonew) / resx;                                /*!< Total cells in x direction */
		rowy   = (yhinew-ylonew) / resy;                                /*!< Total cells in y direction */
		colini = (xlonew - xlo) / resx;                                 /*!< Total cells in x direction from the beginning to the clip area */
		rowini = (ylonew - ylo) / resy;                                 /*!< Total cells in y direction from the beginning to the clip area */
		xmin = xlonew;
		xmax = xhinew;
		ymin = ylonew;
		ymax = yhinew;
		if((xmin < xlo || xmax > xhi) && (ymin < ylo || ymax > yhi)) /*!< Check if clip area is smaller than the real DEM */
		{
			printf("xmin %.3lf - %.3lf : xmax %.3lf %.3lf\n", xmin, xlo, xmax, xhi);
			printf("ymin %.3lf - %.3lf : ymax %.3lf %.3lf\n", ymin, ylo, ymax, yhi);
			printf("%s\n", wrst(20));
			return 1;
		}
	}
	/**
	* Creating arrays
	*/
	topo     = Make2DDoubleArray (colx, rowy);                          /*!< Original DEM */
    topoless = Make2DDoubleArray (colx, rowy);                          /*!< Modified DEM Mod or Sink */
    rast1    = Make2DDoubleArray (colx, rowy);                          /*!< Store data from different algorithms */
    rast2    = Make2DDoubleArray (colx, rowy);                          /*!< Store data from different algorithms */
    rast3    = Make2DDoubleArray (colx, rowy);                          /*!< Store data from different algorithms */
	printf("nx=%i - colx=%i colini=%i colx+colini=%i, ny=%i rowy=%i rowini=%i rowy+rowini=%i\n", nx, colx, colini, (colx+colini), ny, rowy, rowini, (rowy+rowini));
    l=0;
    for(j=0;j<ny;j++) //row
    {
        k=0;
        for(i=0;i<nx;i++)
        {           
            fread(&datofl,sizeof(float),1,in);                          /*!< Read data from original DEM */
            if((datofl[0] > demmax) || (datofl[0] < demmin))datofl[0] = nullval;  /*!< if data is out of z limits, assign null value */
            if(recort != 1)                                             /*!< if not clip section */
			{
				topo[i][j]     = (double)datofl[0];  
				topoless[i][j] = (double)datofl[0];                     /*!< Initially, this array store the original DEM values */
				rast1[i][j]    = 0;
				rast2[i][j]    = 0;
				rast3[i][j]    = 0;
			}
			if(recort == 1)                                             /*!< if clip section, new index are used k,l */
			{	
				
				if( (i > colini && i < (colini+colx)) && (j > rowini && j < (rowini+rowy)))  /*!< Check if the index (colum row) are inside the clip area */
				{
					if(datofl[0] != nullval)                            /*!< if not null */
					{
						if(datofl[0] < minz) minz = datofl[0];          /*!< Recal z min val */
						if(datofl[0] > maxz) maxz = datofl[0];          /*!< Recal z max val */
					}
					topo[k][l]     = (double)datofl[0];	
					topoless[k][l] = (double)datofl[0];
					rast1[k][l]    = 0;
					rast2[k][l]    = 0;
					rast3[k][l]    = 0;
					k++;
				}
			}			
        }
        if(k==colx-1)l++;	                                            /*!< When the row in clip mode is finished, a new colum is added */	                                          
    }    
    fclose(in);
    /**
	* In clip mode, to avoid memory overflow, the index are reduced
	*/
    if(recort == 1)
    {
		colx = colx-1; 
		rowy = rowy-1;
		zlo = minz;                                                     /*!< Update z min val */
		zhi = maxz;                                                     /*!< Update z max val */
	}
	/**
	* Calculating array scales
	*/
    invresx = 1/resx;
    invresy = 1/resy;
    resxx = resx*resx;
    resyy = resy*resy;
    totdemcel = colx * rowy;
    /**
	* Print results
	*/
    printf("Header data DEM file\n");
    printf("limits k=%i - l=%i\n", k, l);
	printf("DEM type: %s\n", cabecera);
    printf("colx  = %5i -- rowy = %5i\n", colx, rowy);
    printf("Tot cells in DEM = %i\n", totdemcel);
	printf("xmin = %f -- ymin = %f\n", xmin, ymin);
	printf("xmax = %f -- ymax = %f\n", xmax, ymax);
    printf("zlo = %10.4f -- zhi = %10.4f\n", zlo, zhi);
    printf("DEM resolution in resx = %f\n", resx);
    printf("DEM resolution in resy = %f\n", resy);
    printf("DEM invresx = %f and invresy = %f\n", invresx, invresy);
    printf("DEM resx2 = %f and resy2 = %f\n", resxx, resyy);
    printf("end read DEM file\n");  
    /**
	* Check irregularities in input data
	*/
    if(demmin > zlo)chksum++;
    if(demmax < zhi)chksum++;
    printf("---------------------------------\n\n");
    
    return 0;
}

/*! READING DEM IN ASCII FORMAT */
int read_ascgrd(void)
{
FILE *in;
char header[8];
int i, j, k, l, colini, rowini;
double dato, minz, maxz;

	printf("Open input file, %s\n", nom_grd);
    if ((in=fopen(nom_grd,"rt"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Reading golden software format head in ASCII
	*/
    fscanf(in,"%s",      header);
    fscanf(in,"%i %i",   &nx, &ny);
	fscanf(in,"%lf %lf", &xlo, &xhi);
	fscanf(in,"%lf %lf", &ylo, &yhi);
	fscanf(in,"%lf %lf", &zlo, &zhi);
    if(recort != 1)                                                     /*!< if not clip section */
    {
		resx = (xhi-xlo)/(double)(nx-1);
		resy = (yhi-ylo)/(double)(ny-1);
		colx = nx;
		rowy = ny;
		colini = 0;
		rowini = 0;
		xmin = xlo;
		xmax = xhi;
		ymin = ylo;
		ymax = yhi;
	}
    if(recort == 1)                                                     /*!< if clip section */
    {
		minz   = 10000;                                                 /*!< Init minz var - Recal z min val */
		maxz   = 0;                                                     /*!< Init maxz var - Recal z max val */
		colx   = (xhinew-xlonew) / resx;                                /*!< Total cells in x direction */
		rowy   = (yhinew-ylonew) / resy;                                /*!< Total cells in y direction */
		colini = (xlonew - xlo) / resx;                                 /*!< Total cells in x direction from the beginning to the clip area */
		rowini = (ylonew - ylo) / resy;                                 /*!< Total cells in y direction from the beginning to the clip area */
		xmin = xlonew;
		xmax = xhinew;
		ymin = ylonew;
		ymax = yhinew;
		if((xmin < xlo || xmax > xhi) && (ymin < ylo || ymax > yhi))    /*!< Check if clip area is smaller than the real DEM */
		{
			printf("xmin %.3lf - %.3lf : xmax %.3lf %.3lf\n", xmin, xlo, xmax, xhi);
			printf("ymin %.3lf - %.3lf : ymax %.3lf %.3lf\n", ymin, ylo, ymax, yhi);
			printf("%s\n", wrst(20));
			return 1;
		}
    }
    /**
	* Creating arrays
	*/
    topo     = Make2DDoubleArray (colx, rowy);                          /*!< Original DEM */
    topoless = Make2DDoubleArray (colx, rowy);                          /*!< Modified DEM Mod or Sink */
    rast1    = Make2DDoubleArray (colx, rowy);                          /*!< Store data from different algorithms */
    rast2    = Make2DDoubleArray (colx, rowy);                          /*!< Store data from different algorithms */
    rast3    = Make2DDoubleArray (colx, rowy);                          /*!< Store data from different algorithms */
    printf("nx=%i - colx=%i colini=%i colx+colini=%i, ny=%i rowy=%i rowini=%i rowy+rowini=%i\n", nx, colx, colini, (colx+colini), ny, rowy, rowini, (rowy+rowini));
    l = 0;
    for(j=0;j<ny;j++)//para cada fila
    {
		k = 0;
		for(i=0;i<nx;i++)//todas las columnas
		{
			fscanf(in,"%lf", &dato);                                    /*!< Read data from original DEM */
			if((dato > demmax) || (dato < demmin))dato = nullval;       /*!< if data is out of limits, assign null value */
			
			if(recort != 1)                                             /*!< if not clip section */
			{
				topo[i][j]     = (double)dato;	
				topoless[i][j] = (double)dato;
				rast1[i][j]    = 0;
				rast2[i][j]    = 0;
				rast3[i][j]    = 0;
			}
			if(recort == 1)                                             /*!< if clip section, new index are used k,l */
			{	
				if( (i > colini && i < (colini+colx)) && (j > rowini && j < (rowini+rowy)))  /*!< Check if index are inside the clip area */
				{	
					if(dato != nullval)                                 /*!< if not null */
					{
						if(dato < minz) minz = dato;                    /*!< Recal z min val */
						if(dato > maxz) maxz = dato;                    /*!< Recal z max val */
					}
					topo[k][l]     = (double)dato;	
					topoless[k][l] = (double)dato;
					rast1[k][l]    = 0;
					rast2[k][l]    = 0;
					rast3[k][l]    = 0;
					k++;
				}
			}		
		}
		if(k==colx-1)l++;	                                            /*!< When the row in clip mode is finished, a new colum is added */
    }    
    fclose(in);
    /**
	* In clip mode, to avoid memory overflow, the index are reduced
	*/
    if(recort == 1)
    {
		colx = colx-1; 
		rowy = rowy-1;
		zlo = minz;                                                     /*!< Update z min val */
		zhi = maxz;                                                     /*!< Update z max val */
	}
    /**
	* Calculating array scales
	*/
    invresx = 1/resx;
    invresy = 1/resy;
    resxx = resx*resx;
    resyy = resy*resy;
    totdemcel = colx * rowy;
    /**
	* Print results
	*/
    printf("Header data DEM file\n");
    printf("limits k=%i - l=%i\n", k, l);
	printf("DEM type: %s\n", header);
    printf("colx = %i  -- rowy  = %i\n", colx, rowy);
    printf("Tot cells in DEM = %i\n", totdemcel);
	printf("xmin = %f -- ymin = %f\n", xmin, ymin);
	printf("xmax = %f -- ymax = %f\n", xmax, ymax);
    printf("zlo = %f -- zhi = %f\n", zlo, zhi);
    printf("DEM resolution in resx = %f\n", resx);
    printf("DEM resolution in resy = %f\n", resy);
    printf("DEM invresx = %f and invresy = %f\n", invresx, invresy);
    printf("DEM resxx = %f and resyy = %f\n", resxx, resyy);
    printf("end read DEM file\n");  
    /**
	* Check irregularities in input data
	*/
    if(demmin > zlo)chksum++;
    if(demmax < zhi)chksum++;
    printf("---------------------------------\n\n");
    return 0;
}

/*! READING RASTER-MASK IN BINARY FORMAT */
int read_grdbinmask(void)
{
FILE *in;
short int datain[4];
double datall[8];
float datafl[4];
int i, j, k, l, coliniw, rowiniw, fink;
int read;

	printf("Open mask input file type %i, %s\n", mask_type, mask_grd);
    if ((in=fopen(mask_grd,"rb"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Reading golden software format head in binary
	*/
    fread(&header,4,1,in);
    fread(&datain,2*sizeof(short int),1,in);
    fread(&datall,6*sizeof(double),1,in);
    header[4] = 0;
    nx2  = datain[0];
    ny2  = datain[1];
    xlo2 = datall[0];
    xhi2 = datall[1]; 
    ylo2 = datall[2];
    yhi2 = datall[3];
    zlo2 = datall[4];
    zhi2 = datall[5];
    if(recort != 1)                                                     /*!< if not clip section */
    {
		resx2  = (xhi2-xlo2)/(double)(nx2-1);
		resy2  = (yhi2-ylo2)/(double)(ny2-1);
		col2x = nx2;
		row2y = ny2;
		coliniw = 0;
		rowiniw = 0;
		xmin2 = xlo2;
		xmax2 = xhi2;
		ymin2 = ylo2;
		ymax2 = yhi2;
	}	
	if(recort == 1)                                                     /*!< if clip section */
	{
		resx2  = resx;
		resy2  = resy;
		col2x   = (xhinew-xlonew) / resx2;                              /*!< Total cells in x direction */
		row2y   = (yhinew-ylonew) / resy2;                              /*!< Total cells in y direction */
		coliniw = (xlonew - xlo2) / resx2;                              /*!< Total cells in x direction from the beginning to the clip area */
		rowiniw = (ylonew - ylo2) / resy2;                              /*!< Total cells in y direction from the beginning to the clip area */
		xmin2 = xlonew;
		xmax2 = xhinew;
		ymin2 = ylonew;
		ymax2 = yhinew;
		if(nx2 < col2x-2 || ny2 < row2y-2)                              /*!< Check if mask is smaller than clip area */
		{
			printf("nx %i col %i - ny %i  row %i\n", nx2, col2x, ny2, row2y);
			printf("%s\n", wrst(21));
			return 1;
		}
		/**
		* Depending relation between mask dimension and clip area, two possibilities
		*/
		if(nx2 == col2x-2 && ny2 == row2y-2) read = 1;
		if(nx2  > col2x-2 && ny2  > row2y-2) read = 0;
		printf("nx %i col %i - ny %i  row %i read %i\n", nx2, col2x, ny2, row2y, read);
	}	
    printf("nx2=%i - col2x=%i colini=%i col2x+colini=%i, ny2=%i row2y=%i rowini=%i row2y+rowini=%i\n", nx2, col2x, coliniw, (col2x+coliniw), ny2, row2y, rowiniw, (row2y+rowiniw));
	/**
	* Creating array
	*/
    mask   = Make2DDoubleArray (col2x, row2y);
    l=0;
    fink=0;
    for(j=0;j<ny2;j++) //rows
    {
		k=0;
		for(i=0;i<nx2;i++) //colums
		{
			fread(&datafl,sizeof(float),1,in);                          /*!< Read data from mask */
			if(datafl[0] != 1) datafl[0] = 0;	                        /*!< if z data is out of limits, assign null value */
			
			if(recort != 1)                                             /*!< if not clip section */
			{
				mask[i][j]  = (double)datafl[0];
			}
			if(recort == 1)                                             /*!< if clip section, new index are used k,l */
			{	
				if(read == 0)                                           /*!< When mask dimensions are higher than the clip area */
				{
					if( (i > coliniw && i < (coliniw+col2x)) && (j > rowiniw && j < (rowiniw+row2y)))   /*!< Check if index are inside the clip area */
					{	
						mask[k][l] = (double)datafl[0];
						if(l==0)fink++;
						k++;
					}
				}
				if(read == 1)                                           /*!< When mask dimensions are equal than the clip area */
				{
					mask[k][l] = (double)datafl[0];
					if(l==0)fink++;
					k++;
				}	
			}	
		}	
		if(k==col2x-1)l++;                                              /*!< When the row in clip mode is finished, a new column is added */
    }      
    fclose(in);
    /**
	* In clip mode, to avoid memory overflow, the index are reduced
	*/    
    if(recort == 1)
    {
		col2x = col2x-1; 
		row2y = row2y-1;
	}
	/**
	* Matching number of columns and rows in DEM and Mask
	*/
    if((nx - nx2) != 0) nx2 = nx;
    if((ny - ny2) != 0) ny2 = ny;
    /**
	* Calculating array scales
	*/
    invresx2 = 1/resx2;
    invresy2 = 1/resy2;
    resxx2 = resx2*resx2;
    resyy2 = resy2*resy2;
    /**
	* Print results
	*/
    printf("limites k=%i - l=%i\n", fink, l);
    printf("Header data MASK file\n");
	printf("MASK type: %s\n", header2);
    printf("nx2 = %i  -- ny2  = %i\n", nx2, ny2);    
    printf("col2x  = %5i -- row2y = %5i\n", col2x, row2y);
	printf("xmin = %f -- ymin = %f\n", xmin, ymin);
	printf("xmax = %f -- ymax = %f\n", xmax, ymax);
    printf("zlo2 = %f -- zhi2 = %f\n", zlo2, zhi2);
    printf("MASK resolution in resx2 = %f\n", resx2);
    printf("MASK resolution in resy2 = %f\n", resy2);
    printf("MASK invresx2 = %f and invresy2 = %f\n", invresx2, invresy2);
    printf("MASK resxx2 = %f and resyy2 = %f\n", resxx2, resyy2);
    printf("end read MASK file\n");  
    printf("---------------------------------\n\n");
    return 0;
}

/*! READING RASTER-MASK IN ASCII FORMAT */
int read_grdascmask(void)
{
FILE *in;
int i, j, k, l, coliniw, rowiniw, fink;
int read;
double dato;

	printf("Open input file type %i, %s\n", mask_type, mask_grd);
    if ((in=fopen(mask_grd,"rt"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Reading golden software format head in ASCII
	*/
    fscanf(in,"%s",      header2);
    fscanf(in,"%i %i",   &nx2, &ny2);
    fscanf(in,"%lf %lf", &xlo2, &xhi2);
    fscanf(in,"%lf %lf", &ylo2, &yhi2);
    fscanf(in,"%lf %lf", &zlo2, &zhi2);
    
    if(recort != 1)                                                     /*!< if not clip section */
    {
		resx2 = (xhi2-xlo2)/(double)(nx2-1);
		resy2 = (yhi2-ylo2)/(double)(ny2-1);
		col2x = nx2;
		row2y = ny2;
		coliniw = 0;
		rowiniw = 0;
		xmin2 = xlo2;
		xmax2 = xhi2;
		ymin2 = ylo2;
		ymax2 = yhi2;
	}
    if(recort == 1)                                                     /*!< if clip section */
    {
		resx2 = resx;
		resy2 = resy;
		col2x   = (int)(xhinew-xlonew) / resx2;                         /*!< Total cells in x direction */
		row2y   = (int)(yhinew-ylonew) / resy2;                         /*!< Total cells in Y direction */
		coliniw = (xlonew - xlo2) / resx2;                              /*!< Total cells in x direction from the beginning to the clip area */
		rowiniw = (ylonew - ylo2) / resy2;                              /*!< Total cells in Y direction from the beginning to the clip area */
		xmin2 = xlonew;
		xmax2 = xhinew;
		ymin2 = ylonew;
		ymax2 = yhinew;
		if(nx2 < col2x-2 || ny2 < row2y-2)                              /*!< Check if mask is smaller than clip area */
		{
			printf("nx %i col %i - ny %i  row %i\n", nx2, col2x, ny2, row2y);
			printf("%s\n", wrst(21));
			return 1;
		}
		/**
		* Depending relation between mask dimension and clip area, two possibilities
		*/
		if(nx2 == col2x-2 && ny2 == row2y-2) read = 1;
		if(nx2  > col2x-2 && ny2  > row2y-2) read = 0;	
		
    }
    printf("nx2=%i - col2x=%i colini=%i col2x+colini=%i, ny2=%i row2y=%i rowini=%i row2y+rowini=%i\n", nx2, col2x, coliniw, (col2x+coliniw), ny2, row2y, rowiniw, (row2y+rowiniw));
    /**
	* Creating array
	*/
    mask   = Make2DDoubleArray (col2x, row2y);
    l=0;
    fink=0;
    for(j=0;j<ny2;j++) //rows
    {
		k=0;
		for(i=0;i<nx2;i++) //colums
		{
			fscanf(in,"%lf", &dato);                                    /*!< Read data from mask */
			if(dato != 1) dato = 0;	
			if(recort != 1)                                             /*!< if not clip section */
			{
				mask[i][j]  = (double)dato;	
			}
			if(recort == 1)                                             /*!< if clip section, new index are used k,l */
			{	
				if(read == 0)                                           /*!< When mask is higher than the clip area */
				{
					if( (i > coliniw && i < (coliniw+col2x)) && (j > rowiniw && j < (rowiniw+row2y)))     /*!< Check if index are inside the clip area */
					{	
						mask[k][l] = (double)dato;
						if(l==0)fink++;
						k++;
					}
				}
				if(read == 1)                                           /*!< When mask is equal than the clip area */
				{
					mask[k][l] = (double)dato;
					if(l==0)fink++;
					k++;
				}
			}
		}
		if(k==col2x-1)l++;		                                        /*!< When the row in clip mode is finished, a new column is added */
    }    
    fclose(in);
    /**
	* In clip mode, to avoid memory overflow, the index are reduced
	*/ 
    if(recort == 1)
    {
		col2x = col2x-1; 
		row2y = row2y-1;
	}
    /**
	* Matching number of columns and rows in DEM and Mask
	*/
    if((nx - nx2) != 0) nx2 = nx;
    if((ny - ny2) != 0) ny2 = ny;
    /**
	* Calculating array scales
	*/
    invresx2 = 1/resx2;
    invresy2 = 1/resy2;
    resxx2 = resx2*resx2;
    resyy2 = resy2*resy2;
    /**
	* Print results
	*/
    printf("limites k=%i - l=%i\n", fink, l);
    printf("Header data MASK file\n");
	printf("MASK type: %s\n", header2);
	printf("col2x = %i  -- row2y  = %i\n", colx, rowy);
	printf("xmin = %f -- ymin = %f\n", xmin, ymin);
	printf("xmax = %f -- ymax = %f\n", xmax, ymax);
    printf("zlo2 = %f -- zhi2 = %f\n", zlo2, zhi2);
    printf("MASK resolution in resx2 = %f\n", resx2);
    printf("MASK resolution in resy2 = %f\n", resy2);
    printf("MASK invresx2 = %f and invresy2 = %f\n", invresx2, invresy2);
    printf("MASK resxx2 = %f and resyy2 = %f\n", resxx2, resyy2);
    printf("end read MASK file\n");  
    printf("---------------------------------\n\n");
    return 0;
}

//---------------------------------------------------------------------------------------
//***************************************************************************************
//*****************************END READING INPUT FILES***********************************
//***************************************************************************************
//*******************************AUXILIAR FUNCTIONS**************************************
//***************************************************************************************
//---------------------------------------------------------------------------------------

/*! GET MESSAGES ACCORDING TO LANGUAGE */
const char * wrst(int n)
{
	if (LAN == 1)return EStrings[n];
	if (LAN == 2)return ENtrings[n];
	
}

/*! DINAMIC ARRAY GENERATOR */
double** Make2DDoubleArray(int arraySizeX, int arraySizeY) 
{
int i;	
double** theArray;
	theArray = (double**) malloc(arraySizeX*sizeof(double*));
	for(i=0; i<arraySizeX;i++)
	{
		theArray[i] = (double*) malloc(arraySizeY*sizeof(double));
		if(theArray[i]== NULL)printf("error 2\n");
	}	
	return theArray;
} 

/*! DINAMIC SINGLE ARRAYS */
double* MakeSingleArray(int arraySize) 
{	
double* theArray;
	theArray = (double*) malloc(arraySize*sizeof(double));
	if(theArray== NULL)printf("error 2\n");

	return theArray;
}

/*! RESET ARRAYS */
void resarray(int typ)
{
int i, j;	
int xcol, yrow;

	if(typ == 0)
	{
		/**
		* Reset control raster to 0 (rast2)
		*/
		for(i=0;i<tocelflw;i++)
		{
			xcol = inxdir[i];
			yrow = inydir[i];
			rast2[xcol][yrow] = 0; 
			inxdir[i] = 0;
			inydir[i] = 0;
		}
	}
	if(typ == 1)
	{
		/**
		* Reset control raster to 0 (rast2), reset only selected cells
		*/
		for(i=0;i<tocelflw;i++)
		{
			xcol = inxdir[i];
			yrow = inydir[i];
			rast1[xcol][yrow] = 0;
			rast2[xcol][yrow] = 0; 
			rast3[xcol][yrow] = 0;
			inxdir[i] = 0;
			inydir[i] = 0;
		}
	}
	if(typ == 2)
	{
		/**
		* Reset control raster to 0 (rast2) Reset whole matrix
		*/
		for(j=0;j<rowy;j++)
		{
			for(i=0;i<colx;i++)
			{
				rast1[i][j] = 0;
				rast3[i][j] = 0;
			}	
		}	
	}
	if(typ == 3)
	{
		/**
		* Reset control raster to 0 (rast2) Reset whole matrix
		*/
		for(j=0;j<rowy;j++)
		{
			for(i=0;i<colx;i++) rast2[i][j] = 0;	
		}	
	}
	if(typ == 4)
	{	
		for(i=0;i<sizearray;i++)
		{
		    inxdir[i] = 0;
			inydir[i] = 0;
		}
	}
	if(typ == 5)
	{
		for(i=0;i<totalpt;i++)
        {
			caucerio[i].cjerar  = 0;
			caucerio[i].crio    = 0;
			caucerio[i].ctramo  = 0;
			caucerio[i].cidpt   = 0; 
			caucerio[i].cdist   = 0;
			caucerio[i].cxcoor  = 0;
			caucerio[i].cycoor  = 0;
			caucerio[i].czcoor  = 0;
			caucerio[i].clong   = 0;
			caucerio[i].clat    = 0;
			caucerio[i].cdx     = 0;
			caucerio[i].cdt     = 0;
			caucerio[i].quality = 0;
		}	
	}			 
}

/*! RANDMON VARIATE GENERATOR */
double box_muller(double m, double s)	/* normal random variate generator */
{				                        /* mean m, standard deviation s */
double x1, x2, w, y1;   
	do
	{
		x1 = 2.0*(double)rand()/(double)RAND_MAX-1.0;
		x2 = 2.0*(double)rand()/(double)RAND_MAX-1.0;
		w  = x1*x1 + x2*x2;
	}while (w >= 1.0);
	w  = sqrt((-2.0*log(w))/w);
	y1 = x1*w;
    return(m + y1*s);
}

/*! RANDMON GAUSS FUNCTION */
void campana(void)
{
int i;
    dlbox = 100 * resx;
    srand(time(NULL));
    for(i=0;i<1000;i++)gauss[i] = (int)(box_muller(0.0,dlbox)/resx);
    dlbox = 100 * 2  * resx;
    for(i=0;i<1000;i++)gaussb[i] = (int)(box_muller(0.0,dlbox)/resx);
}

/*! COORDINATE CONVERSION UTM2DEGREE WGS84 */
int convert_coor(double coorx, double coory)
{
int lambda;
//double alfa, e;
double a, b, a2, b2, eb, c;	
double coorx2, coory2;
double fi, ni, d, A1, A2, J2, J4, J6;
double alfab2, beta, gammag, B, f, dseta, xi, eta, otra, tau;
double latrad;
	/**
	* Init coordinates and UTM grid zone
	*/
	longitud = 0;
	latitud  = 0;
	/**
	* Elipsoide GRS80
	*/
	a = 6378137.0000000000;                                             //Semieje mayor
	b = 6356752.3141400000;                                             //Semieje menor
	a2 = pow(a, 2);
	b2 = pow(b, 2);
	//----------------------
	//e =1-pow((b / a),2);                                              //Excentricidad
	eb = sqrt((a2 - b2)/b);                                             //Segunda Excentricidad
	eb = pow((a / b),2) - 1;
	c =	a2 / b;                                                         //radio polar de Curvatura
	//alfa = (a - b) / b;                                               //aplanamiento o achatamiento
	/**
	* retranqueo o modificacion de coordenadas
	*/
	coorx2 = coorx - 500000;
	if(hemis == 1 )coory2 = coory - 10000000;                             //solo en el caso de operar hemisferios sur
	else coory2 = coory;
	lambda = huso * 6-183;                                              //meridiano central uso
	//--------------
	fi       = coory2 / (6366197.724*0.9996);
	ni       = c / sqrt(1 + (eb * pow(cos(fi),2)))*0.9996;
	A1       = sin(2*fi);
	A2       = A1 * pow(cos(fi),2);
	J2       = fi + (A1 / 2);
	J4       = ((3 * J2) + A2) / 4;
	J6       = ((5 * J4) + (A2 * pow(cos(fi),2))) / 3;
	alfab2    = 3.0 / 4.0 * eb;
	beta     = (5.0 / 3.0) * pow(alfab2, 2);
	gammag   = (35.0 / 27.0) * pow(alfab2, 3); //como una y
	B        = (fi-(alfab2 * J2) + (beta * J4) - (gammag * J6)) * 0.9996 * c;
	d        = coorx2 / ni;       //a
	f        = (coory2 - B) / ni; //b
	dseta    = ((eb * pow(d, 2)) / 2) * pow(cos(fi), 2);//como z
	xi       = d * (1 - dseta / 3); //coomo una e
	eta      = f * (1 - dseta) + fi; //como n
	otra     = atan(sinh(xi)/cos(eta)); //incremento de lambda
	tau      = atan(cos(otra)* tan(eta)); //como una t
	//----------------------------------
	//longrad  = otra + (lambda * PI / 180);
	longitud = (otra / PI * 180) + lambda;
	latrad   = fi + (1 + eb * pow(cos(fi), 2) - (3.0 / 2.0 * eb * sin(fi) * cos(fi)) * (tau - fi)) * (tau - fi);
	latitud  = latrad / PI * 180;

	return 1;
}

/*! GET Z COORDINATES FROM 3X3 MOVING WINDOW */
int getmovingcell(int ix, int jy, int withdif)
{
float hl3;		
	/**
	* Array index named used and 3x3 moving window and equivalent orientation
	* || a b c  || 9  8  7  ||  i-1 j+1 : i j+1 : i+1 j+1  ||  t5  t2 t4  ||  64  128  1
	* || d e f  || 6  5  4  ||  i-1 j   :  i j  : i+1 j    ||  t0  t  t1  ||  32  255  2
	* || g h i  || 3  2  1  ||  i-1 j-1 : i j-1 : i+1 j-1  ||  t6  t3 t7  ||  16    8  4
	*/

	/**
	* Get z values from center and 8 neighbouring cells
	*/
	if (withdif == 0)c5 = topoless[ix][jy];                             /*!< Get z center cell original value (morphometric) */
	if (withdif == 1)c5 = hl+topoless[ix][jy];                          /*!< Get z center cell value with height variation (flow path) */
	if (withdif == 2)
	{
		hl3 = hl + hl2;
		c5 = hl3+topoless[ix][jy];                          /*!< Get z center cell value with height variation (flow path) */
	}	
	//octogonal
	c6 = topoless[ix-1][jy];   //d - 6
	top[0] = c6;
	c4 = topoless[ix+1][jy];   //f - 4
	top[1] = c4;
	c8 = topoless[ix][jy+1];   //b - 8
	top[2] = c8;
	c2 = topoless[ix][jy-1];   //h - 2
	top[3] = c2;
	//oblicuas
	c7 = topoless[ix+1][jy+1]; //c - 7
	top[4] = c7;
	c9 = topoless[ix-1][jy+1]; //a - 9
	top[5] = c9;
	c3 = topoless[ix-1][jy-1]; //g - 3
	top[6] = c3;
	c1 = topoless[ix+1][jy-1]; //i - 1
	top[7] = c1;
	/**
	* Get z-diff from z center cell and its 8 neighboring cells
	*/
	h[0] = c5-c6; 
	h[1] = c5-c4;
	h[2] = c5-c8;
	h[3] = c5-c2;
	h[4] = c5-c7;
	h[5] = c5-c9;
	h[6] = c5-c3;
	h[7] = c5-c1;
	return 1;

}

/*! GET ROW AND COLUM INDEX FROM COORDINATES */
void calc_index(double vx, double vy)
{
double difx, dify, demx, demy;
	
	/**
	* Get col index
	*/
    difx = vx - xmin;                                                   /*!< Calc dif in x */
	vcol = (int)(difx / resx);                                          /*!< Calc col index - first step */
	demx = (vcol * resx) + xmin;                                        /*!< Recalc coordinate again */
	if ((vx - demx) > (resx/2))vcol++;                                  /*!< if difference between original x coordinate and the new one ... */     																  /*!<is less than the middle of resx then increase the col index */
	/**
	* Get row index
	*/
	dify = vy - ymin;
	vrow = (int)(dify / resy);
	demy = (vrow * resy) + ymin;
	if ( (vy - demy) > (resy/2))vrow++;

}

/*! GET X AND Y FROM MATRIX INDEX */
void get_coor(int col, int row)
{
	globx = 0;
	globy = 0;
	/**
	* Get xcoor 
	*/
	globx = xmin + (col * resx);
	/**
	* Get ycoor
	*/
	globy = ymin + (row * resy);

}

/*! DEPRESSION FILL FUNCTION FOR SINGLE PATH MODELS */
void calc_levelfill(int col, int row)
{
int i, j, l, n, celda;	
float hl3;
double h0, h2[8], alt;
	
	for(i=0;i<8;i++)h2[i]=0;
	/**
	* Change Critical Height if necessary
	*/
    hl3 = hl + hl2;
	h0  = hl3+topoless[col][row];                                       /*!< New critical height */
	/**
	* check if neighboring cells were previously selected 
	* if not, recalc z diff
	*/
	if(rast2[col-1][row]  == 0)h2[0] = h0-topoless[col-1][row]; //c6
	if(rast2[col-1][row]  >= 1)h2[0] = 0;
	if(rast2[col+1][row]  == 0)h2[1] = h0-topoless[col+1][row]; //c4
	if(rast2[col+1][row]  >= 1)h2[1] = 0;
	if(rast2[col][row+1]  == 0)h2[2] = h0-topoless[col][row+1]; //c8
	if(rast2[col][row+1]  >= 1)h2[2] = 0;
	if(rast2[col][row-1]  == 0)h2[3] = h0-topoless[col][row-1]; //c2
	if(rast2[col][row-1]  >= 1)h2[3] = 0;
	//---
	if(rast2[col+1][row+1] == 0)h2[4] = h0-topoless[col+1][row+1]; //c7
	if(rast2[col+1][row+1] >= 1)h2[4] = 0;
	if(rast2[col-1][row+1] == 0)h2[5] = h0-topoless[col-1][row+1]; //c9
	if(rast2[col-1][row+1] >= 1)h2[5] = 0;
	if(rast2[col-1][row-1] == 0)h2[6] = h0-topoless[col-1][row-1]; //c3
	if(rast2[col-1][row-1] >= 1)h2[6] = 0;
	if(rast2[col+1][row-1] == 0)h2[7] = h0-topoless[col+1][row-1]; //c1
	if(rast2[col+1][row-1] >= 1)h2[7] = 0;
	j    = 0;
	n    = 0;
	sum1 = 0;
	alt  = 0;
	for(l=0;l<8;l++)
	{
		/**
		* Check surface type
		*/
		if(h2[l]==0)	j++;                                            /*!< if cell used or flats - count */
		if(h2[l] <0)	n++; 	                                        /*!< if Surface depression - count */
		/**
		* Get new xy using LHM
		*/
		if(flowtyp == 1) 
		{
			if(h2[l]>0)                                                 /*!< if z diff is higher than 0 */
			{
				sum1 += h2[l];                                          /*!< Sum all z-diff higher than 0 */
				/**
				* Get max z-diff value
				*/
				if(h2[l] > alt)                                        
				{
					alt = h2[l];                                        /*!< Set max value */
					celda = l;                                          /*!< Set cell number */
				}	
			}
		}
		/**
		* Get new xy using SSM
		*/
		if(flowtyp == 2)
		{
			if(h2[l]>0)                                                 /*!< if z diff is higher than 0 */
			{
				sum1 += h2[l];                                          /*!< Sum all z-diff higher than 0 */
				/**
				* Depending on neighboring cell position
				*/
				if(l <  4)h2[l] = h2[l] / (resx / 3);                   /*!< Get slope-gradient */
				if(l >= 4)h2[l] = h2[l] / (resx / 2);
				/**
				* Get max s-gradient value
				*/
				if(h2[l] > alt) 
				{
					alt = h2[l];                                        /*!< Set max value */
					celda = l;                                          /*!< Set cell number */
				}	
			}
		}	
	}
	/**
	* According to surface type, set sum1 var
	*/
	if(n+j == 8) sum1 = -3;                                             /*!< Mix surface flat-sink */
	if(j == 8)   sum1 = -1;                                             /*!< Mix surface all_cell_used-flat */
	if(n == 8)   sum1 = -2;                                             /*!< Sink */
	/**
	* Set new cell array index
	*/
	switch(celda) 
		{
		case 0:
			col--;
			break;
		case 1:
			col++;
			break;
		case 2:
			row++;
			break;
		case 3:
			row--;
			break;
		//--------	
		case 4:
			row++;
			col++;
			break;
		case 5:
			col--;
			row++;
			break;
		case 6:
			row--;
			col--;
			break;
		case 7:
			col++;
			row--;
			break;
		}
		/**
		* Set global variables
		*/
		col1 = col;
		row1 = row;
		/**
		* Calc distance between center cell and selected cell
		*/
		if(celda <  4) distpt = resx;
		if(celda >= 4) distpt = resx + (resx / 2);
}



/*! DEPRESSION FILL FUNCTION FOR RAMDOM PATH MODEL */
void calc_levelfill2(int col, int row, int fcr)
{
int i, j, l;
int r1, r2, c1, c2, buq;	
double h0, h2[8], hm[8], med, summ;
	
	buq = 0;
	for(i=0;i<8;i++)
	{
		h2[i]=0;
		/**
		*Check last selected cells, if they are the same, end filling depression
		*/
		c1 = coin[i];
		r1 = roin[i];
		for(j=0;j<8;j++)
		{   
			c2 = coin[j];
			r2 = roin[j];
			if(i != j)
			{
				if(c1 == c2 && r1 == r2) buq++;
			}
		}
	}
	/**
	* Calc media value
	*/
	summ = 0;
	hm[0] = topoless[col-1][row]; //c6
	hm[1] = topoless[col+1][row]; //c4
	hm[2] = topoless[col][row+1]; //c8
	hm[3] = topoless[col][row-1]; //c2
	hm[4] = topoless[col+1][row+1]; //c7
	hm[5] = topoless[col-1][row+1]; //c9
	hm[6] = topoless[col-1][row-1]; //c3
	hm[7] = topoless[col+1][row-1]; //c1
	for(i=0;i<8;i++) summ += hm[i];
	med = summ / 8;
	h0  = med;
	topoless[col][row] = med;                   /*!< Change z value in DEM  */
	/**
	* Calc z diferences value
	*/
	h2[0] = h0-topoless[col-1][row]; //c6
	h2[1] = h0-topoless[col+1][row]; //c4
	h2[2] = h0-topoless[col][row+1]; //c8
	h2[3] = h0-topoless[col][row-1]; //c2
	h2[4] = h0-topoless[col+1][row+1]; //c7
	h2[5] = h0-topoless[col-1][row+1]; //c9
	h2[6] = h0-topoless[col-1][row-1]; //c3
	h2[7] = h0-topoless[col+1][row-1]; //c1
	
	//---
	if (buq < 4)         /*!< if repited cells are less than  */
	{
		sum1 = 0;
		for(l=0;l<8;l++)
		{
			if(h2[l]>0)                                                 /*!< if z diff is higher than 0 */
			{
				sum1 += h2[l];                                          /*!< Sum all z-diff higher than 0 */	
			}
			h[l] = h2[l];	
		}
	}
	if(sum1 < 0) sum1 = -2;         /*!< if still sink */
	if(buq  > 4) sum1 = -1;	        /*!< if selected cells higher than .. finish filling depression */
}

/*! DETECT SURFACE DEPRESSION */
int calc_isosink(int coli, int rowj)
{
int i, l, n;
	l = 0;
	n = 0;
	result  = 0;
	higcell = 0;
	iqucell = 0;
	for(i=0;i<8;i++)
	{
		if(top[i] >  c5)l++; //contamos cuantas celdas tienen una cota superior
		if(top[i] == c5)n++; //contamos cuantas celdas tienen una cota igual
	}
	higcell = l;
	iqucell = n;
	result  = l + n;
	return result;
}	

/*! FIFO INIT FUNCTION */
void fifoini(void)
{
int i;
    /**
	* FIFO initialitation. 
	*/
    for(i=0;i<SIZE;i++) 
    {
        FIFO[i][0] = -1;                                                /*!< -1 means empty cell */
        FIFO[i][1] = -1;
    }
}
//----------------------------------------------------------------------
/*! FIFO SUBTRACT FUNCTION */
void fifodesplaza(void)
{
int i;
    i = 0;
    /**
	* Substract values from FIFO and recal its index. 
	*/
    do
    {
        FIFO[i][0] = FIFO[i+1][0];
        FIFO[i][1] = FIFO[i+1][1];
        i++;
    }while(FIFO[i][0] != -1);
    FIFO[SIZE-1][0] = -1;
    FIFO[SIZE-1][1] = -1;
    puntero = i-1;                                                      /*!< 1 must be subtracted to avoid the insertion of -1 values in the middle */
}
//----------------------------------------------------------------------
/*! FIFO LOAD FUNCTION */
void fifocarga(void)
{
    /**
	* Add values to FIFO 
	*/
    if(puntero < SIZE)
    {
        FIFO[puntero][0] = fcolm;
        FIFO[puntero][1] = frowm;
        puntero ++;
    }
}
//----------------------------------------------------------------------

/*!CALL WRITING RASTER ACCORDING TO MODEL */
void call_writeraster(void)
{
int i,j;
	/**
	* Write the ouput raster files according to the flow path algorithm selected
	*/
	if(flowtyp == 1)
	{
		maxval = maximun;                                           /*!< Assign z max value for raster head data */
		chksing3 = write_rast1(10, mod);                                 /*!< Write flow path in sum mode */
		maxval = 1;                                                 /*!< Assign z max value for raster head data */
		chksing4 = write_rast3(10, mod);                                 /*!< Write flow path */
		if(chksing3 == 1 || chksing4 == 1)chksum++;
	}
	if(flowtyp == 2)
	{
		maxval = maximun;                                           /*!< Assign z max value for raster head data */
		chksing3 = write_rast1(11, mod);                                 /*!< Write flow path in sum mode */
		maxval = 1;                                                 /*!< Assign z max value for raster head data */
		chksing4 = write_rast3(11, mod);                                 /*!< Write flow path */
		if(chksing3 == 1 || chksing4 == 1)chksum++;
	}
	if(flowtyp == 3)	
	{
		/**
		* Reading rast3 matrix to change log scale
		*/
		for(j=0;j<rowy;j++)
		{
			for(i=0;i<colx;i++)
			{
				/**
				* Change to logarithmic scale
				*/
				if(rast3[i][j]>0)rast3[i][j] = log(rast3[i][j]);        /*!< changing rast3 value */	
			}	
		}
		maxval = 1;                                                 /*!< Assign z max value for raster head data */
		chksing3 = write_rast1(12, mod);                                 /*!< Write Montecarlo flow path */
		chksing4 = write_rast3(12, mod);                                 /*!< Write Montecarlo flow path Logscale */
		if(chksing3 == 1 || chksing4 == 1)chksum++;
	}
	if(flowtyp == 4)	
	{
		maxval = 1;                                                     /*!< Assign z max value for raster head data */
		chksing3 = write_rast1(13, mod);                                /*!< Write Multiflow path */
		maxval = maximun;                                           
		chksing4 = write_rast3(13, mod);                                /*!< Write Multiflow path Sum*/ 
		if(chksing3 == 1 || chksing4 == 1)chksum++;
	}


}

//---------------------------------------------------------------------------------------
//***************************************************************************************
//***************************END AUXILIAR FUNCTIONS**************************************
//***************************************************************************************
//*************************DEM MODIFICATION SECTION**************************************
//***************************************************************************************
//---------------------------------------------------------------------------------------

/*! STEP 1, GET XYZ FROM VALID CELLS */
int getnpt(void)
{
FILE *file;	
int i, j, k;
double tx, ty, tz, tnew, supmask;
	k=0;                                                                /*!< Count selected cells */
	/**
	* Assign filename and open file 
	*/
	sprintf(nom_mask, "%sMod_xyz_step1.txt", dir_out);                  /*!< Assign output filename */
	if((file = fopen(nom_mask,"wt"))== NULL)
	{
		printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        return 1;
    }
	else
    {
		for(j=0;j<row2y;j++) //column
		{
			for(i=0;i<col2x;i++) //row
			{
				/**
				* Get cell values. 
				*/
				tx = xmin + (i*resx);         /*!< Convert array index in coordinates */
				ty = ymin + (j*resy);
				tz = topo[i][j];
				tnew = mask[i][j];
				if(tnew > 0)                   /*!< if mask value is higher than 0 */
				{
					/**
					* Write directly in the output txt file
					* The structure of the output file has the column for the new z coordinate
					*/
					if(k==0)fprintf(file,"%s\n", "XCORR YCOOR OLDZCOOR NEWZCOOR");
					fprintf(file, "%lf %lf %lf %lf\n",
						tx,
						ty,
						tz,
						0.0
						);
					k++;	
				}
			}		
		}
	}	
	totmask = k;
	supmask = k * resy2 * resx2;
	printf("Total points/cell selected %i\n", totmask);
	printf("Total surface %.2lf m2\n", supmask);
	return 0;
}	

/*! STEP 2, CHANGE THE OLD Z BY THE NEW ONE */
int readchangenewz(void)
{
FILE *file;
char texto[256];
int i, j;
double txcorr, tycorr, tzoldcorr, tznewcorr;
	i = 0;                                                              /*!< count used points */
	j = 0;                                                              /*!< count read points */
	volummask = 0;
	/**
	* The output filename in step 1 must be kept 
	*/
	printf("\n***Reading new z point file, %s***\n", name_newz);
	if ((file = fopen(name_newz,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        return 1;
    }
    else
	{
		/**
		* Reading the yyzz file
		*/
		fgets(texto,256,file);
		while (fscanf(file,"%lf %lf %lf %lf", 
                &txcorr,
                &tycorr,
                &tzoldcorr,
                &tznewcorr       
                ) == 4)
        {   
            /**
			* Check if the coordinate i inside the array dimension
			*/
            if((txcorr > xmin2 && txcorr < xmax2) && (tycorr > ymin2 && tycorr < ymax2))  
            {
				calc_index(txcorr, tycorr);                             /*!< Get the array index from coordinates */
				topoless[vcol][vrow]   = tznewcorr;                     /*!< Change the z value in topoless array */
				volummask += fabs(tzoldcorr - tznewcorr) * resx * resy; /*!< Calc volume */
				i++;
			}
			j++;	
        }     
	}
	fclose(file);
	if (j == 0) return 1;                                               /*!< if no points are read */
	totmask = j;                                                        /*!< total read points and z changed */
	totmodmask = i;                                                     /*!< total available points  */
    supmask = i * resx * resy;                                          /*!< total surface changed  */
	supmodmask =  j * resx * resy;                                      /*!< total surface planed  */
    printf("Total modified cells %i\n", totmask);
	printf("Total available cells found %i\n", totmodmask);
	printf("Surface changed %.4lf m2\n", supmask);
	printf("Surface estimated to be changed %.4lf m2\n", supmodmask);
	printf("Volume changed %.4lf m3\n", volummask);                     /*!< total volume altered */
	maxval = zhi;                                                       /*!< Assign z max value for raster head data */
	chkmod3 = write_newdem(0);                                          /*!< Write new DEM */
	if(modidem == 1 && fase == 2)
	{
		if(totmask != totmodmask)chksum++;       
	}	
	return 0;
}	

//---------------------------------------------------------------------------------------
//***************************************************************************************
//**************************END DEM MODIFICATION SECTION*********************************
//***************************************************************************************
//****************************MORPHOMETRIC SECTION***************************************
//***************************************************************************************
//---------------------------------------------------------------------------------------

/*! DETECT OR MODIFY SURFACE DEPRESSION (write on topoless)*/
int fix_sinks(void)
{
FILE *file;	
int i, j, k, l, m, o, q, ci, rj;
int tisloate;
double h0,  diff;
double txcoor, tycoor;
double dzdx, dzdy, aspect, cell, celda;
float nrun;
	
	srand ( time(NULL) );
	if(metsink == 1)  printf("%s\n", wrst(22));
	if(metsink == 2)  printf("%s\n", wrst(23));
	m = 0;                                                              /*!< count surface depressions */
	sprintf(nom_sink, "%sSink_pt.xyz", dir_out);                        /*!< Assign output filename */
    if((file = fopen(nom_sink,"wt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        return 1;
    }
    else
    {
		/**
		* Print the head file 
		*/
		fprintf(file,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
				 "id", "xcoor","ycoor", "zcoorold", "zcoornew", "diffz", "mode", "iso", "ptim1j","pti1j","ptij1","ptijm1","pti1j1","ptim1j1","ptim1jm1","pti1jm1"); 
				 
		for(j=0;j<rowy;j++) //column
		{
			for(i=0;i<colx;i++) //row
			{
				if(( i>3 && i<colx-3 ) && ( j>3 && j<rowy-3 ))          /*!< if array index are in working area */
				{
					h0  = topo[i][j];                                   /*!< get z coordinate value */
					if(h0 != nullval)                                   /*!< if z coordinate value is not null */
					{
						getmovingcell(i, j, 0);                         /*!< get 3x3 moving cell z values */
						calc_isosink(i, j);                             /*!< check if cell is a sink */
						l = 0;                                          /*!< count neighboring cells with z value higher */
						q = 0;                                          /*!< modification mode used */
						if(result == 8)                                 /*!< if center cell is a sink */
						{	
							if(metsink == 1)                            /*!< if algorithm detection is activated */
							{
								diff = -999;
								q    = -999;
								tisloate = -999;
							}	
							if(metsink == 2)                            /*!< if algorithm correction is activated */
							{
								/**
								* Check if there is not another sink in the 16 neighboring cells 
								*/
								for(k=0;k<8;k++)
								{
									if (k == 0)                         /*!< if z-diff in zero is higher than 0 */
									{
										ci = i--;                       /*!< Change column index */
										rj = j;                         /*!< Change column index */
									}
									if (k == 1)
									{
										ci = i++;                                
										rj = j;                                  
									}
									if (k == 2)
									{
										ci = i;                                  
										rj = j++;                                
									}
									if (k == 3)
									{
										ci = i;                                  
										rj = j--;                                
									}
									if (k == 4)
									{
										ci = i++;                                
										rj = j++;                                
									}
									if (k == 5)
									{
										ci = i--;                                
										rj = j++;                                
									}
									if (k == 6)
									{
										ci = i--;                                
										rj = j--;                                
									}
									if (k == 7)
									{
										ci = i++;                                
										rj = j--;                                
									}
									calc_isosink(ci, rj);               /*!< check if neighboring cell is a sink */
									if (result == 8) l++; 
								}	
								if (l  >  0) tisloate = 0;              /*!< sink cell not isolate */
								if (l  == 0) tisloate = 1;              /*!< isolate sink cell */
								if (tisloate == 1)
								{
									getmovingcell(i, j, 0);                /*!< get 3x3 moving cell z values */
									/**
									* Using the SSM to get the spill cell 
									* http://help.arcgis.com/en/arcgisdesktop/10.0/help/index.html#//00q900000023000000
									* Burrough and McDonell (1998)
									* dzdx = ((c + 2f + i) - (a + 2d + g)) / 8
									* dzdy = ((g + 2h + i) - (a + 2b + c)) / 8	
									*/	
									dzdx = ((c7 + (2*c4) + c1) - (c9 + (2*c6) + c3)) / 8;	
									dzdy = ((c3 + (2*c2) + c1) - (c9 + (2*c8) + c7)) / 8;  	
									aspect = 180/PI * atan2 (dzdy, - dzdx);   /*!< calc s-aspect in degrees */
									/**
									* Change degree values starting from North as 0 
									*/
									if (aspect >= 0 && aspect <= 90) cell = 90 - aspect; 
									if (aspect > 90 && aspect <= 180) cell = 360 - (aspect - 90);
									if (aspect >= -180 && aspect < 0) cell =  (aspect*-1) + 90;
									/**
									* Getting the spill cell z value 
									*/
									if (cell >=     0 && cell <   22.5) celda = top[2];
									if (cell >=  22.5 && cell <   67.5) celda = top[4];
									if (cell >=  67.5 && cell <  112.5) celda = top[1];
									if (cell >= 112.5 && cell <  157.5) celda = top[7];
									if (cell >= 157.5 && cell <  202.5) celda = top[3];
									if (cell >= 202.5 && cell <  247.5) celda = top[6];
									if (cell >= 247.5 && cell <  292.5) celda = top[0];
									if (cell >= 292.5 && cell <  337.5) celda = top[5];
									if (cell >= 337.5 && cell <= 360.0) celda = top[2];
									/**
									* if neighboring cell is lower than the center cell 
									*/
									if(h0 > celda)
									{
										topoless[i][j] = celda + 0.1;   /*!< exchange z value + 0.1 */
										diff = topoless[i][j] - h0;
										q = 1;
									}
									/**
									* if neighboring cell is equal than the center cell 
									*/
									if(celda == h0)
									{
										nrun = (float) (rand()/ (float)RAND_MAX); 
										topoless[i][j] = h0 + nrun;     /*!< use a random value */
										diff = nrun;
										q = 2;
									}	
									/**
									* Check if the new center cell produce new surface depressions around (16 neighboring cells) 
									*/
									o = 0;
									for(k=0;k<8;k++)
									{
										if (k == 0)                     /*!< if z-diff in zero is higher than 0 */
										{
											ci = i--;                   /*!< Change column index */
											rj = j;                     /*!< Change column index */
										}
										if (k == 1)
										{
											ci = i++;                                
											rj = j;                                  
										}
										if (k == 2)
										{
											ci = i;                                  
											rj = j++;                                
										}
										if (k == 3)
										{
											ci = i;                                  
											rj = j--;                                
										}
										if (k == 4)
										{
											ci = i++;                                
											rj = j++;                                
										}
										if (k == 5)
										{
											ci = i--;                                
											rj = j++;                                
										}
										if (k == 6)
										{
											ci = i--;                                
											rj = j--;                                
										}
										if (k == 7)
										{
											ci = i++;                                
											rj = j--;                                
										}	
										calc_isosink(ci, rj);
										if (result == 8) o++;           /*!< if there is a new sink */
									}	
									if(o > 0)                           /*!< if so */
									{
										topoless[i][j] = h0;            /*!< do not change the center z value */
										diff = 0;
										q = 0;
									}
									/**
									* For debugging
									* printf("%i %i  -N %i Q %i-  %lf  %lf  %lf\n", i,  j, nalt, q, h0, topoless[i][j], diff);
									*/	
								}	
								if (tisloate == 0)                      /*!< if sink cell is not isolate */
								{
									diff = 0;                           /*!< do not change the z value */
									q    = 0;
								}
							}
							/**
							* Write surface depressions in the output file 
							*/
							txcoor = (i * resx) + xmin;                 /*!< get coordinate from array index */
							tycoor = (j * resy) + ymin;
							fprintf(file,"%i %lf %lf %lf %lf %lf %i %i %.4lf %.4lf %.4lf %.4lf %.4lf %.4lf %.4lf %.4lf\n",
								m,
								txcoor,
								tycoor,
								h0,                                     /*!< original z value */
								topoless[i][j],                         /*!< new z value */
								diff,                                   /*!< difference between z values */
								q,
								tisloate,
								top[0],                                 /*!< i-- j   ptim1j */
								top[1],                                 /*!< i++ j   pti1j */
								top[2],                                 /*!< i j++   ptij1 */
								top[3],                                 /*!< i j--   ptijm1 */
								top[4],                                 /*!< i++ j++ pti1j1 */
								top[5],                                 /*!< i-- j++ ptim1j1 */
								top[6],                                 /*!< i-- j-- ptim1jm1 */
								top[7]                                  /*!< i++ j-- pti1jm1 */
								);
							m++;
						}
					}
				}
			}
		}
	}									
	totalsink = m;
	printf("Total surface depressions found %i\n", m);
	maxval = zhi;                                                       /*!< Assign z max value for raster head data */	
	if(metsink == 2) 
	{
		chksink2 = write_newdem(1);                                     /*!< if modify method is activated, write the new DEM */
		if (chksink2 == 1)chksum++;                                     /*!< if error creating output newdem file */	
	}	
	return 0;	
	
}

/*! SLOPE-ASPECT CALCULATION (write on direct and rast2) */
int calc_saspect(void)
{
FILE *file;
char nom_hist[256];	
int i, j, k, n;
int celda;
double h0, alt, superf, aspect, cell, aspor[10];
double dzdx, dzdy;
	
	printf("Calculing slope-Aspect method %i\n", metasp);
	for(k=0;k<10;k++)
	{
		cuenta[k] = 0;                                    /*!< Reset values, count cells by s-aspect class */
		aspor[k]  = 0; 
	}	
	for(j=0;j<rowy;j++) //column
    {
        for(i=0;i<colx;i++) //row
        {
			alt = 0;
			n   = 0;                                                    /*!< Count if cell has slope-aspect value  */
			h0  = topoless[i][j];                                       /*!< Get center cell z value  */
			if(( i>3 && i<colx-3 ) && ( j>3 && j<rowy-3 ))              /*!< if array index are inside working area  */
			{
				if(h0 != nullval)                                       /*!< if z coordinate value is not null */     
				{
					getmovingcell(i, j, 0);                    			/*!< get 3x3 moving cell z values */		
					/**
					* Using the LHM 
					*/
					if(metasp == 1)                                     
					{
						for(k=0;k<8;k++)
						{
							if(h[k] > 0)                                /*!< if z difference is higher than 0  */ 
							{
								if(h[k] > alt)                          /*!< if z difference is the higher value */ 
								{
									alt = h[k];
									celda = k;                            /*!< Get index */ 
									n++;
								}
							}
						}
					}
					/**
					* Using the SSM 
					* http://help.arcgis.com/en/arcgisdesktop/10.0/help/index.html#//00q900000023000000
					* Burrough and McDonell (1998)
					* dzdx = ((c + 2f + i) - (a + 2d + g)) / 8
					* dzdy = ((g + 2h + i) - (a + 2b + c)) / 8	
					*/	
					if(metasp == 2)                                    
					{	
						dzdx = ((c7 + (2*c4) + c1) - (c9 + (2*c6) + c3)) / 8;	
						dzdy = ((c3 + (2*c2) + c1) - (c9 + (2*c8) + c7)) / 8;  	
						aspect = 180/PI * atan2 (dzdy, - dzdx); 		/*!< calc s-aspect in degrees  0 located in East*/
						/**
						* Change degree values starting from North as 0 
						*/
						if (aspect >= 0 && aspect <= 90) cell = 90 - aspect; 
						if (aspect > 90 && aspect <= 180) cell = 360 - (aspect - 90);
						if (aspect >= -180 && aspect < 0) cell =  (aspect*-1) + 90;
						rast1[i][j] = cell;                             /*!< Save s-aspect in degrees */
						if (cell >=     0 && cell <   22.5) celda = 2;
						if (cell >=  22.5 && cell <   67.5) celda = 4;
						if (cell >=  67.5 && cell <  112.5) celda = 1;
						if (cell >= 112.5 && cell <  157.5) celda = 7;
						if (cell >= 157.5 && cell <  202.5) celda = 3;
						if (cell >= 202.5 && cell <  247.5) celda = 6;
						if (cell >= 247.5 && cell <  292.5) celda = 0;
						if (cell >= 292.5 && cell <  337.5) celda = 5;
						if (cell >= 337.5 && cell <= 360.0) celda = 2;
						n++;	
					}					
					if(n == 0)                                          /*!< if cell is flat  */ 
					{
						rast2[i][j] = 255;                              /*!< Save s-aspect in class */
						cuenta[8]++; 
					}	
					if(n > 0) 											/*!< if cell is not flat or null  */
					{
						switch(celda) 									/*!< get the spill neighboring value cell  */
						{
						case 0: 
							rast2[i][j] = 32; 							/*!< Save s-aspect in class */
							cuenta[5]++;                                /*!< Count total cells with class 128 */
							break;
						case 1:
							rast2[i][j] = 2; 
							cuenta[1]++;
							break;
						case 2:
							rast2[i][j] = 128; 
							cuenta[7]++;
							break;
						case 3:
							rast2[i][j] = 8;
							cuenta[3]++;
							break;
						case 4:
							rast2[i][j] = 1;
							cuenta[0]++;
							break;
						case 5:
							rast2[i][j] = 64;
							cuenta[6]++;
							break;
						case 6:
							rast2[i][j] = 16; 
							cuenta[4]++;
							break;
						case 7:
							rast2[i][j] = 4; 
							cuenta[2]++;
							break;
						}	
					}				
				}
				if(h0 == nullval)                                       /*!< if cell is null  */
				{
					rast2[i][j] = nullval;                              /*!< The null valued is transfered  */
					rast1[i][j] = nullval;	 
					cuenta[9]++;
				}	
			}
		}			
	}
	maxval = 255;                                                       /*!< Assign z max value for raster head data */
	chkasp1 = write_rast2(2, 0);                                        /*!< Write S-aspect class */
	if(chkasp1 == 1)chksum++;                                           /*!< if error creating output rast2 file */
	maxval = 360;                                                       /*!< Assign z max value for raster head data */
	if(metasp == 2)
	{
		write_rast1(2, 0);	                                            /*!< S-aspect degree raster is only written in mode 2  */
		if(chkasp2 == 1)chksum++;                                       /*!< if error creating output rast1 file */
	}
	int* val = (int[10]){1,2,4,8,16,32,64,128,255,-9999};               /*!< array with class values  */	
	/**
	* Write histograms
	*/
	printf("\nWrite histograms - slope-aspect %i\n", metasp);
	i = 0;
	sprintf(nom_hist, "%sS-aspect_Histo_%i.txt", dirfinout, metasp);
    if((file = fopen(nom_hist,"w"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
    }
    else
    {	 
	    fprintf(file,"%s\n",
	         "id freq perc surf"); //primera linea
        for(i=0;i<10;i++)
        {
			/**
			* Calc normalized values
			*/
			if(cuenta[i] == 0)aspor[i]=0;
			if(cuenta[i]  > 0)aspor[i]= (double)cuenta[i] / (double)totdemcel;
			superf = (cuenta[i] * resx * resy)/1000000;					/*!< Calc total surface by s-aspect class in km2 */
			printf("Total cells %i, surface %.2lf km2 with s-aspect %i\n", cuenta[i], superf, val[i]);
			/**
			* Write final values
			*/
			fprintf(file,"%i %i %lf %lf\n",
				i,
				cuenta[i],
				aspor[i],
				superf
				);
		}	
    } 
    fclose(file);
    return 0;
}		

/*! SLOPE-GRADIENT CALCULATION (write on rast1 rast2 rast3) */
int calc_sgrad(void)
{
FILE *file;
char nom_hist[256];	
int i, j, k, degslo[19];
double h0, topo[8], degpor[19];
double fx, fy, fx2, fy2, rise;
double slprad, slpdeg, superf;
double inival, finx, finz;	
	
	printf("Calculating slope-gradient method %i\n", metslop);
	/**
	* Reset to 0 to calculate the histogram
	*/
	for(i=0;i<19;i++)
	{
		degslo[i] = 0;
		degpor[i] = 0;
	}
	/**
	* Start slope calculation
	*/	
	for(j=0;j<rowy;j++)
    {
        for(i=0;i<colx;i++)
        {
			if(( i>3 && i<colx-3 ) && ( j>3 && j<rowy-3 ))              /*!< if array index are in working area */
			{
				h0  = topoless[i][j];                                   /*!< Get center cell z value  */
				if(h0 != nullval)                                       /*!< if z coordinate value is not null */
				{                 
					getmovingcell(i, j, 0);                                /*!< get 3x3 moving cell z values */
					/**
					* Using the SSM 
					* http://help.arcgis.com/en/arcgisdesktop/10.0/help/index.html#//00q900000023000000
					* Burrough and McDonell (1998)
					* dzdx = ((c + 2f + i) - (a + 2d + g)) / 8 * g
					* dzdy = ((g + 2h + i) - (a + 2b + c)) / 8 * g 
					*/
					if(metslop == 1)
					{
						fx = ((c7 + (2*c4) + c1) - (c9 + (2*c6) + c3)) / (8 * resx);
						fy = ((c3 + (2*c2) + c1) - (c9 + (2*c8) + c7)) / (8 * resy);
					}
					/**
					* Second-order finite difference 2FD 
					* //https://www.witpress.com/Secure/elibrary/papers/RM11/RM11013FU1.pdf
					* Fleming and Hoffer (1979)
					* fx=(z6-z4)/2g  
					* fy=(z8-z2)/2g  
					*/
					if(metslop == 2)
					{	
                        fx = (c6 - c4) / (2*resx);
                        fy = (c8 - c2) / (2*resy);
					}
					/**
					* Three-order  Finite  Difference  Weighted  by  Reciprocal  of  Distance  3FDWRD
					* //https://www.witpress.com/Secure/elibrary/papers/RM11/RM11013FU1.pdf
					* Unwin (1981)
					* fx=(z3-z1+√2(z6-z4)+z9-z7)/(4+2√2)g   
					* fy=(z8-z2)/2g 
					*/	
					if(metslop == 3)
					{	 
                        fx = ((c3 - c1) + (sqrt(2)*(c6-c4)) + (c9 - c7)) / ((4 + (2*sqrt(2)) )* resx);
                        fy = (c8 - c2) / (2*resy);  
                        
					}	
					/**
					* Three-order  Finite  Difference,  Linear  regression  plan  3FD
					* //https://www.witpress.com/Secure/elibrary/papers/RM11/RM11013FU1.pdf
					* Sharpnack et al (1969)
					* fx=(z3-z1+z6-z4+z9-z7)/6*g  
					* fy=(z7-z1+z8-z2+z9-z3)/6*g 
					*/		
					if(metslop == 4)
					{		
						fx = (((((c3-c1)+c6)-c4)+c9)-c7) / (6*resx);
						fy = (((((c7-c1)+c8)-c2)+c9)-c3) / (6*resy);
					}
					/**
					* Three-order  Finite  Difference  Weighted  by  Reciprocal  of  Squared  Distance 3FDWRSD
					* //https://www.witpress.com/Secure/elibrary/papers/RM11/RM11013FU1.pdf
					* Horn (1981)
					* fx=(z3-z1+2 (z6-z4)+z9-z7/8*g  
					* fy=(z7-z1+2(z8-z2)+z9-z3)/8*g 
					*/
					if(metslop == 5)
					{ 
						fx = ((c3-c1)+(2*(c6-c4))+(c9-c7)) / (8*resx);
						fy = ((c7-c1)+(2*(c8-c2))+(c9-c3)) / (8*resy);	
					}
					/**
					* Frame Finite difference FFD
					* //https://www.witpress.com/Secure/elibrary/papers/RM11/RM11013FU1.pdf
					* Chu and Tsai (1995)
					* fx=(z3-z1+z9-z7)/4*g 
					* fy=(z7-z1+z9-z3)/4*g
					*/
					if(metslop == 6)
					{	
						fx=(c3-c1+c9-c7)/(4*resx);
						fy=(c7-c1+c9-c3)/(4*resy);
					}
					/**
					* Maximum Max
					* https://babel.hathitrust.org/cgi/pt?id=umn.31951d029867321;view=1up;seq=17
					* Travis et al. (1975)
					* max(abs((z5-z1)/(√2×g)),abs((z5-z2)/g), abs((z5-z3)/(√2×g)),abs((z5-z9)/(√2×g)), 
					* abs((z5-z7)/(√2×g)),abs((z5-z6)/g), abs((z5-z8)/g), abs((z5-z4)/g))
					*/
					if(metslop == 7)
					{	
						topo[0] = fabs((c5-c1)/(sqrt(2)*resx));
						topo[1] = fabs((c5-c2)/resx);
						topo[2] = fabs((c5-c3)/(sqrt(2)*resx));
						topo[3] = fabs((c5-c9)/(sqrt(2)*resx)); 
						topo[4] = fabs((c5-c7)/(sqrt(2)*resx));
						topo[5] = fabs((c5-c6)/resx);
						topo[6] = fabs((c5-c8)/resx); 
						topo[7] = fabs((c5-c4)/resx);
						inival = 0.0;
						for(k=0;k<8;k++)
						{	
							if(topo[k] > inival)
							{
								if(k==0||k==2||k==3||k==4)finx = sqrt(2)*resx;
								if(k==1||k==5||k==6||k==7)finx = resx;
								if(k==0)finz = fabs(c5-c1);
								if(k==1)finz = fabs(c5-c2);
								if(k==2)finz = fabs(c5-c3);
								if(k==3)finz = fabs(c5-c9);
								if(k==4)finz = fabs(c5-c7);
								if(k==5)finz = fabs(c5-c6);
								if(k==6)finz = fabs(c5-c8);
								if(k==7)finz = fabs(c5-c4);
							}	
						}
						fx = finx; //distance
						fy = finz; //dif height
					}
					/**
					* Simple difference Simple-D
					* //https://www.witpress.com/Secure/elibrary/papers/RM11/RM11013FU1.pdf
					* Jones (1998)
					* fx=(z5-z4)/g
					* fy=(z5-z2)/g
					*/	                                    
					if(metslop == 8)
					{	
						fx=(c5-c4)/resx;
						fy=(c5-c2)/resy;	
					}
					/**
					* Constrained Quadratic Surface Quadsurface (Wood Evans [13])
					* //https://www.witpress.com/Secure/elibrary/papers/RM11/RM11013FU1.pdf
					* Wood (1996)
					* F(x,y)=ax^2 +by^2 +cxy+dx+ey+f
					* δz/δy =2by + cx + e                                
                    * si x,y = 0
                    * δz/δx = d
                    * δz/δy = e 
                    * D = [(c6 + c4) /2 - c5] / resx^2
					* E = [(c8 + c2) /2 - c5] / resy^2
					*/
					if(metslop == 9)
					{	
						fx=(((c6 + c4) /2) - c5) / pow(resx,2);
						fy=(((c8 + c2) /2) - c5) / pow(resy,2);
                    }    
                    /**
					* For debugging
					* printf("x %.2lf y %.2lf\n", fx, fy);
					*/	
                    fx2 = pow(fx, 2);
					fy2 = pow(fy, 2);
                    rise = sqrt((fx2 + fy2));
                    if(metslop < 7 || metslop > 8)
                    {
						slprad = atan (rise);                           /*!< Calculating s-gradient in radians */
					}	
					if(metslop == 7)
					{
						slprad = atan (fy / fx);                        /*!< Calculating s-gradient in radians */
					}
					slpdeg = slprad * (180 / PI);                       /*!< Calculating s-gradient in degrees  */ 
					rast1[i][j] = slpdeg;                               /*!< Saving s-gradient in degrees  */
					rast3[i][j] = slpdeg * 100 / 45;                    /*!< Saving s-gradient in percentages  */
					/**
					* Classifying s-gradient and saving results
					*/
					if(slpdeg < 2)                   rast2[i][j] = 1;    /*!< Very Slightly leaned/sloped - Muy ligeramente inclinado */
					if(slpdeg >= 2  && slpdeg < 5)   rast2[i][j] = 2;    /*!< Slightly leaned/sloped - ligeramente inclinado */
					if(slpdeg >= 5  && slpdeg < 10)  rast2[i][j] = 3;    /*!< leaned/sloped - inclinado */
					if(slpdeg >= 10 && slpdeg < 15)  rast2[i][j] = 4;    /*!< heavily leaned/sloped - Fuertemente inclinado */
					if(slpdeg >= 15 && slpdeg < 30)  rast2[i][j] = 5;    /*!< Moderately steep - Moderadamente escarpado */
					if(slpdeg >= 30 && slpdeg < 60)  rast2[i][j] = 6;    /*!< Steep - Escarpado */
					if(slpdeg >= 60)                 rast2[i][j] = 7;    /*!< Very steep Muy escarpado */	
					/**
					* Classifying s-gradient histogram frequency
					*/
					if(slpdeg == 0)                  degslo[0]++;    
					if(slpdeg  > 0  && slpdeg < 5)   degslo[1]++;    
					if(slpdeg >= 5  && slpdeg < 10)  degslo[2]++;     
					if(slpdeg >= 10 && slpdeg < 15)  degslo[3]++;     
					if(slpdeg >= 15 && slpdeg < 20)  degslo[4]++;     
					if(slpdeg >= 20 && slpdeg < 25)  degslo[5]++;    
					if(slpdeg >= 25 && slpdeg < 30)  degslo[6]++;     
					if(slpdeg >= 30 && slpdeg < 35)  degslo[7]++;     
					if(slpdeg >= 35 && slpdeg < 40)  degslo[8]++;     
					if(slpdeg >= 40 && slpdeg < 45)  degslo[9]++;     
					if(slpdeg >= 45 && slpdeg < 50)  degslo[10]++;   
					if(slpdeg >= 50 && slpdeg < 55)  degslo[11]++;      
					if(slpdeg >= 55 && slpdeg < 60)  degslo[12]++;      
					if(slpdeg >= 60 && slpdeg < 65)  degslo[13]++;      
					if(slpdeg >= 65 && slpdeg < 70)  degslo[14]++;      
					if(slpdeg >= 70 && slpdeg < 75)  degslo[15]++;      
					if(slpdeg >= 75 && slpdeg < 80)  degslo[16]++;         
					if(slpdeg >= 80 && slpdeg < 85)  degslo[17]++; 
					if(slpdeg >= 85 )                degslo[18]++;	
				}	
				if(h0 == nullval)                                       /*!< if z = null value */
				{
					rast1[i][j] = nullval;                              /*!< save null value */
					rast2[i][j] = nullval;
					rast3[i][j] = nullval;
					degslo[18]++;
				}
			}
		}
	}
	/**
	* Calling raster writting functions
	*/
	maxval = 90;                                                        /*!< Assign z max value for raster head data */
	chkslop1 = write_rast1(3, 0);                                       /*!< Write raster in Degrees */	
	if(chkslop1 == 1)chksum++;                                          /*!< if error creating output rast1 file */
	maxval = 7;                                                         /*!< Assign z max value for raster head data */	
	chkslop2 = write_rast2(3, 0);                                       /*!< Write raster in Class */
	if(chkslop2 == 1)chksum++;                                          /*!< if error creating output rast2 file */	
	maxval = 100;                                                       /*!< Assign z max value for raster head data */
	chkslop3 = write_rast3(3, 0);                                       /*!< Write raster in Percentages */
	if(chkslop3 == 1)chksum++;                                          /*!< if error creating output rast1 file */	
	/**
	* Write histograms
	*/
	printf("\nWrite histograms - slope-gradient %i\n", metslop);
	i = 0;
	sprintf(nom_hist, "%sS-gradie_Histo_%i.txt", dirfinout, metslop);
    if((file = fopen(nom_hist,"w"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
    }
    else
    {	 
	    fprintf(file,"%s\n",
	         "id freq perc surf"); //primera linea
        for(i=0;i<19;i++)
        {
			/**
			* Calc normalized values
			*/
			if(degslo[i] == 0)degpor[i]=0;
			if(degslo[i]  > 0)degpor[i]= (double)degslo[i] / (double)totdemcel;
			superf = (degslo[i] * resx * resy)/1000000;					/*!< Calc total surface by s-gradient class in km2 */
			printf("Total cells %i, surface %.2lf km2 with s-grad class %i\n", degslo[i], superf, i);
			/**
			* Write final values
			*/
			fprintf(file,"%i %i %lf %lf\n",
				i,
				degslo[i],
				degpor[i],
				superf
				);
		}	
    } 
    fclose(file);
    return 0;
}

//---------------------------------------------------------------------------------------
//***************************************************************************************
//***************************END MORPHOMETRIC SECTION************************************
//***************************************************************************************
//**********************************FLOW PATHS*******************************************
//***************************************************************************************
//---------------------------------------------------------------------------------------

/*! SINGLE FLOW PATH LHM OR SSM (1 2) */
void calc_singflow(void)
{	
int i, j, m, n, o, maxbdpt, ext;
unsigned char done;
double ll, tx, ty, tz;

	iniepoch = clock();                                                 /*!< Get init time */
	if(nfile == 0) firsepoch = clock();
	done = 0;                                                           /*!< Control the loop */
	m    = 0;                                                           /*!< Count points/cells per flow path crossed */
	o    = 0;                                                           /*!< Count sinks crossed */
	hl2  = 0;
	maxbdpt = (BEDRIVER * 80) / 100;                                    /*!< Decrease the max value of points available for each flow path to avoid memory overflow */
	i= nxc;                                                             /*!< Init column index */
	j= nyc;                                                             /*!< Init row index */
	rast2[i][j] = 1;                                                    /*!< Starting point value */
	rast1[i][j] = 1;                                                    /*!< Starting point value */
	ext=0;
	//-----------------
	do
	{
		n = 0;                                                          /*!< Count how many times the z is increased when sink */
		calc_levelfill(i, j);								            /*!< Get a new cell in the flow path */
		                                                        
		if(row1<5||row1>rowy-5||col1<5||col1>colx-5||topoless[col1][row1]== 0||topoless[col1][row1]== nullval)done = 0;    /*!< if the new cell index is out of limits - stop loop */
		else
		{
			if(sum1 == -1)
			{
				ext = 1;
				break;                                        			/*!< if all neighboring cell were used before - stop */	
			}	
			if(sum1 == -2 || sum1 == -3)                                /*!< if the cell is a sink */
			{

				if(incre > 0)												/*!< if fill increase is higher than 0 */
				{
					hl2 += incre;                                       /*!< increase the z+critical height value using Filling Increase var */
					calc_levelfill(i, j);                               /*!< Get a new cell in the flow path */
					n++;
				}
				else break;		
			}	
			if(sum1 > 0)                                                /*!< if the cell is not a sink */
			{
				tx = xmin + (col1*resx);                                /*!< getting the coordinate from array index */
				ty = ymin + (row1*resy);
				tz = topoless[col1][row1];                              /*!< getting original z value */
				//------------------------
				if (tz == nullval) 
				{
					ext = 3;
					break;								/*!< Avoid z null values */
				}	
				//------------------------
				/**
				* Saving data in caucerio struc 
				*/
				caucerio[m].cjerar  = 1;
				caucerio[m].crio    = nfile;
				caucerio[m].ctramo  = 0;
				caucerio[m].cidpt   = m; 
				if(m == 0)caucerio[m].cdist = 0;
				if(m >  0)caucerio[m].cdist += caucerio[m-1].cdist + distpt;
				caucerio[m].cxcoor  = tx;
				caucerio[m].cycoor  = ty;
				caucerio[m].czcoor  = tz;
				caucerio[m].cdx     = distpt;
				caucerio[m].cdt     = 0;
				caucerio[m].quality = n;
				rast1[col1][row1]   = m;                                /*!< total cells selected */
				rast2[col1][row1]   = 1;                                /*!< Control raster - this cell was selected */
				rast3[col1][row1]   = 1;								/*!< Final results - this cell was selected */
				
				inxdir[m] = col1;                                       /*!< Save array index from selected cells */
				inydir[m] = row1;
				
				if(maximun <  rast1[i][j])maximun = rast1[i][j];        /*!< Get the maximum value */
				ll = sqrt((tx-xc)*(tx-xc)+(ty-yc)*(ty-yc)); /*!< Calc distance from cell to init cell */
				//--------
				if(ll>lmax)
				{
					ext = 4;
					done = 0;                                    /*!< If distance is higher than Max. Dist - stop loop */
				}	
				else done = 1;      
				j = row1;                                               /*!< reset the new array index values */
				i = col1;
				if(n>0)o++;                                             /*!< if sink, add count */
				m++;
				n = 0; 													/*!< Reset Fill increase count */
				hl2 = 0;                                                /*!< Reset Fill increase value */
			}
		}	
		if(m == maxbdpt)
		{
			ext = 5;
			break;                                          /*!< if total points allowed per flow path is reached */
		}	
	}while(done>0);                                                     /*!< Keep the loop on while done > 0 */
	printf("nfile %i why %i hl2 %f\n", nfile, ext, hl2);
	resflow = m;
	if(m>0)                                                             /*!< If the flow path exist */
	{
		ok = 1;
		totalpt = m;	
		if(ncentros <= OUTPUTPATH)	                                    /*!< If total z points are less than ... */
		{
			/**
			* Writing output txt and kml files
			*/
			sprintf(nom_trayec, "%suflowpath%i_%i.csv", dirfinout, flowtyp, nfile);
			sprintf(nom_outviakl, "%suflowpath%i_%i.kml", dirfinout, flowtyp, nfile);
			chksing1 = write_pathcsv();
			chksing2 = write_outkmll();
			if (chksing1 == 1)
			{
				errcsv[nfile] = 1;
				chksum++;
			}	
			if (chksing2 == 1)
			{
				errkml[nfile] = 1;
				chksum++;
			} 
			resarray(5);                                                /*!< Reset struct cauce rio */
		}
		mxdisflw = ll;
		tocelflw = totalpt;
		areaflow = totalpt * resx * resy;
		totskflw = o;
		printf("Maximum distance reached %.2lf\n", ll);
		printf("Total points/cells per flow path calculated %i\n", totalpt);
		printf("Total sinks per flow path found %i\n", o);
	}
	
	endepoch = clock();
	difepoch =  endepoch/1000000 -  iniepoch/1000000;        /*!< Calc computing time */
	printf("Computing time %ld sec\n", difepoch);

	if(forcevar == 0 && mod == 0)
	{
		resarray(0);                           /*!< Reset Control rast2 */
	}	
	if(forcevar == 0 && mod == 1)
	{
		call_writeraster();                    /*!< Write raster 1 and 3*/
		resarray(1);                           /*!< Reset all raster */
	}	
	//if(forcevar == 1 && mod == 0)              /*!< Nothing to do */	
	if(forcevar == 1 && mod == 1)
	{
		call_writeraster();                    /*!< Write raster 1 and 3 */
		resarray(2);                           /*!< Reset raster 1 and 3 */
	}
	write_flowres();
}

/*! DRUNK SAILER MONTECARLO TYPE FLOW PATH (3) */
void calc_montflow(void)
{
int i, j, k, l, m, q, o;
int cont1, c, r, cr, cn;
int limext, pasreach, mxdisreach, badcell, nowayout; 
int n_pasos, limpass, oks, rep;
double s[8];
double pr, sum;
unsigned char done;
double ll, inix, iniy;
	/**
	* Init var
	*/
	iniepoch = clock();                                                 /*!< Get init time */
	if(nfile == 0) firsepoch = clock();
    for(i=0;i<8;i++)s[i]= 0; 
    cn         = 0;                          
    o          = 0;                                                     /*!< Count total number of sinks in flow path */
    m          = 0;                                                     /*!< Count total number of cells in flow path */
    oks        = 1;
    rep        = 0;                                                     /*!< Control noway-out */
    cont1      = 0;                                                     /*!< Count total number of iterations */
    nowayout   = 0;                                                     /*!< Count cells selected high numbers of times consecutively */
    badcell    = 0;														/*!< Count cells with null value or 0 in altitude */
    pasreach   = 0;														/*!< Count number of times where the total number of steps by flow simulation is reached */
    mxdisreach = 0;														/*!< Count number of times where the maximum distance is reached */
    limext     = 0;														/*!< Count number of times where the raster's limits are reached */
    limpass    = (lmax / resx) * nitera;                                /*!< Total cells permitted per flow path calculation */
    srand(time(NULL));
    /**
	* First Loop - Control iterations and init zpoint
	*/
    do
    {
        n_pasos = 0;                                                    /*!< Count cells selected per path */
        rep     = 0;
        done    = 0;                                                    /*!< Control the loop - reset to 0 */
        /**
		* Change original xy using a random function
		*/
        k = (int)(500.0*(float)rand()/(float)RAND_MAX); 
	    i= nxc + gauss[k]; 
        k = (int)(500.0*(float)rand()/(float)RAND_MAX);
	    j= nyc + gauss[k];
	    /**
		* Get coordinates of original xy
		*/ 
	    get_coor(i, j);
	    inix = globx;
	    iniy = globy;
	    /**
		* Second Loop - Get the flow path in each iteration
		*/
        do
        {
            getmovingcell(i, j, 1);                                     /*!< get 3x3 moving cell z values */
			sum = 0;                                                    /*!< reset sum var */
            for(l=0;l<8;l++)
            {
                if(h[l]>0)sum += h[l];                                  /*!< Sum all z-diff higher than 0 */
                s[l]=sum;                               /*!< store sum var in s[] array for each element */                                             
            }
            /**
			* Check loop conditions - sink - z value - sum
			*/
			if(topoless[i][j] == 0 || topoless[i][j] == nullval)
            {
				badcell++;
				done = 1;  /*!< if z == 0 if sink s[7]=0 or s[7]=hl and sum=0 then done=1 out loop 2 */
			}
            if(sum==0)                                                 /*!< if s[7] is 0 is a sink */
            {
				o++;
				if(incre > 0)											/*!< if fill increase is higher than 0 */
				{
					if(n_pasos > 1)
					{
						precol = inxdir[n_pasos-2];                     /*!< Save array index from selected cells */
						prerow = inydir[n_pasos-2];                     /*!< increase the z+critical height value using Filling Increase function */
						/**
						* flow direction according last cell
						*/
						c = 0;
						r = 0;
						if(precol == i-1) c = 1;
						if(precol == i+1) c = 2;
						if(precol == i)   c = 3;
						if(prerow == j-1) r = 10;
						if(prerow == j+1) r = 20;
						if(prerow == j)   r = 30;
						cr = c + r;
						if(cr > 0 && cr != 33)
						{
							calc_levelfill2(i, j, cr);
							if(sum1 > 0)
							{
								for(l=0;l<8;l++)
								{
									if(h[l]>0)sum += h[l];                      /*!< Sum all z-diff higher than 0 */
									s[l]=sum;                                   /*!< store sum var in s[] array for each element */
								}
							}
							if(sum1 == -1)
							{
								done = 2;
								break;
							}
						}
						else done = 2;				
					}
					else done = 2;
				}	
				else done = 2;
			}		
			if(done==0)
            {
    		    /**
				* select the new cell
				*/
    		    pr = s[7]*((double)(rand()%1000)/1001.0);               /*!< val from 0 to sum max */
                q = 7;                                                  /*!< starting value for q var */
                if(pr<s[6])q=6;                                         /*!< if pr random var is less than s[x] */
                if(pr<s[5])q=5;
                if(pr<s[4])q=4;
                if(pr<s[3])q=3;
                if(pr<s[2])q=2;
                if(pr<s[1])q=1;
                if(pr<s[0])q=0;
                /**
				* Set new cell array index
				*/
                switch(q) 
                {
                case 0:
                    i--;
                    break;
                case 1:
                    i++;
                    break;
                case 2:
                    j++;
                    break;
                case 3:
                    j--;
                    break;
                case 4:
                    i++;
                    j++;
                    break;
                case 5:
                    i--;
                    j++;
                    break;
                case 6:
                    i--;
                    j--;
                    break;
                case 7:
                    i++;
                    j--;
                    break;
                }
                /**
				* Check loop conditions to see if new cell is inside array
				* borders are not considered
				*/
                if((i<2 || i>colx-3) || (j<2 || j>rowy-3))
                {
					limext++;
					done = 3;     /*!< if outside then done= 3 out loop 2 */
				}	
                else
                {
					/*!< Count new cells in m, */
					if(forcevar == 0 && incre  > 0) 
					{
						if(rast2[i][j]==0)m++;
					}	
					else
					{
						if (cont1 == 0)
						{
							if(rast2[i][j]==0)m++;
						}	
						if (cont1 > 0)
						{
							if(rast1[i][j]==0)m++;
						}
					}										
					rast1[i][j] ++;										/*!< sum 1 to the cell */
					rast2[i][j] = 1;                                    /*!< Control raster */
					rast3[i][j] ++;                                     /*!< sum 1 to the cell */
					if(maximun < rast1[i][j])maximun = rast1[i][j];     /*!< Get the maximum value */
					inxdir[n_pasos] = i;                                /*!< Save array index from selected cells */
					inydir[n_pasos] = j;
					coin[cn] = i;
					roin[cn] = j;
					if(cn == 8) cn = 0;                                 /*!< Save array index of last eight cells -control filling depression */
					if(cn  < 8) cn++;
					/**
					* Check other loop conditions 
					*/
					get_coor(i, j);
					ll = sqrt((globx-inix)*(globx-inix)+(globy-iniy)*(globy-iniy));         /*!< Calc distance from init */
					if(ll>lmax)                                         /*!< Check if distance is higher than permitted */
					{
						mxdisreach++;
						done = 5;                                       /*!< if yes then done=5 - exit second loop */
					}	
					if(n_pasos > limpass)	                            /*!< Check if number of cells is higher than permitted */
					{
						pasreach++;
						done = 6;                                       /*!< if yes then done=6 - exit second loop */
					}
					if(n_pasos > 2) 
					{
						if(inxdir[n_pasos-2] == i && inydir[n_pasos-2] == j) rep++;   /*!< Check if cell has been selected recently */
						
						if(rep > REPIT)                                  /*!< if cell has been selected recently more than 1000 times */
						{ 
							nowayout++;
							done = 4;                                   /*!< if yes then done=4 - exit second loop */
						}	
					}	
					n_pasos++;	                                        /*!< Count selected cells */
				}	
            }
        }
	    while(!done);                                                   /*!< While done = 0 keep loop 2 */
	    /**
		* check if remain in the first Loop
		*/
        if(done == 3 || done == 4 || done == 5) done=0;
        if(done == 6)
        {
			limpass = limpass / 2;                                      /*!< Reduce the number of allowed steps */
			done=0;	
		}	
        if(done == 1 || done == 2)                                      /*!< if done=1-4 */ 
        {
            if(ok < 100)done=0;
            oks++;   
        }	
        cont1++;                                                        /*!< Count iterations */
    }while(!done && cont1 < nitera);                                    /*!< While done = 0 and cont1 lower than total iterations then keep loop 1 */

	
    iterafin = cont1;
    tocelflw = m;
    totskflw = o;
    glpasreach = pasreach; 
    gllimext = limext; 
    glmxdisreach = mxdisreach; 
    glbadcell = badcell; 
    glnowayout = nowayout;
	/**
	* Write and Reset raster
	*/                                         
	resarray(4);                               /*!< Reset noway-out arrays */
	if(forcevar == 0 && mod == 0)
	{
		resarray(3);                           /*!< Reset Control rast2 */ 
	}	
	if(forcevar == 0 && mod == 1)
	{
		call_writeraster();                    /*!< Write raster 1 and 3*/
		resarray(1);                           /*!< Reset all raster */
	}	
	//if(forcevar == 1 && mod == 0)              /*!< Nothing to do */	
	if(forcevar == 1 && mod == 1)
	{
		call_writeraster();                    /*!< Write raster 1 and 3 */
		resarray(2);                           /*!< Reset raster 1 and 3 */
	}
	write_flowres();
	endepoch = clock();
	difepoch =  endepoch/1000000 -  iniepoch/1000000;                   /*!< Calc computing time */
	printf("Why exit: intera %i pasosalc %i outrast %i maxdist %i badcell %i noway %i\n", iterafin, pasreach, limext, mxdisreach, badcell, nowayout);
    printf("Total cells selected per flow path %i\n", tocelflw);
	printf("Total sinks found %i\n", totskflw);
	printf("Computing time %ld sec\n", difepoch);
    //-----
}

/*! MULTITRAYECTORY FLOW PATH (4) */
void calc_mulflow(void)
{
int i, j, k, l, cont, cont2, getval, sk;	
float sumh,value; 
double ll;	
	
	iniepoch = clock();                                                 /*!< Get init time */
	if(nfile == 0) firsepoch = clock();
	cont    = 0;                                                        /*!< Count total number of selected cell per flow path */
	cont2   = 0;
	puntero = 0;
	sk      = 0;														/*!< Count sinks */ 
	fifoini();                                                          /*!< Init FIFO */
	/**
	* Get init array index and z value
	*/
	fcolm = nxc;
	frowm = nyc;
	i= nxc;    
	j= nyc;     
	/**
	* Set init z point to 1
	*/                                                                  
	rast1[i][j] = 1;                                                    /*!< Store value */   
	rast3[i][j] = 1;                                                    /*!< Store value */
	fifocarga();                                                        /*!< Sent first data to the FIFO */
	cont++;
	/**
	* Start Loop
	*/
	do
	{
		cont2++;
		i = FIFO[0][0];                                                 /*!< Get from FIFO col value */
		j = FIFO[0][1];                                                 /*!< Get from FIFO row value */
		if(i == -1 && j == -1)break;                                    /*!< if FIFO is empty */
		getval = rast3[i][j];                                           /*!< Get rast2 value */
		if (topoless[i][j] != nullval)									/*!< If z val is not null */
		{
			getmovingcell(i, j, 0);                                         /*!< Get 3x3 moving cell z values */
			/**
			* Sum z-diff higher than 0 
			*/
			sumh = 0.0;                                                 /*!< Sum z-diff values higher than 0 */
			l    = 0;                                                   /*!< Count cells with z-diff values higher than 0 */
			for(k=0;k<8;k++)
			{
				if(h[k] > 0) 
				{
					sumh += h[k]; 
					l++;
				} 	
			}
			if (l > 0)                                                  /*!< if there are z-diff values higher than 0 */
			{
				for(k=0;k<8;k++)
				{
					if (h[k] > 0)                                       /*!< if z-diff is higher than 0 */
					{
						if (k == 0)                                     /*!< if z-diff in cero is higher than 0 */
						{
							fcolm = i--;                                /*!< Change column index */
							frowm = j;                                  /*!< Change column index */
						}
						if (k == 1)
						{
							fcolm = i++;                                
							frowm = j;                                  
						}
						if (k == 2)
						{
							fcolm = i;                                  
							frowm = j++;                                
						}
						if (k == 3)
						{
							fcolm = i;                                  
							frowm = j--;                                
						}
						if (k == 4)
						{
							fcolm = i++;                                
							frowm = j++;                                
						}
						if (k == 5)
						{
							fcolm = i--;                                
							frowm = j++;                                
						}
						if (k == 6)
						{
							fcolm = i--;                                
							frowm = j--;                                
						}
						if (k == 7)
						{
							fcolm = i++;                                
							frowm = j--;                                
						}
						/**
						* Check if cell is inside array
						*/
						if((i < 2 || i >= colx-2) || (j < 2 || j >= rowy-2))break; 		/*!< if not then stop loop*/
						else
						{
							value = (h[k] * 100) / sumh;	                /*!< Calc percentage of total sum */
							if( rast1[fcolm][frowm] == 0 )                  /*!< if it is a new cell in the flow path */    
							{
								if(value > distran)                         /*!< if percentage is higher than Restric. Multiflow var then ... */       
								{  
									rast1[fcolm][frowm] = 1;                /*!< Set new value in rast1 */ 
									rast3[fcolm][frowm] = getval+1;         /*!< Set new value in rast3 */ 
									if(maximun < rast3[fcolm][frowm])maximun = rast3[fcolm][frowm];        /*!< Get the maximum value */  
									fifocarga();                            /*!< Load array index in FIFO */ 
									cont++;	                                /*!< Count selected cell */                                                                
								}	
							}
							get_coor(fcolm, frowm);						/*!< Get coordinates */  
							ll = sqrt((globx-xc)*(globx-xc)+(globy-yc)*(globy-yc));             /*!< Calc distance from init */ 
							if(ll>lmax)break;                               /*!< Check if the distance is higher than Max. Dist. */	
						}	
					}  
				}	
				fifodesplaza();	                                        /*!< subtract cell from FIFO */
			}
			if (l == 0)
			{
				sk++;
				fifodesplaza();                                         /*!< if not, subtract cell from FIFO */                    		
			}	
		}
		if (topoless[i][j] == nullval)  fifodesplaza();	   	            /*!< subtract cell from FIFO */
	}while(puntero > 1 || i != -1 || j != -1);		                    /*!< While FIFO has cells */
	
	tocelflw = cont;
    totskflw = sk;
	/**
	* Write and Reset raster
	*/
	//if(forcevar == 0 && mod == 0)            /*!< Nothing to do */
	if(forcevar == 0 && mod == 1)              	
	{
		call_writeraster();                    /*!< Write raster 1 and 3*/
		resarray(2);                           /*!< Reset all raster */
	}	
	//if(forcevar == 1 && mod == 0)            /*!< Nothing to do */	
	//if(forcevar == 1 && mod == 1)            /*!< Nothing to do */

	write_flowres();													/*!< Write resume flow path file */
	endepoch = clock();
	difepoch =  endepoch/1000000 -  iniepoch/1000000;                   /*!< Calc computing time */
	printf("Computing time %ld sec\n", difepoch);
	printf("Maximun distance reached %.2lf m\n", ll);
	printf("Total points/cells selected %i\n\n", tocelflw);
	printf("Total sinks found %i\n", totskflw);


}

//---------------------------------------------------------------------------------------
//***************************************************************************************
//********************************END FLOW PATHS*****************************************
//***************************************************************************************
//***************************WRITE OUTPUT RASTER FILES***********************************
//***************************************************************************************
//---------------------------------------------------------------------------------------


/*! WRITE FROM SURFACE DEPRESSION AND MODIFIED DEM */
int write_newdem(int typ)
{
char   buffer[255];
short int buff_int[4];
double buff_double[32];
float  *buff_float;
int i, j;
FILE *out;
    
    /**
	* Assign filenames
	*/
    if(typ == 0)sprintf(nom_newdem, "%sMod_newmdtxyz.grd", dir_out);    /*!< if DEM modification is on - Assign output filename */
    if(typ == 1)sprintf(nom_newdem, "%sSink_newmdt.grd", dir_out);      /*!< if Surface depression is on - Assign output filename */
    printf("Write output raster %s\n", nom_newdem);  
    /**
	* Get raster head data
	*/
    sprintf(buffer,"DSBB");
    buff_int[0]    = colx;
    buff_int[1]    = rowy;
	buff_double[0] = xmin;
	buff_double[1] = xmax;
	buff_double[2] = ymin;
	buff_double[3] = ymax;
    buff_double[4] = 0;
    buff_double[5] = maxval;
    /**
	* Create dynamic buffer to store one row
	*/
    buff_float = (float *)malloc(sizeof(float)*colx);
    /**
	* Open file 
	*/
    if((out=fopen(nom_newdem,"wb"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Write raster head data
	*/
    fwrite(buffer, 4, 1, out);
    fwrite(buff_int,    sizeof(short int)*2, 1, out);
    fwrite(buff_double, sizeof(double)*6, 1, out);
    /**
	* Read and write row by row 
	*/
	for (j=0;j<rowy;j++)
	{
        for(i=0;i<colx;i++)buff_float[i] = (float)topoless[i][j];       /*!< Store row in buffer */
	    fwrite(buff_float, sizeof(float)*colx, 1, out);                 /*!< Write buffer in output file */
	}
    fclose(out);
    return 0;
}

/*! WRITE FROM S-ASPECT/GRAD DEGREE AND SINGLE SUM FLOW PATH LHM AND SSM */
int write_rast1(int typ, int mod)
{
char   buffer[255];
short int buff_int[4];
double buff_double[32];
float  *buff_float;
int i, j;
FILE *out;
	/**
	* Assign filenames
	*/ 
	if(mod < 2)
	{
		//--------------------------------------------------------------MORPHOMETRIC 
		if(typ == 2)sprintf(nom_rast1, "%sS-aspect_degree_%i.grd", dir_out, metasp);	
		if(typ == 3)sprintf(nom_rast1, "%sS-gradie_degree_%i.grd", dir_out, metslop);
	}	
	if(mod == 0)
	{	
		//------------------------------------------------------------------
		if(typ == 10)sprintf(nom_rast1, "%sFlowpath_LHM_sum_%.1f_%.1f_%i.grd", dir_out, hl, incre, forcevar);
		if(typ == 11)sprintf(nom_rast1, "%sFlowpath_SSM_sum_%.1f_%.1f_%i.grd", dir_out, hl, incre, forcevar);
		if(typ == 12)sprintf(nom_rast1, "%sFlowpath_MTramdom_%.1f_%.1f_%i.grd", dir_out, hl, incre, forcevar);
		if(typ == 13)sprintf(nom_rast1, "%sFlowpath_Multiflow_%.1f_%.1f.grd", dir_out, hl, distran);
		//------------------------------------------------------------------
	}
	if(mod == 1)
	{
		//------------------------------------------------------------------
		if(typ == 10)sprintf(nom_rast1, "%sFlowpath_LHM_sum_%.1f_%.1f_%i_%i.grd", dir_out, hl, incre, forcevar, nfile);
		if(typ == 11)sprintf(nom_rast1, "%sFlowpath_SSM_sum_%.1f_%.1f_%i_%i.grd", dir_out, hl, incre, forcevar, nfile);
		if(typ == 12)sprintf(nom_rast1, "%sFlowpath_MTramdom_%.1f_%.1f_%i_%i.grd", dir_out, hl, incre, forcevar, nfile);
		if(typ == 13)sprintf(nom_rast1, "%sFlowpath_Multiflow_%.1f_%.1f_%i.grd", dir_out, hl, distran, nfile);

		//------------------------------------------------------------------
	}		
	printf("Write output raster1 %s\n", nom_rast1);  
	/**
	* Get raster head data
	*/
    sprintf(buffer,"DSBB");
    buff_int[0]    = colx;
    buff_int[1]    = rowy;
	buff_double[0] = xmin;
	buff_double[1] = xmax;
	buff_double[2] = ymin;
	buff_double[3] = ymax;
    buff_double[4] = 0;
    buff_double[5] = maxval;
    /**
	* Create dynamic buffer to store one row
	*/
    buff_float = (float *)malloc(sizeof(float)*colx);
    /**
	* Open file 
	*/
    if((out=fopen(nom_rast1,"wb"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Write raster head data
	*/
    fwrite(buffer, 4, 1, out);
    fwrite(buff_int,    sizeof(short int)*2, 1, out);
    fwrite(buff_double, sizeof(double)*6, 1, out);
    /**
	* Read and write row by row 
	*/
	for (j=0;j<rowy;j++)
	{
        for(i=0;i<colx;i++)buff_float[i] = (float)rast1[i][j];          /*!< Store row in buffer */
	    fwrite(buff_float, sizeof(float)*colx, 1, out);                 /*!< Write buffer in output file */
	}
    fclose(out);	
	return 0;
}

/*! WRITE FROM S-ASPECT/GRAD CLASS, SINGLE FLOW PATH LHM AND SSM, MONTECARLO AND MULTIFLOW */
int write_rast2(int typ, int mod)
{
char   buffer[255];
short int buff_int[4];
double buff_double[32];
float  *buff_float;
int i, j;
FILE *out;
    /**
	* Assign filenames
	*/ 
	if(mod < 2)
	{
		//-----------------------------------------------------------MORPHOMETRIC  
		if(typ == 2)sprintf(nom_rast2, "%sS-aspect_clasific_%i.grd", dir_out, metasp);
		if(typ == 3)sprintf(nom_rast2, "%sS-gradie_clasific_%i.grd", dir_out, metslop);
	}
    printf("Write rast2 %s\n", nom_rast2); 
    /**
	* Get raster head data
	*/
    sprintf(buffer,"DSBB");
    buff_int[0]    = colx;
    buff_int[1]    = rowy;
	buff_double[0] = xmin;
	buff_double[1] = xmax;
	buff_double[2] = ymin;
	buff_double[3] = ymax;
    buff_double[4] = 0;
    buff_double[5] = maxval;
    /**
	* Create dynamic buffer to store one row
	*/
    buff_float = (float *)malloc(sizeof(float)*colx);
    /**
	* Open file 
	*/
    if((out=fopen(nom_rast2,"wb"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Write raster head data
	*/
    fwrite(buffer, 4, 1, out);
    fwrite(buff_int,    sizeof(short int)*2, 1, out);
    fwrite(buff_double, sizeof(double)*6, 1, out);
    /**
	* Read and write row by row 
	*/
	for (j=0;j<rowy;j++)
	{
        for(i=0;i<colx;i++)buff_float[i] = (float)rast2[i][j];          /*!< Store row in buffer */
	    fwrite(buff_float, sizeof(float)*colx, 1, out);                 /*!< Write buffer in output file */
	}
    fclose(out);
	return 0; 
}

/*! WRITE FROM S-GRAD PERCENTAGE */
int write_rast3(int typ, int mod)
{
char   buffer[255];
short int buff_int[4];
double buff_double[32];
float  *buff_float;
int i, j;
FILE *out;
     
    /**
	* Assign filenames
	*/
	if(mod < 2)
	{
		//-----------------------------------------------------------MORPHOMETRIC  
		if(typ == 3)sprintf(nom_rast3, "%sS-gradie_porcen_%i.grd", dir_out, metslop);
	}
    if(mod == 0)	
	{
		//-----------------------------------------------------------FLOW PATH
		if(typ == 10)sprintf(nom_rast3, "%sFlowpath_LHM_%.1f_%.1f_%i.grd", dir_out, hl, incre, forcevar);	
		if(typ == 11)sprintf(nom_rast3, "%sFlowpath_SSM_%.1f_%.1f_%i.grd", dir_out, hl, incre, forcevar);
		if(typ == 12)sprintf(nom_rast3, "%sFlowpath_MTramdomLog_%.1f_%.1f_%i.grd", dir_out, hl, incre, forcevar);
		if(typ == 13)sprintf(nom_rast3, "%sFlowpath_Multiflow_sum_%.1f_%.1f.grd", dir_out, hl, distran);
	}
	if(mod == 1)
	{
		//-----------------------------------------------------------FLOW PATH
		if(typ == 10)sprintf(nom_rast3, "%sFlowpath_LHM_%.1f_%.1f_%i_%i.grd", dir_out, hl, incre, forcevar, nfile);	
		if(typ == 11)sprintf(nom_rast3, "%sFlowpath_SSM_%.1f_%.1f_%i_%i.grd", dir_out, hl, incre, forcevar, nfile);
		if(typ == 12)sprintf(nom_rast3, "%sFlowpath_MTramdomLog_%.1f_%.1f_%i_%i.grd", dir_out, hl, incre, forcevar, nfile);
		if(typ == 13)sprintf(nom_rast3, "%sFlowpath_Multiflow_sum_%.1f_%.1f_%i.grd", dir_out, hl, distran, nfile);
	}
	printf("Write rast3 %s\n", nom_rast3);		
    /**
	* Get raster head data
	*/
    sprintf(buffer,"DSBB");
    buff_int[0]    = colx;
    buff_int[1]    = rowy;
    buff_double[0] = xmin;
    buff_double[1] = xmax;
    buff_double[2] = ymin;
    buff_double[3] = ymax;
    buff_double[4] = 0;
    buff_double[5] = maxval;
    /**
	* Create dynamic buffer to store one row
	*/
    buff_float = (float *)malloc(sizeof(float)*colx);
    /**
	* Open file 
	*/
    if((out=fopen(nom_rast3,"wb"))==NULL)
	{
	    printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
	}
	/**
	* Write raster head data
	*/
    fwrite(buffer, 4, 1, out);
    fwrite(buff_int,    sizeof(short int)*2, 1, out);
    fwrite(buff_double, sizeof(double)*6, 1, out);
    /**
	* Read and write row by row 
	*/
	for (j=0;j<rowy;j++)
	{
        for(i=0;i<colx;i++)buff_float[i] = (float)rast3[i][j];          /*!< Store row in buffer */
	    fwrite(buff_float, sizeof(float)*colx, 1, out);                 /*!< Write buffer in output file */
	}
    fclose(out);
	return 0;
}


//---------------------------------------------------------------------------------------
//***************************************************************************************
//***************************END OUTPUT RASTER FILES*************************************
//***************************************************************************************
//***************************WRITE OUTPUT CSV/KML FILES**********************************
//***************************************************************************************
//---------------------------------------------------------------------------------------

/*! WRITE TXT FILE FROM SINGLE FLOW PATH LHM AND SSM  */
int write_pathcsv(void)
{
FILE *file;
int i;

	printf("\nWrite flow path in xyz file %i\n", nfile);
	i = 0;
    if((file = fopen(nom_trayec,"w"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
    }
    else
    {
			 
	    fprintf(file,"%s\n",
	         "njear nrios ntramo npt ndist xcoor ycoor zcoor dx	dt quality"); //primera linea
        for(i=0;i<totalpt;i++)
        {		
			fprintf(file,"%i %i %i %i %.2f %lf %lf %lf %.2f %.2f %i\n",
				
				caucerio[i].cjerar,
				caucerio[i].crio,
				caucerio[i].ctramo,
				caucerio[i].cidpt,
				caucerio[i].cdist,
				caucerio[i].cxcoor,
				caucerio[i].cycoor,
				caucerio[i].czcoor,
				caucerio[i].cdx,
				caucerio[i].cdt,
				caucerio[i].quality			
				);	
		}
	}
	fclose(file);
	
	
	printf("Output file name = %s\n",nom_trayec);
	printf("End writing output file\n");
	printf("---------------------------------\n");
	return 0;
}	

/*! WRITE KML FILE FROM SINGLE FLOW PATH LHM AND SSM  */
int write_outkmll(void)
{
FILE *file;
char text[60];
char *buffer;
int i;
double tx, ty;
//float phywith, toutwith;
	
	printf("Write kml file, %s\n", nom_outviakl);
	/**
	* Create dynamic buffer to store coordinates
	*/
	buffer = (char *)malloc(sizeof(char)*((totalpt*50)+1));
	strcpy(buffer, "");
	 /**
	* Open file 
	*/
    if((file = fopen(nom_outviakl,"w"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
	    return 1;
    }
    else
    {
        /**
		* Print head kml file
		*/
        fprintf(file,"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
		fprintf(file,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(file,"<Document>\n");
		//esquema
		fprintf(file,"	<name>flujo_l</name>\n");
		fprintf(file,"	<Schema name=\"flujo.sch\"  id=\"flujoid.sch\">\n");
		fprintf(file,"		<SimpleField type=\"string\" name=\"idcount\"></SimpleField>\n");
		fprintf(file,"	</Schema>\n");
        //estilo
		fprintf(file,"	<Style id=\"id_style\">\n");
		fprintf(file,"		<LabelStyle>\n");
		fprintf(file,"			<color>ff9370db</color>\n");
		fprintf(file,"			<scale>0.5</scale>\n");
		fprintf(file,"		</LabelStyle>\n");
		fprintf(file,"		<LineStyle>\n");
		fprintf(file,"			<color>ff9370db</color>\n");
		fprintf(file,"			<width>2</width>\n");
		fprintf(file,"		</LineStyle>\n");
		fprintf(file,"		<BalloonStyle>\n");
		fprintf(file,"			<bgColor>ffffffbb</bgColor>\n");
		fprintf(file,"			<text>\n");
		fprintf(file,"				<![CDATA[\n");
		fprintf(file,"					<h2><b>Flujo gravitacional</b></h2>\n");
		fprintf(file,"					Nombre: $[name]\n");
		fprintf(file,"					<br/><br/>\n");
		fprintf(file,"					Nombre tipología: Flujo en %i\n", flowtyp);
		fprintf(file,"					<br/><br/>\n");
		fprintf(file,"					Tipo: $[description]\n");
		fprintf(file,"					id: $[flujo.sch/idcount]\n");
		fprintf(file,"				]]>\n");
		fprintf(file,"			</text>\n");
		fprintf(file,"		</BalloonStyle>\n");
		fprintf(file,"	</Style>\n");
		/**
		* Store coordinates in buffer
		*/
		for(i=0;i<totalpt;i++)
        {
			tx = caucerio[i].cxcoor;
			ty = caucerio[i].cycoor;
			convert_coor(tx, ty);
			caucerio[i].clong = longitud;
			caucerio[i].clat  = latitud;
			if(i< totalpt-1)sprintf(text, "%.8lf,%.8lf ", longitud, latitud);
			if(i==totalpt-1)sprintf(text, "%.8lf,%.8lf", longitud, latitud);
			strcat(buffer, text);							
		}
		/**
		* Print end kml file
		*/
		fprintf(file,"<Placemark>\n");
		fprintf(file,"	<name>%i</name>\n", nfile);
		fprintf(file,"	<description>Flujo %i</description>\n", nfile);
		//fprintf(file,"	<Style><LineStyle><color>ff0000ff</color></LineStyle><PolyStyle><fill>0</fill></PolyStyle></Style>\n");
		fprintf(file," <styleUrl>#id_style</styleUrl>\n");
		fprintf(file,"	<ExtendedData>\n");
		fprintf(file,"		<SchemaData schemaUrl=\"#flujoid.sch\">\n");
		fprintf(file,"			<SimpleData name=\"idcount\">%i</SimpleData>\n", nfile);				
		fprintf(file,"		</SchemaData>\n");
		fprintf(file,"	</ExtendedData>\n");
		fprintf(file,"	<LineString>\n");
		fprintf(file,"		<coordinates>\n");
		fprintf(file,"		%s\n", buffer);
		fprintf(file,"		</coordinates>\n");
		fprintf(file,"	</LineString>\n");
		fprintf(file,"</Placemark>\n");
		fprintf(file,"</Document></kml>\n");
		/**
		* Reset to 0 buff
		*/ 
		strcpy(buffer, "");
	}
	fclose(file); 
	return 0;	
}					


/*! WRITE FLOW PATH RESUME FILE  */
int write_flowres(void)
{
FILE *file;
char nom_res[256], mode[10], text[256];
int j, i, difs;
float difm;
	/**
	* Write resume
	*/
	if(flowtyp == 1)sprintf(nom_res, "%sFlowpath_LHH_Resum_%.1f_%.1f_%i.txt", dir_out, hl, incre, forcevar);
	if(flowtyp == 2)sprintf(nom_res, "%sFlowpath_SSM_Resum_%.1f_%.1f_%i.txt", dir_out, hl, incre, forcevar);
	if(flowtyp == 3)sprintf(nom_res, "%sFlowpath_MTramdom_Resum_%.1f_%.1f_%i.txt", dir_out, hl, incre, forcevar);
	if(flowtyp == 4)sprintf(nom_res, "%sFlowpath_Multiflow_Resum_%.1f_%.1f_%i.txt", dir_out, hl, distran, forcevar);
	
	if(nfile == 0) sprintf(mode, "w");
	if(nfile  > 0) sprintf(mode, "a+");
	
	printf("Resume %i\n", nfile);
	
	if((file = fopen(nom_res,mode))== NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		return 1;
	}
	else
	{	 
		if(nfile == 0)
		{
			sprintf(text, "Flowpaht Type %i: Parameters CH %.1f R.M. %f FI %.1f FoInt %i WRM %i\n", flowtyp, hl, distran, incre, forcevar, mod);
			fprintf(file,"%s\n\n", text); //primera linea
		}
		
		if(flowtyp < 3)
		{
			if(resflow > 0)
			{
				areaflow = tocelflw * resx * resy;
				fprintf(file,"Flow Path number %i\n", nfile);
				fprintf(file,"Time computing %ld sec\n", difepoch);
				fprintf(file,"Maximun distance reached %lf m\n", mxdisflw);
				fprintf(file,"Total cells per flow path %i\n", tocelflw);
				fprintf(file,"Total area per flow path %lf m2 and %lf km2\n", areaflow, areaflow/100000);
				fprintf(file,"Total sinks per flow path %i\n", totskflw);
				fprintf(file,"-----------------------------------------\n\n");
			}
			if(resflow == 0)
			{
				fprintf(file,"Flow Path number %i\n", nfile);
				fprintf(file,"Time computing %ld sec\n", difepoch);	
				fprintf(file,"Flow Path with no wayout\n");
				fprintf(file,"-----------------------------------------\n\n");
			}
			
			if(mod == 0 && nfile == ncentros - 1)
			{
				for(j=0;j<rowy;j++)
				{
					for(i=0;i<colx;i++)
					{
						if(rast3[i][j] > 0) acumldis++; 
					}	
				}
				acumarea = acumldis * resx * resy; 
				difs = endepoch/1000000 - firsepoch/1000000;
				difm = (float)difs / 60.0;
				fprintf(file,"Total Time computing %i - %.2f s\n", difs, difm);
				fprintf(file,"Total cells affected %lf\n", acumldis);
				fprintf(file,"Total area %lf m2 and %lf km2\n", acumarea, acumarea/100000);
				fprintf(file,"END REPORT\n");
			}
		}
		if(flowtyp == 3)
		{
			areaflow = tocelflw * resx * resy;
			fprintf(file,"Flow Path number %i\n", nfile);
			fprintf(file,"Time computing %ld s\n", difepoch);
			fprintf(file,"Why exit: pasosalc %i outrast %i maxdist %i badcell %i noway %i\n", glpasreach, gllimext, glmxdisreach, glbadcell, glnowayout);
			fprintf(file,"Total cells per flow path %i\n", tocelflw);
			fprintf(file,"Total area per flow path %lf m2 and %lf km2\n", areaflow, areaflow/100000);
			fprintf(file,"Total sinks per flow path %i\n", totskflw);
			fprintf(file,"-----------------------------------------\n\n");
			
			if(mod == 0 && nfile == ncentros - 1)
			{
				for(j=0;j<rowy;j++)
				{
					for(i=0;i<colx;i++)
					{
						if(rast1[i][j] > 0) acumldis++; 
					}	
				}
				acumarea = acumldis * resx * resy; 
				difs = endepoch/1000000 - firsepoch/1000000;
				difm = (float)difs / 60.0;
				fprintf(file,"Total Time computing %i - %.2f s\n", difs, difm);
				fprintf(file,"Total cells affected %lf\n", acumldis);
				fprintf(file,"Total area %lf m2 and %lf km2\n", acumarea, acumarea/100000);
				fprintf(file,"END REPORT\n");
			}
			if(mod == 1 && nfile == ncentros - 1)fprintf(file,"END REPORT\n");
		}
		if(flowtyp == 4)
		{
			areaflow = tocelflw * resx * resy;
			fprintf(file,"Flow Path number %i\n", nfile);
			fprintf(file,"Time computing %ld s\n", difepoch);
			fprintf(file,"Total cells per flow path %i\n", tocelflw);
			fprintf(file,"Total area per flow path %lf m2 and %lf km2\n", areaflow, areaflow/100000);
			fprintf(file,"Total sinks per flow path %i\n", totskflw);
			fprintf(file,"-----------------------------------------\n\n");
			
			if(mod == 0 && nfile == ncentros - 1)
			{
				for(j=0;j<rowy;j++)
				{
					for(i=0;i<colx;i++)
					{
						if(rast1[i][j] > 0) acumldis++; 
					}	
				}
				acumarea = acumldis * resx * resy; 
				difs = endepoch/1000000 - firsepoch/1000000;
				difm = (float)difs / 60.0;
				fprintf(file,"Total Time computing %i - %.2f s\n", difs, difm);
				fprintf(file,"Total cells affected %lf\n", acumldis);
				fprintf(file,"Total area %lf m2 and %lf km2\n", acumarea, acumarea/100000);
				fprintf(file,"END REPORT\n");
			}
			if(mod == 1 && nfile == ncentros - 1)fprintf(file,"END REPORT\n");
		}	
							
	}
	fclose(file);
	 
	return 0;
}


/*! WRITE TXT RESUME FILE  */
int write_resum(void)
{
FILE *file;
int i, j, k;
char text[10], text2[10];
char  *buff_csv, *buff_kml;
double superf;
	i=0;
	printf("\nWrite Resume File, %s\n", nom_resum);
	sprintf(nom_resum, "%sResumen.txt", dir_out);
    if((file = fopen(nom_resum,"wt"))== NULL)
    {
        printf("ERROR %s\n", nom_resum);
        return 0;
    }
    else
    {
		fprintf(file,"*****AVISOS*****\n\n");
		/**
		* Warning reading files 
		*/		
		if(demmin > zlo) 
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"READING DEM\n");
			fprintf(file,"ATTENTION, the z minimum value to read the DEM is higher than the real minimum Z value\n"); 
			fprintf(file,"This issue can generate null values in areas with real z data values\n");
			fprintf(file,"\n**********************************\n");
			i++;
		}
		if (demmax < zhi) 
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"READING DEM\n");
			fprintf(file,"ATTENTION, the z maximum value to read the DEM is lower than the real maximum Z value\n"); 
			fprintf(file,"This issue can generate null values in areas with real z data values\n");
			fprintf(file,"\n**********************************\n");
			i++;
		}
		/**
		* Warning modifiying the DEM file 
		*/
		if(modidem == 1 && fase == 2)
		{
			if(totmask != totmodmask)
			{
				fprintf(file,"\n**********************************\n");
				fprintf(file,"DEM Modification: Writing coordinate file in stage 2\n");
				fprintf(file,"ATTENTION, the total point changed in the DEM are less than the available\n"); 
				fprintf(file,"It could be a problem during the reading process\n");
				fprintf(file,"\n**********************************\n");
				i++;
			}	
		}
		if(modidem == 1)
		{
			fprintf(file,"***MODIFICATION DEM RESULTS***\n");
			fprintf(file,"STAGE %i\n", fase);
			fprintf(file,"Total number of point to be modified %i\n",  totmask);
			if(fase == 2)fprintf(file,"Total number of point modified %i\n",  totmask);
			fprintf(file,"Initial surface to be modified %.4lf m2\n", supmask);
			if(fase == 2)
			{
				fprintf(file,"Total surface modified %.4lf m2\n", supmask);
				fprintf(file,"Total volum modified %.4lf m3\n", volummask);
			}
		}
		/**
		* Warning searching surface depressions
		*/
		if (chksink2 == 1)
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"Morphometry, sink detection: writing output raster\n");
			fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
			//fprintf(file,"Ruta: %s\n", nom_newdem);
			fprintf(file,"\n**********************************\n");
			i++;
		}
		/**
		* Warning writting s-aspect
		*/	
		if(chkasp1 == 1)
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"Morphometry, S-ASPECT: writing output raster - CLASES\n");
			fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
			//fprintf(file,"Ruta: %s\n", nom_rast2);
			fprintf(file,"\n**********************************\n");
			i++;
		}
		if(chkasp2 == 1)
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"Morphometry, S-ASPECT: writing output raster - DEGREES\n");
			fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
			//fprintf(file,"Ruta: %s\n", nom_rast2);
			fprintf(file,"\n**********************************\n");
			i++;
		}
		if(metasp > 0)                                                      /*!< if slope-aspect is activated */
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"Morphometry, S-ASPECT: Frequency by orientation\n");
			int* val = (int[10]){1,2,4,8,16,32,64,128,255};                     /*!< array with class values  */ 
			for(j=0;j<9;j++)
			{
				superf = (cuenta[j] * resx * resy)/1000000;                      /*!< Calc total surface by s-aspect class  */
				printf("Total cells %i, surface %.2lf km2 with s-aspect %i\n", cuenta[j], superf, val[j]);
			}
			fprintf(file,"\n**********************************\n");
		}	
		
		/**
		* Warning writting s-gradient
		*/	
		if(chkslop1 == 1)
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"Morphometry, S-GRADIENT: writing output raster - DEGREES\n");
			fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
			//fprintf(file,"Ruta: %s\n", nom_rast1);
			fprintf(file,"\n**********************************\n");
			i++;
		}
		if(chkslop2 == 1)
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"Morphometry, S-GRADIENT: writing output raster - CLASSES\n");
			fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
			//fprintf(file,"Ruta: %s\n", nom_rast2);
			fprintf(file,"\n**********************************\n");
			i++;
		}
		if(chkslop3 == 1)
		{
			fprintf(file,"\n**********************************\n");
			fprintf(file,"Morphometry, S-GRADIENT: writing output raster - PERCENTAGES\n");
			fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
			//fprintf(file,"Ruta: %s\n", nom_rast3);
			fprintf(file,"\n**********************************\n");
			i++;
		}
		/**
		* Warning writting vectorial flow path
		*/
		if(flowtyp == 1 || flowtyp == 2)
		{
			k = 0;
			buff_csv = (char *)malloc(sizeof(char)*(ncentros));
			buff_kml = (char *)malloc(sizeof(char)*(ncentros));
			for(j=0;j<ncentros;j++)
			{
				if (errcsv[j] == 1)
				{
					sprintf(text, "%i", j);
					strcat(buff_csv, text);
					k++;
				}
				if (errkml[j] == 1)
				{
					sprintf(text2, "%i", j);
					strcat(buff_kml, text2);
					k++;
				}
			}	
			if(k > 0)
			{
				fprintf(file,"\n**********************************\n");
				fprintf(file,"SINGLE FLOW PATH %i: Writing CSV files\n", flowtyp);
				fprintf(file,"ATTENTION, the following files were not written\n"); 
				fprintf(file,"Numeros: %s\n", buff_csv);
				fprintf(file,"**********************************\n");
				fprintf(file,"SINGLE FLOW PATH %i: Writing KML files\n", flowtyp);
				fprintf(file,"ATTENTION, the following files were not written\n"); 
				fprintf(file,"Numeros: %s\n", buff_kml);
				fprintf(file,"\n**********************************\n");
			}	
			if(chksing3 == 1)
			{
				fprintf(file,"\n**********************************\n");
				fprintf(file,"SINGLE FLOW PATH %i: Writing output RASTER\n", flowtyp);
				fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
				//fprintf(file,"Numeros: %s\n", buff_csv);
			}	
			if(chksing4 == 1)
			{
				fprintf(file,"\n**********************************\n");
				fprintf(file,"SINGLE FLOW PATH %i: Writing output RASTER - SUM\n", flowtyp);
				fprintf(file,"ATTENTION, an error occurred in the output file\n");
			}	
		}
		if(flowtyp == 3)
		{
			if (chksing3 == 1)
			{
				fprintf(file,"\n**********************************\n");
				fprintf(file,"MONTECARLO FLOW PATH: Writing output RASTER\n");
				fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
				//fprintf(file,"Numeros: %s\n", buff_csv);
			}
		}
		if(flowtyp == 4)
		{
			if (chksing3 == 1)
			{
				fprintf(file,"\n**********************************\n");
				fprintf(file,"MULTIFLOW PATH: Writing output RASTER\n");
				fprintf(file,"ATTENTION, an error occurred in the output file\n"); 
				//fprintf(file,"Numeros: %s\n", buff_csv);
			}
		}		
	}
	fclose(file);	
	return 1;

}	
//---------------------------------------------------------------------------------------
//***************************************************************************************
//***************************FIN ESCRITURA ARCHIVOS**************************************
//***************************************************************************************
//****************************INICIO FUNCION MAIN****************************************
//***************************************************************************************
//---------------------------------------------------------------------------------------

/*! MAIN FUNCTION  */
int main(int argn, char **args)
{
int k, i; 
double zc;

	sprintf(name_incfg,"%s",args[1]);                                   /*!< Argument config filename */
	getcwd(cwd, sizeof(cwd));                                           /*!< get working directory */
	chksum = 0;                                                         /*!< Count errors */
	printf("\n\n");
	printf("*******************************************************************\n");
    printf("%s\n", wrst(5));
    printf("*******************************************************************\n\n");
	read_cfg();                                                         /*!< Call read cfg function */
	
	printf("*******************************************************************\n");
    printf("%s\n", wrst(6));
    printf("*******************************************************************\n\n");
	if(dem_type == 1)opendem = read_bingrd();                           /*!< Call read DEM function Binary */
	if(dem_type == 2)opendem = read_ascgrd();                           /*!< Call read DEM function ASCII */
	if(opendem == 1)                                                    /*!< if error reading dem */
	{
		printf("%s\n", wrst(25));
		printf("%s\n", wrst(26));
		exit(0);                                                        /*!< exit program */
	}	
	if(modidem == 1)                                                    /*!< if DEM modified is activated */
	{
		printf("*******************************************************************\n");
		printf("%s\n", wrst(7));
		printf("*******************************************************************\n");
		if(fase == 1)                                                   /*!< if Step 1 */
		{
			printf("%s\n", wrst(8));
			printf("*******************************************************************\n");
			if(mask_type == 1)openmask = read_grdbinmask();             /*!< Call read MASK function Binary */
			if(mask_type == 2)openmask = read_grdascmask();             /*!< Call read MASK function ASCII */
			if(openmask == 1)                                           /*!< if error reading mask */
			{
				printf("%s\n", wrst(25));
				printf("%s\n", wrst(27));
				exit(0);                                                /*!< exit program */
			}	
			if(colx != col2x && rowy != row2y)                          /*!< Check if DEM and MASK dimension are equal */
			{
				printf("\n**********************************\n");
				printf("%s\n", wrst(10));
				printf("%s\n", wrst(11));
				printf("\n**********************************\n");
				printf("%s\n", wrst(25));
				printf("%s\n", wrst(28));
				exit(0);                                                /*!< If not, exit the program */
			}
			chkmod1 = getnpt();                                         /*!< call get - xyzz */
			/**
			* Realizing memory
			*/
			for(i=0;i<colx;i++)free(mask[i]);
			free(mask);
			mask = NULL;	
			if(chkmod1 == 1)                                            /*!< if error creating output coordinate file */
			{
				printf("%s\n", wrst(25));
				printf("%s\n", wrst(29));
				printf("Filename: %s\n", nom_mask);
				exit(0);                                                /*!< exit program */
			}
		}
		if(fase == 2)                                                   /*!< if Step 2 */
		{
			printf("%s\n", wrst(9));
			printf("*******************************************************************\n");
			chkmod2 = readchangenewz();                                 /*!< Call read/change xyzz function */
			if(chkmod2 == 1)                                            /*!< if error reading input coordinate file */
			{
				printf("%s\n", wrst(25));
				printf("%s\n", wrst(30));
				printf("Filename: %s\n", name_newz);
				exit(0);                                                /*!< exit program */
			}
		}	
	}
	if(metsink > 0)                                                     /*!< if surface depression is activated */
	{
		printf("*******************************************************************\n");
		printf("%s\n", wrst(12));
		printf("*******************************************************************\n\n");
		chksink = fix_sinks();                                          /*!< Calling surface depression detection/modification */
		if(chksink == 1)                                                /*!< if error creating xyzz file */
		{
			printf("%s\n", wrst(25));
			printf("%s\n", wrst(29));
			printf("Ruta: %s\n", nom_sink);
			exit(0);                                                    /*!< exit program */
		}	
	}	 
	if(metasp > 0)                                                      /*!< if slope-aspect is activated */
	{
		printf("\n\n");
		printf("*******************************************************************\n");
		printf("%s\n", wrst(13));
		printf("*******************************************************************\n\n");
		calc_saspect();                                                 /*!< Calling slope-aspect */
	}	
	if(metslop > 0)                                                     /*!< if slope-gradient is activated */
	{
		printf("\n\n");
		printf("*******************************************************************\n");
		printf("%s\n", wrst(14));
		printf("*******************************************************************\n\n");
		calc_sgrad();                                                   /*!< Calling slope-gradient */
	}		
	if(flowtyp > 0)                                                     /*!< if flow-path is activated */
	{
		printf("\n\n");
		printf("*******************************************************************\n");
		printf("%s\n", wrst(15));
		printf("*******************************************************************\n\n");
		/**
		* Optimize array reset operation
		* Create single temporal array to save arrays index from main rasters
		* To reduce reset operation time, only reset to 0 those selected cells
		*/
		if(flowtyp < 3) sizearray =  (lmax / resx) * 100;
		if(flowtyp == 3) sizearray =  (lmax / resx) * nitera;           /*!< Max cells per flow path */
		inxdir = MakeSingleArray(sizearray);
		inydir = MakeSingleArray(sizearray);
		
		printf("Max cells per flow path %i\n", sizearray); 
		
		maximun = 0.0;
		for(k=0;k<ncentros;k++)                                         /*!< total init z points */
		{  
			nfile = k;
			xc   = puntos[k].ptxcoor;                                   /*!< getting init z points values */
			yc   = puntos[k].ptycoor;
			ok = 0;
			printf("\n%s %i\n", wrst(16), k);
			printf("Limits x %.2lf  xmin %.2lf xmax %.2lf, y %.2lf  ymin %.2lf ymax %.2lf\n", xc, xmin, xmax, yc, ymin, ymax);
			if(xc > xmin && xc < xmax && yc > ymin && yc < ymax)        /*!< if the init z point is inside the working area */
			{
				calc_index(xc, yc);
				nxc = vcol;
				nyc = vrow;
				zc = topoless[nxc][nyc];
				/**
				* cells located in border won't be considered here
				*/
				if( (vcol > 2 && vcol < colx-2) && (vrow > 2 && vrow < rowy-2) ) 
				{
					printf("Init Z point %3i de %i  nxc= %i nyc= %i  zc=%.2lf\n", k, ncentros, nxc, nyc, zc);
					
					/**
					* according to the selected algorithm, the proper function will be called
					*/				
					if(flowtyp == 1 || flowtyp == 2)calc_singflow();
					if(flowtyp == 3) calc_montflow();
					if(flowtyp == 4) calc_mulflow();
				}			
			}
			else printf("%s\n", wrst(17));
		}
		if(mod == 0)call_writeraster();	
	}	
	/**
	* Realising memory
	*/
	printf("*******************************************************************\n");
    printf("%s\n", wrst(18));
    printf("*******************************************************************\n\n");
	for(i=0;i<colx;i++)
	{
		free(topo[i]);
		free(topoless[i]);
		free(rast2[i]);
		free(rast3[i]);
		free(rast1[i]);
	}	
	free(topo);
	topo = NULL;
	free(topoless);
	topoless = NULL;
	free(rast2);
	rast2 = NULL;
	free(rast3);
	rast3 = NULL;
	free(rast1);
	rast1 = NULL;
	
	if (chksum == 0)
    {
		printf("*******************************************************************\n");
		printf("%s. \n", wrst(31));
		printf("*******************************************************************\n\n");
		if (modidem == 1 || metasp > 0)
		{
			write_resum();
		}
    }
	if (chksum > 0)
	{
		printf("*******************************************************************\n");
		printf("%s. Total=%i\n", wrst(24), chksum);
		printf("*******************************************************************\n\n");
		write_resum();
    }
    
    printf("-------------------\n\n");
    printf("%s\n", wrst(19));
    return 0;
}


 
