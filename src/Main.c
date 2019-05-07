#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
#include </Users/fabiofurini/Applications/IBM/ILOG/CPLEX_Studio126/cplex/include/ilcplex/cplex.h>
//#include </home/fabio/ILOG/CPLEX_Studio_AcademicResearch125/cplex/include/ilcplex/cplex.h>
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct data
{
	char *istname;
	int algorithm;
	int number_of_CPU;
	int timeLimit;

	int item_number;
	double capacity;
	double *weights;
	double *profits;

	int status,ccnt,rcnt,nzcnt,lpstat,nodecount,cur_numrows,cur_numcols;
	int* rmatbeg,*rmatind,*cmatbeg, *cmatind;
	double *rmatval,*cmatval,*rngval,*x,*pi,*obj, *lb, *ub,*rhs,coef_p,objval,bestobjval;
	char *xctype,*sense;
	char **rowname,**colname;

	CPXENVptr env;
	CPXLPptr  lp;

} data;

/***********************************************************************************/
void ReadFile(data *KP_instance);
/***********************************************************************************/

/***********************************************************************************/
void FreeData(data *KP_instance);
/***********************************************************************************/

/***********************************************************************************/
void  kp_load_cplex(data *KP_instance);
/***********************************************************************************/

/***********************************************************************************/
double kp_solve_cplex(data *KP_instance);
/***********************************************************************************/

/***********************************************************************************/
void kp_free_cplex(data *KP_instance);
/***********************************************************************************/

/***********************************************************************************/
int main(int argc, char** argv)
/***********************************************************************************/
{


	data KP_instance;

	KP_instance.istname=(char*) calloc(2000,sizeof(char));
	if (argc == 2) {
		strcpy(KP_instance.istname, argv[1]);
	}
	else {
		printf("ERROR INPUT: \t instance\n");
		exit(2);
	}


	KP_instance.number_of_CPU=1;
	KP_instance.timeLimit=600;

	printf("\n\n");
	printf("-> CPU NUMBER %d\n",KP_instance.number_of_CPU);
	printf("-> TIME LIMIT %d\n",KP_instance.timeLimit);
	printf("\n\nSOLUTION INSTANCE: ->\t %s\n",KP_instance.istname);


	///////////////////////////////////////////////////////////////////////////////////
	ReadFile(&KP_instance);
	///////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	kp_load_cplex(&KP_instance);
	printf("\n-> model loaded!\n");
	///////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	kp_solve_cplex(&KP_instance);
	printf("\n-> model solved!\n");
	///////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	kp_free_cplex(&KP_instance);
	printf("\n-> model freed!\n");
	///////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	FreeData(&KP_instance);
	///////////////////////////////////////////////////////////////////////////////////

	printf("\nDONE!\n");

	return 1;
}

/***********************************************************************************/
void FreeData(data *KP_instance)
/***********************************************************************************/
{

	free(KP_instance->istname);
	free(KP_instance->weights);
	free(KP_instance->profits);

}

/***********************************************************************************/
void ReadFile(data *KP_instance)
/***********************************************************************************/
{

	int i,j;
	int status;
	FILE* inputFile;

	/* open the file */
	if( (inputFile=fopen(KP_instance->istname, "r")) == NULL ){
		printf("Error reading: %s\n", KP_instance->istname);
		exit(2);
	}


	status=fscanf(inputFile, "%d", &(KP_instance->item_number));

	KP_instance->weights=(double*) calloc(KP_instance->item_number,sizeof(double));
	KP_instance->profits=(double*) calloc(KP_instance->item_number,sizeof(double));

	int dummy=0;

	long int total_weight=0;

	for(i=0; i<KP_instance->item_number; i++){
		status=fscanf(inputFile, "%d", &dummy);
		status=fscanf(inputFile, "%d", &dummy);
		KP_instance->profits[i]=dummy;
		status=fscanf(inputFile, "%d", &dummy);
		KP_instance->weights[i]=dummy;
		total_weight+=KP_instance->weights[i];
	}

	status=fscanf(inputFile, "%d", &dummy);
	KP_instance->capacity=dummy;


	fclose(inputFile);


	printf("\n\nFeatures:\n");
	printf("Items\t\t%d\n",KP_instance->item_number);
	printf("Capacity\t%.1f\n\n",KP_instance->capacity);


	printf("Weights\n");
	for(j=0; j<KP_instance->item_number; j++)
	{
		printf("profit item %d -> %.1f\n",j,KP_instance->profits[j]);
	}
	printf("\n");
	printf("Profits\n");
	for(j=0; j<KP_instance->item_number; j++)
	{
		printf("weight item %d -> %.1f\n",j,KP_instance->weights[j]);
	}
	printf("\n");

	printf("Instance read!\n");
	///////////////////////////////////////////////////////////

}



