#include <vector>
#include <iostream>
#include "nts.hpp"

using namespace std;
using namespace nts;


struct Example
{
	struct Nts2
	{
		BasicNts *basic;
		BitVectorVariable *arg_1;
		BitVectorVariable *arg_2;
		BitVectorVariable *arg_ret;


		Nts2()
		{
			basic = new BasicNts ( "nts_2" );
			arg_1 = new BitVectorVariable ( "arg_1", 4 );
			arg_2 = new BitVectorVariable ( "arg_2", 8 );
			arg_ret = new BitVectorVariable ( "arg_ret", 16 );

			arg_1->insert_param_in_to    ( *basic );
			arg_2->insert_param_in_to    ( *basic );
			arg_ret->insert_param_out_to ( *basic );
		}

		~Nts2()
		{
			delete basic;
			basic = nullptr;
		}
	};

	// This should call Nts2
	struct Nts1
	{
		BasicNts *basic;
		BitVectorVariable *var_1;
		BitVectorVariable *var_2;
		BitVectorVariable *var_3;

		TransitionRule *ctr1;
		Transition *t1;

		Nts1 ( struct Nts2 *n )
		{
			basic = new BasicNts ( "nts_1" );
			var_1 = new BitVectorVariable ( "var_1",  4 );
			var_2 = new BitVectorVariable ( "var_2",  8 );
			var_3 = new BitVectorVariable ( "var_3", 16 );

			var_1->insert_to ( *basic );
			var_2->insert_to ( *basic );
			var_3->insert_to ( *basic );

			auto s1 = new State ( "s1" );
			auto s2 = new State ( "s2" );
			auto s3 = new State ( "s3" );
			
			s1->insert_to ( *basic );	
			s2->insert_to ( *basic );
			s3->insert_to ( *basic );


			ctr1 = new CallTransitionRule ( n->basic, { var_1, var_2}, { var_3 } );
			// Transition automatically belongs to BasicNts,
			// which owns given states
			new Transition ( *ctr1, *s1, *s2 );
		}
		~Nts1()
		{
			delete basic;
		}

	};


	void examples()
	{
		Nts2 n2;
		Nts1 n1 ( &n2 );
		std::cout << *n2.basic;
		std::cout << *n1.basic;
	}

};

struct Example_callees_callers
{
	BitVectorVariable *bvvar[4];

	BasicNts *nb[2];
	Nts toplevel_nts;

	vector < CallTransitionRule *> ctr;
	vector < FormulaTransitionRule *> ftr;
	vector < Transition * > tr;


	Example_callees_callers() :
		toplevel_nts ( "namedNts" )
	{	
		bvvar[0] = new BitVectorVariable ( "var1",  8 );
		bvvar[1] = new BitVectorVariable ( "var2", 16 );
		bvvar[2] = new BitVectorVariable ( "var3",  1 );
		bvvar[3] = new BitVectorVariable ( "var4",  4 );

		nb[0] = new BasicNts ( "nb0" );
		nb[1] = new BasicNts ( "nb1" );

		auto s1 = new State ( "s1" );
		auto s2 = new State ( "s2" );
		auto s3 = new State ( "s3" );
		auto s4 = new State ( "s4" );

		for ( auto s : { s1, s2, s3, s4 } )
		{
			s->insert_to ( *nb[0] );
		}

		// It is not wise to call before BasicNts has all parameters

		// tr[0] - call transition
		ctr.push_back ( new CallTransitionRule ( nb[1], {}, {} ) );
		tr.push_back ( new Transition ( *ctr.back(), *s1, *s2 ) );

		// tr[1] - call transition
		ctr.push_back ( new CallTransitionRule ( nb[1], {}, {} ) );
		tr.push_back ( new Transition ( *ctr.back(), *s1, *s3 ) );

		// tr[2] - formula transition
		ftr.push_back ( new FormulaTransitionRule ( nullptr ) );
		tr.push_back ( new Transition ( *ftr.back(), *s1, *s4 ) );

		// tr[3] - formula transition
		ftr.push_back ( new FormulaTransitionRule ( nullptr ) );
		tr.push_back ( new Transition ( *ftr.back(), *s2, *s3 ) );

		// After this block toplevel_nts owns all BasicNtses
		nb[0]->insert_to ( toplevel_nts );
		nb[1]->insert_to ( toplevel_nts );

		bvvar[0]->insert_to ( toplevel_nts );
		bvvar[1]->insert_param_in_to (  *nb[1] );
		bvvar[2]->insert_before ( *bvvar[1] );

	}

	// It will automatically destroy everything
	~Example_callees_callers()
	{
		delete bvvar[3];
	}


	void try_callees()
	{
		auto i = nb[0]->callees().begin();

		printf ( "%p == %p\n", tr[0], (*i).transition() );
		i++;
		printf ( "%p == %p\n", tr[1], (*i).transition() );
		i++;
		printf ( "end? %s\n", i == nb[0]->callees().end() ? "yes" : "no" );
	}

	void try_callers()
	{
		auto i = nb[1]->callers().begin();
		auto e = nb[1]->callers().end();
		for (int j = 0; j < 5 && i != e; ++j, ++i )
		{
			printf( "%d: %p\n", j, (*i).transition() );
		}
	}

	void print()
	{
		cout << *nb[0];
		cout << *nb[1];
	}

};

int main ( void )
{
	printf ( "Hello world\n" );

	Example e1;
	cout << "e1.examples()\n";
	e1.examples();

	Example_callees_callers e2;
	cout << "e2.callees()\n";
	e2.try_callees();
	cout << "e2.callers()\n";
	e2.try_callers();
	cout << "e2.print()\n";
	e2.print();

	return 0;
}
