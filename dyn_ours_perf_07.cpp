#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include<cmath>
using namespace std;

#include "trace.h"
#include "energy_costs.h"
#include "area_costs.h"
#include "performance_costs.h"

int granularity;
ofstream dst;

void set_banks(int &d,const vector<vector<int>> &S,int i,int j, vector<int> &banks);
void print_banks(const vector<int> &banks);
void calculate_costs(const vector<int> &banks, const vector<vector<int>> &f, float &energy, float &area);

int main() {
   
    //1.Set up the file to hold testing results:
    std::ostringstream s;
	s << "performance_optimization_results.xls";
	string file = s.str();

	//writing to Excel file
	dst.open(file.c_str());

	if(dst)
	{
		cout << "file created\n";
		dst << "\nTesting results optimized for: PERFORMANCE\n";
		dst << "\nGran. (B)\tMin. Part. Size (B)\tEnergy Cost (nJ)\tPerformance Cost (ns) [opt.]\tArea Cost (mm2)\tCPU Time (sec)\tRAM Usage (B)\tBanking Configuration (Cut Set)\n";
	}
	else
	{
		cout << "file not created\n";
		return -1;
	}

	int i, j, k, l, N, d, trace_size;//min_bank_size is a calculated parameter from the given area constraint:
															//min_bank_size = N/x; where x is the maximum integer fullfilling:
															//x.area_of(N/x)+(x-1).new_bank_overhead <= area_constraint
														//So far no evidence of multiple ranges @ the same bank
											//granularity specifies the exploration accuracy for each partition possibility
											//i.e. granularity specifies the word width.

   clock_t startTime; //to calculate execution time
   trace_size = sizeof(A)/sizeof(int);

   /*for(int min_bank_size=512; min_bank_size<=4096; min_bank_size*=2)
	   for(granularity=min_bank_size; granularity>=512; granularity/=2);*/

	for(granularity=4; granularity<=512; granularity*=2)
	{
		N = trace_size/granularity;   
		vector<vector<int>> f(N+1, vector<int>(N+1,0)); //access frequency matrix
		//2. Set Access frequency Patterns for address ranges:
		   for (i=1; i<=N; i++)
			 for (j=i; j<=N; j++)
			 {
			   for (k=(i-1)*granularity; k<=j*granularity; k++) f[i][j] += A[k];
			   //f[i][j] /= 5000;
			 }
		for(int min_bank_size=512; (min_bank_size>=granularity)&&(min_bank_size<=trace_size); min_bank_size*=2)	   
	   {			
			cout << "\nGranularity: " << setw(5) << granularity << ", Min. Bank Size: " << min_bank_size <<endl;

		   //3.Declare needed vectors and variables
			startTime = clock();
		   vector<vector<float>> P(N+1, vector<float>(N+1,0)); //processing matrix
		   vector<vector<int>> S(N+1, vector<int>(N+1,0)); //address ranges matrix
		   float energy_cost=0, area_cost=0;
		   vector<int> banks(N+1,0); //defines the partitioning solution		   

		   //4. Processing (Filling Up the Dynamic Table):
		   for(j=min_bank_size/granularity; j<=N; j++)
			   for(i=1,k=j; k<=N ;i++,k++)
			   {
			
				   if(k-i+1 == 0) continue;
			
					//set values for unpartitioned i-k
					S[i][k] = 0; //unpartitioned range
					P[i][k] = f[i][k]*performance_costs[(k-i+1)*granularity];  /*cost per access to a memory of this size (i-j) / f[i][j]; */
			
					//base case: No Further Partitioning possible
					 if((k-i)*granularity < 2 * min_bank_size) //No chance for partitioning
						 continue;
					 else //Choices to be made about the best partitioning position:
					 {
						 for(l=i+min_bank_size/granularity; l<=k-min_bank_size/granularity; l++)
						 {
							 if(P[i][l]+P[l+1][k] < P[i][k])//partition at position l is better than the best found so far
							 {
								 S[i][k] = l;
								 P[i][k] = P[i][l]+P[l+1][k];
							 }
						 }
					 }

			   }

			   //5. Set the banks vector to contain the solution cut set and calculate the banks costs
			   d=1;
			   set_banks(d,S,1,N,banks); 
			   banks[d]=N*granularity;
			   calculate_costs(banks, f, energy_cost, area_cost);

			   //7. Print the results to the excel file:
			   //"\nGranularity(B)\tMinimum Part. Size\tEnergy Cost(nJ)-optimized\t"
			   dst<<granularity<<"\t"<<min_bank_size<<"\t"<<energy_cost<<"\t";
			    //"Performance Cost(ns)\tArea Cost(nm2)\t"
			   dst<<P[1][N]<<"\t"<<area_cost<<"\t";
			   //"CPU Time(sec)\t"
			   dst<<double( clock() - startTime ) / (double)CLOCKS_PER_SEC<<"\t";
			   //"RAM Usage(B)\t"
			   dst<<sizeof(int)*(2*(N+1)*(N+1)+N+1)+sizeof(float)*(N+1)*(N+1)<<"\t";
			   //"Banking Configuration(Cut Set)\n";
			   print_banks(banks);
			   dst<<endl;

	   }//end for granularity
	}//end for min_bank_size

   dst.close();

   system("pause");
}

//Function to set the banks vector to contain the solution cut set
void set_banks(int &d, const vector<vector<int>>& S,int i,int j, vector<int> &banks) //d is the depth of recursion, it also represents the number of banks added so far
{
	if(S[i][j]==0||S[i][j]<=i||S[i][j]>=j) return;

	set_banks(d, S, i, S[i][j], banks);
	banks[d]=S[i][j]*granularity;
	d++;
	//dst << S[i][j]*granularity << ", ";
	set_banks(d, S, S[i][j]+1, j, banks);
}

//Function to calculate area and performance costs of the banking configuration given by banks vector
void calculate_costs(const vector<int> &banks, const vector<vector<int>> &f, float &energy, float &area)
{
	int size, i, accesses, total_accesses=0;
	for(i=1;(banks[i]!= 0)&&(i<banks.size()); i++)
	{
		size = banks[i]-banks[i-1];
		accesses = f[banks[i-1]/granularity+1][banks[i]/granularity];
		energy += accesses * energy_costs[size];
		area += area_costs[size];
	}
}

//Function to print the cut set 
void print_banks(const vector<int> &banks)
{
	int i;
	dst<<"{0,";
	for(i=1;banks[i+1]!= 0; i++)
		dst<<banks[i]<<", ";
	dst<<banks[i]<<"}";
}