/***********************************************************************************/
void kp_load_cplex(data *KP_instance)
/***********************************************************************************/
{

	int i,j;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * setting the CPLEX environment

	//opening the environment
	KP_instance->env=CPXopenCPLEX(&(KP_instance->status));
	if(KP_instance->status!=0)
	{
		printf("cannot open CPLEX environment\n");
		exit(-1);
	}

	//opening the pointer to the problem
	KP_instance->lp=CPXcreateprob(KP_instance->env,&(KP_instance->status),"KP");
	if(KP_instance->status!=0)
	{
		printf("cannot create problem\n");
		exit(-1);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * creating the variables *
	KP_instance->ccnt=KP_instance->item_number;
	KP_instance->obj=(double*) calloc(KP_instance->ccnt,sizeof(double));
	KP_instance->lb=(double*) calloc(KP_instance->ccnt,sizeof(double));
	KP_instance->ub=(double*) calloc(KP_instance->ccnt,sizeof(double));
	KP_instance->xctype=(char*) calloc(KP_instance->ccnt,sizeof(char));


	KP_instance->colname=(char**) calloc(KP_instance->ccnt,sizeof(char*));
	for(i=0;i<KP_instance->ccnt;i++){KP_instance->colname[i]=(char*) calloc(2000,sizeof(char));}

	for(i=0; i<KP_instance->ccnt; i++)
	{
		KP_instance->obj[i]=KP_instance->profits[i];
		KP_instance->lb[i]=0.0;
		KP_instance->ub[i]=1.0;
		KP_instance->xctype[i]='B';
		sprintf(KP_instance->colname[i], "x%d",i+1);
	}

	KP_instance->status=CPXnewcols(KP_instance->env,KP_instance->lp,KP_instance->ccnt,KP_instance->obj,KP_instance->lb,KP_instance->ub,KP_instance->xctype,KP_instance->colname);
	if(KP_instance->status!=0)
	{
		printf("error in CPXnewcols\n");
		exit(-1);
	}

	free(KP_instance->obj);
	free(KP_instance->lb);
	free(KP_instance->ub);
	free(KP_instance->xctype);

	for(i=0;i<KP_instance->ccnt;i++){free(KP_instance->colname[i]);}
	free(KP_instance->colname);


	// * setting the objective function in the maximization form
	CPXchgobjsen(KP_instance->env,KP_instance->lp,CPX_MAX);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * creating the knapsack constraint *
	KP_instance->rcnt=1;
	KP_instance->nzcnt=KP_instance->item_number;
	KP_instance->rhs=(double*) calloc(KP_instance->rcnt,sizeof(double));
	KP_instance->sense=(char*) calloc(KP_instance->rcnt,sizeof(double));

	KP_instance->rhs[0]=KP_instance->capacity;
	KP_instance->sense[0]='L';


	KP_instance->rmatbeg=(int*) calloc(KP_instance->rcnt,sizeof(int));
	KP_instance->rmatind=(int*) calloc(KP_instance->nzcnt,sizeof(int));
	KP_instance->rmatval=(double*) calloc(KP_instance->nzcnt,sizeof(double));

	for(i=0; i<KP_instance->item_number; i++)
	{
		KP_instance->rmatval[i]=KP_instance->weights[i];
		KP_instance->rmatind[i]=i;
	}

	KP_instance->rmatbeg[0]=0;

	KP_instance->status=CPXaddrows(KP_instance->env,KP_instance->lp,0,KP_instance->rcnt,KP_instance->nzcnt,KP_instance->rhs,KP_instance->sense,KP_instance->rmatbeg,KP_instance->rmatind,KP_instance->rmatval,NULL,NULL);
	if(KP_instance->status!=0)
	{
		printf("error in CPXaddrows\n");
		exit(-1);
	}

	free(KP_instance->rmatbeg);
	free(KP_instance->rmatval);
	free(KP_instance->rmatind);
	free(KP_instance->rhs);
	free(KP_instance->sense);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * writing the created ILP model on a file *
	KP_instance->status=CPXwriteprob(KP_instance->env,KP_instance->lp,"kp.lp",NULL);
	if(KP_instance->status!=0) {
		printf("error in CPXwriteprob\n");
		exit(-1);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////




}

/***********************************************************************************/
void kp_free_cplex(data *KP_instance)
/***********************************************************************************/
{


	KP_instance->status=CPXfreeprob(KP_instance->env,&(KP_instance->lp));
	if(KP_instance->status!=0) {printf("error in CPXfreeprob\n");exit(-1);}

	KP_instance->status=CPXcloseCPLEX(&(KP_instance->env));
	if(KP_instance->status!=0) {printf("error in CPXcloseCPLEX\n");exit(-1);}

}

/***********************************************************************************/
double kp_solve_cplex(data *KP_instance)
/***********************************************************************************/
{

	int i,j;


	CPXsetintparam (KP_instance->env, CPX_PARAM_SCRIND, CPX_ON);



	// * Set relative tolerance *
	KP_instance->status = CPXsetdblparam (KP_instance->env, CPX_PARAM_EPAGAP, 0.0);
	if (KP_instance->status)
	{
		printf ("error for CPX_PARAM_EPAGAP\n");
	}

	// * Set relative tolerance *
	KP_instance->status = CPXsetdblparam (KP_instance->env, CPX_PARAM_EPGAP, 0.0);
	if (KP_instance->status)
	{
		printf ("error for CPX_PARAM_EPGAP\n");
	}


	// * Set mip tolerances integrality *
	KP_instance->status = CPXsetdblparam (KP_instance->env, CPX_PARAM_EPINT, 0.0);
	if (KP_instance->status)
	{
		printf ("error for CPX_PARAM_EPINTP\n");
	}

	// * Set Feasibility tolerance *
	KP_instance->status = CPXsetdblparam (KP_instance->env, CPX_PARAM_EPRHS, 1e-9);
	if (KP_instance->status)
	{
		printf ("error for CPX_PARAM_EPRHS\n");
	}

	// * Set number of CPU*

	KP_instance->status = CPXsetintparam (KP_instance->env, CPX_PARAM_THREADS, KP_instance->number_of_CPU);
	if (KP_instance->status)
	{
		printf ("error for CPX_PARAM_EPRHS\n");
	}

	// * Set time limit *

	KP_instance->status = CPXsetdblparam (KP_instance->env, CPX_PARAM_TILIM,KP_instance->timeLimit);
	if (KP_instance->status)
	{
		printf ("error for CPX_PARAM_EPRHS\n");
	}



	///////////////////////////////////////////////////////////////////////////////////
	// * solving the MIP model
	clock_t time_start=clock();


	KP_instance->status=CPXmipopt(KP_instance->env,KP_instance->lp);
	if(KP_instance->status!=0)
	{
		printf("error in CPXmipopt\n");
		exit(-1);
	}

	clock_t time_end=clock();
	double solution_time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;
	///////////////////////////////////////////////////////////////////////////////////


	// * getting the solution

	KP_instance->x=(double*) calloc(KP_instance->item_number,sizeof(double));


	KP_instance->status=CPXgetmipx(KP_instance->env,KP_instance->lp,KP_instance->x,0,KP_instance->item_number-1);
	if(KP_instance->status!=0)
	{
		printf("error in CPXgetmipx\n");
		exit(-1);
	}

	KP_instance->status=CPXgetmipobjval(KP_instance->env,KP_instance->lp,&(KP_instance->objval));
	if(KP_instance->status!=0)
	{
		printf("error in CPXgetmipobjval\n");
		exit(-1);
	}

	printf("\n\nMIP solution value ->\t\%f",KP_instance->objval);


	printf("\n\nSolution\n");
	for (i = 0; i < KP_instance->item_number; i++)
	{
		printf("item %d -> %d\n",i+1 ,(int)(KP_instance->x[i]+0.5));
	}
	printf("\n\n");

	KP_instance->status=CPXgetbestobjval(KP_instance->env,KP_instance->lp,&(KP_instance->bestobjval));
	if(KP_instance->status!=0)
	{
		printf("error in CPXgetbestobjval\n");
		exit(-1);
	}

	KP_instance->lpstat=CPXgetstat(KP_instance->env,KP_instance->lp);
	KP_instance->nodecount = CPXgetnodecnt(KP_instance->env, KP_instance->lp);


	///////////////////////////////////////////////////////////////////////////////////
	/* linear programming relaxation*/

	double solution_time_lp=0;

	KP_instance->status = CPXchgprobtype (KP_instance->env, KP_instance->lp, CPXPROB_LP);

	clock_t time_start_lp=clock();
	KP_instance->status=CPXlpopt(KP_instance->env,KP_instance->lp);
	if(KP_instance->status!=0)
	{
		printf("err_FILEor in CPXlpopt slave solve\n");
		exit(-1);
	}

	clock_t time_end_lp=clock();

	solution_time_lp=(double)(time_end_lp-time_start_lp)/(double)CLOCKS_PER_SEC;

	double cplex_lp;

	KP_instance->status=CPXgetobjval(KP_instance->env,KP_instance->lp,&cplex_lp);
	if(KP_instance->status!=0)
	{
		printf("error in CPXgetmipobjval\n");
		exit(-1);
	}

	printf("\n\nLP solution value ->\t\%f",cplex_lp);

	KP_instance->status=CPXgetx(KP_instance->env,KP_instance->lp,KP_instance->x,0,KP_instance->item_number-1);
	if(KP_instance->status!=0)
	{
		printf("error in CPXgetmipx\n");
		exit(-1);
	}

	printf("\n\nSolution\n");
	for (i = 0; i < KP_instance->item_number; i++)
	{
		printf("item %d -> %.3f (p/w %.3f)\n",i+1,KP_instance->x[i],KP_instance->profits[i]/KP_instance->weights[i]);
	}
	printf("\n");

	///////////////////////////////////////////////////////////////////////////////////

	KP_instance->cur_numcols=CPXgetnumcols(KP_instance->env,KP_instance->lp);
	KP_instance->cur_numrows=CPXgetnumrows(KP_instance->env,KP_instance->lp);

	printf("\nnumcols\t%d\n",KP_instance->cur_numcols);
	printf("\nnumrows\t%d\n",KP_instance->cur_numrows);

	FILE *out;
	out=fopen("info_kp.txt","a+");
	fprintf(out,"%s\t%f\t%f\t%f\t%f\t%d\t%d\t%f\t%f\n",KP_instance->istname,KP_instance->capacity,KP_instance->objval,KP_instance->bestobjval,solution_time,KP_instance->lpstat,KP_instance->nodecount,cplex_lp,solution_time_lp);
	fclose(out);

	free(KP_instance->x);

	return KP_instance->objval;

}




